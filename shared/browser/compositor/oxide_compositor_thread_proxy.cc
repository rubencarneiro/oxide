// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/output/gl_frame_data.h"
#include "cc/output/software_frame_data.h"
#include "content/common/host_shared_bitmap_manager.h"

#include "oxide_compositor.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_utils.h"

namespace oxide {

CompositorThreadProxy::~CompositorThreadProxy() {}

void CompositorThreadProxy::DidSkipSwapCompositorFrame(
    uint32 surface_id,
    scoped_refptr<CompositorFrameHandle>* frame) {
  FrameHandleVector frames;
  if (frame) {
    frames.push_back(*frame);
    *frame = nullptr;
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, frames));
}

void CompositorThreadProxy::SendSwapGLFrameOnOwnerThread(
    uint32 surface_id,
    const gfx::Size& size,
    float scale,
    scoped_ptr<GLFrameData> gl_frame_data) {
  if (!gl_frame_data) {
    DidSkipSwapCompositorFrame(surface_id, nullptr);
    return;
  }

  scoped_refptr<CompositorFrameHandle> frame(
      new CompositorFrameHandle(surface_id, this, size, scale));
  frame->gl_frame_data_ = gl_frame_data.Pass();

  if (!owner().compositor) {
    DidSkipSwapCompositorFrame(surface_id, &frame);
    return;
  }

  owner().compositor->SendSwapCompositorFrameToClient(surface_id, frame.get());
}

void CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread(
    uint32 surface_id,
    const gfx::Size& size,
    float scale,
    unsigned id,
    const gfx::Rect& damage_rect,
    const cc::SharedBitmapId& bitmap_id) {
  scoped_ptr<cc::SharedBitmap> bitmap(
      content::HostSharedBitmapManager::current()->GetSharedBitmapFromId(
        size, bitmap_id));
  if (!bitmap) {
    DidSkipSwapCompositorFrame(surface_id, nullptr);
    return;
  }

  scoped_refptr<CompositorFrameHandle> frame(
      new CompositorFrameHandle(surface_id, this, size, scale));
  frame->software_frame_data_.reset(
      new SoftwareFrameData(id, damage_rect, bitmap->pixels()));

  if (!owner().compositor) {
    DidSkipSwapCompositorFrame(surface_id, &frame);
    return;
  }

  owner().compositor->SendSwapCompositorFrameToClient(surface_id, frame.get());
}

void CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread(
    uint32 surface_id,
    FrameHandleVector returned_frames) {
  if (!impl().output) {
    return;
  }

  if (surface_id == impl().output->surface_id()) {
    impl().output->DidSwapBuffers();
  }

  while (!returned_frames.empty()) {
    scoped_refptr<CompositorFrameHandle> frame(returned_frames.back());
    returned_frames.pop_back();

    if (!frame.get()) {
      continue;
    }

    cc::CompositorFrameAck ack;
    if (frame->gl_frame_data()) {
      scoped_ptr<GLFrameData> gl_frame_data = frame->gl_frame_data_.Pass();
      ack.gl_frame_data.reset(new cc::GLFrameData());
      ack.gl_frame_data->mailbox = gl_frame_data->mailbox();
      ack.gl_frame_data->size = frame->size_in_pixels();
    } else if (frame->software_frame_data()) {
      scoped_ptr<SoftwareFrameData> software_frame_data =
          frame->software_frame_data_.Pass();
      ack.last_software_frame_id = software_frame_data->id();
    } else {
      NOTREACHED();
    }

    if (frame->surface_id_ == impl().output->surface_id()) {
      impl().output->ReclaimResources(ack);
    }
  }
}

void CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread(
    uint32 surface_id,
    const gfx::Size& size_in_pixels,
    scoped_ptr<GLFrameData> gl_frame_data,
    scoped_ptr<SoftwareFrameData> software_frame_data) {
  if (!impl().output) {
    return;
  }

  if (surface_id != impl().output->surface_id()) {
    return;
  }

  cc::CompositorFrameAck ack;
  if (gl_frame_data.get()) {
    ack.gl_frame_data.reset(new cc::GLFrameData());
    ack.gl_frame_data->mailbox = gl_frame_data->mailbox();
    ack.gl_frame_data->size = size_in_pixels;
  } else if (software_frame_data.get()) {
    ack.last_software_frame_id = software_frame_data->id();
  } else {
    NOTREACHED();
  }

  impl().output->ReclaimResources(ack);
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
    : owner_message_loop_(base::MessageLoopProxy::current()) {
  owner().compositor = compositor;
  impl_thread_checker_.DetachFromThread();
}

void CompositorThreadProxy::CompositorDestroyed() {
  owner().compositor = nullptr;
}

void CompositorThreadProxy::SetOutputSurface(CompositorOutputSurface* output) {
  DCHECK(!output || !impl().output);
  impl().output = output;
  impl_message_loop_ = base::MessageLoopProxy::current();
}

void CompositorThreadProxy::SwapCompositorFrame(cc::CompositorFrame* frame) {
  if (frame->gl_frame_data) {
    cc::GLFrameData* gl_frame_data = frame->gl_frame_data.get();

    CompositorUtils::GetInstance()->CreateGLFrameHandle(
        impl().output->context_provider(),
        gl_frame_data->mailbox,
        gl_frame_data->sync_point,
        base::Bind(
          &CompositorThreadProxy::SendSwapGLFrameOnOwnerThread,
          this, impl().output->surface_id(),
          gl_frame_data->size, frame->metadata.device_scale_factor),
        owner_message_loop_);
  } else {
    DCHECK(frame->software_frame_data);
    cc::SoftwareFrameData* software_frame_data = frame->software_frame_data.get();

    owner_message_loop_->PostTask(
        FROM_HERE,
        base::Bind(&CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread,
                   this, impl().output->surface_id(),
                   software_frame_data->size,
                   frame->metadata.device_scale_factor,
                   software_frame_data->id,
                   software_frame_data->damage_rect,
                   software_frame_data->bitmap_id));
  }
}

void CompositorThreadProxy::DidSwapCompositorFrame(
    uint32 surface_id,
    FrameHandleVector* returned_frames) {
  FrameHandleVector frames;
  std::swap(frames, *returned_frames);

  for (FrameHandleVector::iterator it = frames.begin();
       it != frames.end(); ++it) {
    CHECK((*it)->HasOneRef()) <<
        "Returned a frame that's still referenced from outside of the "
        "compositor";
    DCHECK((*it)->proxy_ == this) << "Frame returned to wrong compositor";
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, frames));
}

void CompositorThreadProxy::ReclaimResourcesForFrame(
    CompositorFrameHandle* frame) {
  DCHECK(frame) << "Null frame";
  DCHECK(frame->proxy_ == this) << "Frame returned to wrong compositor";

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread,
        this, frame->surface_id_, frame->size_in_pixels(),
        base::Passed(&frame->gl_frame_data_),
        base::Passed(&frame->software_frame_data_)));
}

} // namespace oxide
