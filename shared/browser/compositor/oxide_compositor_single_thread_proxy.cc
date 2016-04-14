// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_compositor_single_thread_proxy.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/linked_ptr.h"
#include "base/single_thread_task_runner.h"
#include "base/trace_event/trace_event.h"
#include "cc/output/context_provider.h"
#include "gpu/command_buffer/client/context_support.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/GLES2/gl2extchromium.h"

#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_gpu_shims.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_utils.h"

namespace oxide {

bool CompositorSingleThreadProxy::SurfaceIdIsCurrent(uint32_t surface_id) {
  return output_surface_ && output_surface_->surface_id() == surface_id;
}

void CompositorSingleThreadProxy::GetTextureFromMailboxResponse(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    GLuint texture) {
  DCHECK(CalledOnValidThread());

  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::CompositorSingleThreadProxy:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  --mailbox_resource_fetches_in_progress_;

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (texture == 0) {
    // FIXME: This just causes the compositor to hang
    return;
  }

  mailbox_buffer_map_.AddTextureMapping(mailbox, texture);
  DispatchQueuedGLFrameSwaps();
}

void CompositorSingleThreadProxy::CreateEGLImageFromMailboxResponse(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    EGLImageKHR egl_image) {
  DCHECK(CalledOnValidThread());

  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::CompositorSingleThreadProxy:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  --mailbox_resource_fetches_in_progress_;

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (egl_image == EGL_NO_IMAGE_KHR) {
    // FIXME: This just causes the compositor to hang
    return;
  }

  mailbox_buffer_map_.AddEGLImageMapping(mailbox, egl_image);
  DispatchQueuedGLFrameSwaps();
}

void CompositorSingleThreadProxy::DidCompleteGLFrame(
    uint32_t surface_id,
    scoped_ptr<CompositorFrameData> frame) {
  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::CompositorSingleThreadProxy:frames_waiting_for_completion",
      this,
      "frames_waiting_for_completion", frames_waiting_for_completion_);
  --frames_waiting_for_completion_;

  ContinueSwapGLFrame(surface_id, std::move(frame));
}

void CompositorSingleThreadProxy::ContinueSwapGLFrame(
    uint32_t surface_id,
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(CalledOnValidThread());
  DCHECK(frame->gl_frame_data);

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (!queued_gl_frame_swaps_.empty()) {
    QueueGLFrameSwap(std::move(frame));
    return;
  }

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE: {
      GLuint texture =
          mailbox_buffer_map_.ConsumeTextureFromMailbox(
            frame->gl_frame_data->mailbox);
      if (texture == 0U) {
        QueueGLFrameSwap(std::move(frame));
        return;
      }
      frame->gl_frame_data->resource.texture = texture;
      frame->gl_frame_data->type = GLFrameData::Type::TEXTURE;
      break;
    }

    case COMPOSITING_MODE_EGLIMAGE: {
      EGLImageKHR egl_image =
          mailbox_buffer_map_.ConsumeEGLImageFromMailbox(
            frame->gl_frame_data->mailbox);
      if (egl_image == EGL_NO_IMAGE_KHR) {
        QueueGLFrameSwap(std::move(frame));
        return;
      }
      frame->gl_frame_data->resource.egl_image = egl_image;
      frame->gl_frame_data->type = GLFrameData::Type::EGLIMAGE;
      break;
    }

    default:
      NOTREACHED();
  }

  if (!client()) {
    // XXX: Should we synthesize the ack here?
    return;
  }

  client()->SwapCompositorFrameFromProxy(surface_id, std::move(frame));
}

void CompositorSingleThreadProxy::QueueGLFrameSwap(
    scoped_ptr<CompositorFrameData> frame) {
  queued_gl_frame_swaps_.push(std::move(frame));
}

void CompositorSingleThreadProxy::DispatchQueuedGLFrameSwaps() {
  DCHECK(output_surface_);

  std::queue<scoped_ptr<CompositorFrameData>> swaps;

  std::swap(swaps, queued_gl_frame_swaps_);
  while (!swaps.empty()) {
    ContinueSwapGLFrame(output_surface_->surface_id(),
                        std::move(swaps.front()));
    swaps.pop();
  }
}

void CompositorSingleThreadProxy::SetOutputSurface(
    CompositorOutputSurface* output_surface) {
  DCHECK(CalledOnValidThread());
  DCHECK(!output_surface || !output_surface_);

  output_surface_ = output_surface;

  mailbox_buffer_map_.DropAllResources();

  while (!queued_gl_frame_swaps_.empty()) {
    queued_gl_frame_swaps_.pop();
  }
}

void CompositorSingleThreadProxy::MailboxBufferCreated(
    const gpu::Mailbox& mailbox,
    uint64_t sync_point) {
  DCHECK(CalledOnValidThread());

  TRACE_EVENT_ASYNC_BEGIN1(
      "cc",
      "oxide::CompositorSingleThreadProxy:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  ++mailbox_resource_fetches_in_progress_;

  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    CompositorUtils::GetInstance()->GetTextureFromMailbox(
        output_surface_->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorSingleThreadProxy::GetTextureFromMailboxResponse,
          this, output_surface_->surface_id(), mailbox));
  } else {
    DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
    CompositorUtils::GetInstance()->CreateEGLImageFromMailbox(
        output_surface_->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorSingleThreadProxy::CreateEGLImageFromMailboxResponse,
          this, output_surface_->surface_id(), mailbox));
  }
}

void CompositorSingleThreadProxy::MailboxBufferDestroyed(
    const gpu::Mailbox& mailbox) {
  DCHECK(CalledOnValidThread());

  mailbox_buffer_map_.MailboxBufferDestroyed(mailbox);
}

void CompositorSingleThreadProxy::SwapCompositorFrame(
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(CalledOnValidThread());
  DCHECK(output_surface_);

  TRACE_EVENT0("cc", "oxide::CompositorSingleThreadProxy::SwapCompositorFrame");

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE: {
      DCHECK(frame->gl_frame_data);

      TRACE_EVENT_ASYNC_BEGIN1(
          "cc",
          "oxide::CompositorSingleThreadProxy:frames_waiting_for_completion",
          this,
          "frames_waiting_for_completion", frames_waiting_for_completion_);
      ++frames_waiting_for_completion_;

      cc::ContextProvider* context_provider =
          output_surface_->context_provider();
      gpu::gles2::GLES2Interface* gl = context_provider->ContextGL();

      if (!context_provider->ContextCapabilities().gpu.sync_query) {
        gl->Finish();
        DidCompleteGLFrame(output_surface_->surface_id(), std::move(frame));
      } else {
        uint32_t query_id;
        gl->GenQueriesEXT(1, &query_id);
        gl->BeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query_id);
        context_provider->ContextSupport()->SignalQuery(
            query_id,
            base::Bind(&CompositorSingleThreadProxy::DidCompleteGLFrame,
                       this,
                       output_surface_->surface_id(),
                       base::Passed(&frame)));
        gl->EndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);
        gl->Flush();
        gl->DeleteQueriesEXT(1, &query_id);
      }
      break;
    }
    case COMPOSITING_MODE_SOFTWARE:
      DCHECK(frame->software_frame_data);
      client()->SwapCompositorFrameFromProxy(
          output_surface_->surface_id(),
          std::move(frame));
      break;
  }
}

void CompositorSingleThreadProxy::AllFramesReturnedFromClient() {
  if (!client()) {
    return;
  }

  client()->AllFramesReturnedFromClient();
}

void CompositorSingleThreadProxy::DidSwapCompositorFrame(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  DCHECK(CalledOnValidThread());

  if (SurfaceIdIsCurrent(surface_id)) {
    output_surface_->DidSwapBuffers();
  }

  while (!returned_frames.empty()) {
    scoped_refptr<CompositorFrameHandle> handle(returned_frames.back());
    returned_frames.pop_back();

    if (!handle.get()) {
      continue;
    }

    scoped_ptr<CompositorFrameData> frame = std::move(handle->data_);
    ReclaimResourcesForFrame(handle->surface_id_, frame.get());
  }
}

void CompositorSingleThreadProxy::ReclaimResourcesForFrame(
    uint32_t surface_id,
    CompositorFrameData* frame) {
  DCHECK(CalledOnValidThread());

  CompositorFrameAck ack;
  switch (mode_) {
    case COMPOSITING_MODE_SOFTWARE:
      ack.software_frame_id = frame->software_frame_data->id;
      break;
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE:
      ack.gl_frame_mailbox = frame->gl_frame_data->mailbox;
      mailbox_buffer_map_.ReclaimMailboxBufferResources(
          ack.gl_frame_mailbox);
      break;
  }

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  output_surface_->ReclaimResources(ack);
}

CompositorSingleThreadProxy::CompositorSingleThreadProxy(
    CompositorProxyClient* client)
    : CompositorProxy(client),
      mode_(CompositorUtils::GetInstance()->GetCompositingMode()),
      output_surface_(nullptr),
      mailbox_buffer_map_(mode_),
      frames_waiting_for_completion_(0),
      mailbox_resource_fetches_in_progress_(0) {}

CompositorSingleThreadProxy::~CompositorSingleThreadProxy() {}

} // namespace oxide
