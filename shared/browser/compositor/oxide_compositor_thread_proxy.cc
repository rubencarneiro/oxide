// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_compositor_thread_proxy.h"

#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/output/context_provider.h"
#include "cc/output/gl_frame_data.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "gpu/command_buffer/client/context_support.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/GLES2/gl2extchromium.h"

#include "oxide_compositor.h"
#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_gpu_shims.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_utils.h"

namespace oxide {

CompositorThreadProxy::~CompositorThreadProxy() {}

void CompositorThreadProxy::GetTextureFromMailboxResponseOnOwnerThread(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    GLuint texture) {
  if (texture == 0) {
    return;
  }

  MailboxBufferMap::DelayedFrameQueue ready_frames;

  mailbox_buffer_map_.AddTextureMapping(surface_id,
                                        mailbox,
                                        texture,
                                        &ready_frames);

  while (!ready_frames.empty()) {
    linked_ptr<CompositorFrameData> frame = ready_frames.front();
    ready_frames.pop();
    SendSwapGLFrameOnOwnerThread(make_scoped_ptr(frame.release()));
  }
}

void CompositorThreadProxy::CreateEGLImageFromMailboxResponseOnOwnerThread(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    EGLImageKHR egl_image) {
  if (egl_image == EGL_NO_IMAGE_KHR) {
    return;
  }

  MailboxBufferMap::DelayedFrameQueue ready_frames;

  if (!mailbox_buffer_map_.AddEGLImageMapping(surface_id,
                                              mailbox,
                                              egl_image,
                                              &ready_frames)) {
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                   GpuUtils::GetHardwareEGLDisplay(),
                   egl_image));
  }

  while (!ready_frames.empty()) {
    linked_ptr<CompositorFrameData> frame = ready_frames.front();
    ready_frames.pop();
    SendSwapGLFrameOnOwnerThread(make_scoped_ptr(frame.release()));
  }
}

void CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread(
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(frame->software_frame_data);

  scoped_ptr<cc::SharedBitmap> bitmap(
      content::HostSharedBitmapManager::current()->GetSharedBitmapFromId(
        frame->size_in_pixels, frame->software_frame_data->bitmap_id));
  if (!bitmap) {
    // This occurs if the output surface was destroyed, so there's no point
    // in sending an ack
    return;
  }

  if (!owner().compositor) {
    DidSkipSwapCompositorFrame(std::move(frame));
    return;
  }

  frame->software_frame_data->pixels = bitmap->pixels();

  owner().compositor->SendSwapCompositorFrameToClient(std::move(frame));
}

void CompositorThreadProxy::DidCompleteGLFrameOnImplThread(
    scoped_ptr<CompositorFrameData> frame) {
  if (!mailbox_buffer_map_.CanBeginFrameSwap(frame.get())) {
    return;
  }

  owner_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&CompositorThreadProxy::SendSwapGLFrameOnOwnerThread,
                 this,
                 base::Passed(&frame)));
}

void CompositorThreadProxy::SendSwapGLFrameOnOwnerThread(
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(frame->gl_frame_data);

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE: {
      GLuint texture =
          mailbox_buffer_map_.ConsumeTextureFromMailbox(
            frame->gl_frame_data->mailbox);
      if (texture == 0) {
        // This should only occur if the output surface was destroyed, so
        // there's no need to send an ack
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
        // This should only occur if the output surface was destroyed, so
        // there's no need to send an ack
        return;
      }
      frame->gl_frame_data->resource.egl_image = egl_image;
      frame->gl_frame_data->type = GLFrameData::Type::EGLIMAGE;
      break;
    }

    default:
      NOTREACHED();
  }

  if (!owner().compositor) {
    DidSkipSwapCompositorFrame(std::move(frame));
    return;
  }

  owner().compositor->SendSwapCompositorFrameToClient(std::move(frame));
}

void CompositorThreadProxy::DidSkipSwapCompositorFrame(
    scoped_ptr<CompositorFrameData> frame) {
  uint32_t surface_id = frame->surface_id;

  FrameHandleVector frames;
  frames.push_back(new CompositorFrameHandle(this, std::move(frame)));

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, frames));
}

void CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  if (impl().output_surface &&
      surface_id == impl().output_surface->surface_id()) {
    impl().output_surface->DidSwapBuffers();
  }

  while (!returned_frames.empty()) {
    scoped_refptr<CompositorFrameHandle> handle(returned_frames.back());
    returned_frames.pop_back();

    if (!handle.get()) {
      continue;
    }

    scoped_ptr<CompositorFrameData> frame = std::move(handle->data_);

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

    if (impl().output_surface &&
        frame->surface_id == impl().output_surface->surface_id()) {
      impl().output_surface->ReclaimResources(ack);
    }
  }
}

void CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread(
    scoped_ptr<CompositorFrameData> frame) {
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

  if (!impl().output_surface ||
      frame->surface_id != impl().output_surface->surface_id()) {
    return;
  }

  impl().output_surface->ReclaimResources(ack);
}

CompositorThreadProxy::OwnerData& CompositorThreadProxy::owner() {
  DCHECK(owner_thread_checker_.CalledOnValidThread());
  return owner_unsafe_access_;
}

CompositorThreadProxy::ImplData& CompositorThreadProxy::impl() {
  DCHECK(impl_thread_checker_.CalledOnValidThread());
  return impl_unsafe_access_;
}

CompositorThreadProxy::CompositorThreadProxy(Compositor* compositor)
    : mode_(CompositorUtils::GetInstance()->GetCompositingMode()),
      owner_message_loop_(base::ThreadTaskRunnerHandle::Get()),
      mailbox_buffer_map_(mode_) {
  owner().compositor = compositor;
  impl_thread_checker_.DetachFromThread();
}

void CompositorThreadProxy::CompositorDestroyed() {
  owner().compositor = nullptr;
}

void CompositorThreadProxy::SetOutputSurface(
    CompositorOutputSurface* output_surface) {
  DCHECK(!output_surface || !impl().output_surface);
  DCHECK(!impl_message_loop_ ||
         impl_message_loop_ == base::ThreadTaskRunnerHandle::Get());

  impl().output_surface = output_surface;
  impl_message_loop_ = base::ThreadTaskRunnerHandle::Get();

  mailbox_buffer_map_.SetOutputSurfaceID(
      output_surface ? output_surface->surface_id() : 0);
}

void CompositorThreadProxy::MailboxBufferCreated(const gpu::Mailbox& mailbox,
                                                 uint32_t sync_point) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    CompositorUtils::GetInstance()->GetTextureFromMailbox(
        impl().output_surface->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorThreadProxy::GetTextureFromMailboxResponseOnOwnerThread,
          this, impl().output_surface->surface_id(), mailbox),
        owner_message_loop_);
  } else {
    DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
    CompositorUtils::GetInstance()->CreateEGLImageFromMailbox(
        impl().output_surface->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorThreadProxy::CreateEGLImageFromMailboxResponseOnOwnerThread,
          this, impl().output_surface->surface_id(), mailbox),
        owner_message_loop_);
  }
}

void CompositorThreadProxy::MailboxBufferDestroyed(
    const gpu::Mailbox& mailbox) {
  mailbox_buffer_map_.MailboxBufferDestroyed(mailbox);
}

void CompositorThreadProxy::SwapCompositorFrame(CompositorFrameData* frame) {
  scoped_ptr<CompositorFrameData> frame_copy =
      CompositorFrameData::AllocFrom(frame);

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE: {
      DCHECK(frame_copy->gl_frame_data);

      cc::ContextProvider* context_provider =
          impl().output_surface->context_provider();
      gpu::gles2::GLES2Interface* gl = context_provider->ContextGL();

      if (!context_provider->ContextCapabilities().gpu.sync_query) {
        gl->Finish();
        DidCompleteGLFrameOnImplThread(std::move(frame_copy));
      } else {
        uint32_t query_id;
        gl->GenQueriesEXT(1, &query_id);
        gl->BeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query_id);
        context_provider->ContextSupport()->SignalQuery(
            query_id,
            base::Bind(&CompositorThreadProxy::DidCompleteGLFrameOnImplThread,
                       this,
                       base::Passed(&frame_copy)));
        gl->EndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);
        gl->Flush();
        gl->DeleteQueriesEXT(1, &query_id);
      }
      break;
    }
    case COMPOSITING_MODE_SOFTWARE: {
      DCHECK(frame_copy->software_frame_data);

      owner_message_loop_->PostTask(
          FROM_HERE,
          base::Bind(&CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread,
                     this,
                     base::Passed(&frame_copy)));
      break;
    }
  }
}

void CompositorThreadProxy::DidSwapCompositorFrame(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  for (auto it = returned_frames.begin(); it != returned_frames.end(); ++it) {
    // This is a CHECK because CompositorFrameHandle isn't actually thread safe
    // and we're going to clear some of its members. There are potential
    // security implications of there being a reference outside of the
    // compositor now
    CHECK((*it)->HasOneRef())
        << "Returned a frame that's still referenced from outside of the "
        << "compositor";
    DCHECK((*it)->proxy_ == this) << "Frame returned to wrong compositor";
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, std::move(returned_frames)));
}

void CompositorThreadProxy::ReclaimResourcesForFrame(
    CompositorFrameData* frame) {
  DCHECK(frame);

  scoped_ptr<CompositorFrameData> frame_copy =
      CompositorFrameData::AllocFrom(frame);

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread,
        this, base::Passed(&frame_copy)));
}

} // namespace oxide
