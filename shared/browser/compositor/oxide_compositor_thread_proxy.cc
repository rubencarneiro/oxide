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

#include "oxide_compositor.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_utils.h"

namespace oxide {

CompositorThreadProxyBase::CompositorThreadProxyBase() {
  DCHECK(owner_thread_checker_.CalledOnValidThread());
}

CompositorThreadProxyBase::~CompositorThreadProxyBase() {}

CompositorThreadProxyBase::OwnerData& CompositorThreadProxyBase::owner() {
  DCHECK(owner_thread_checker_.CalledOnValidThread());
  return owner_;
}

CompositorThreadProxyBase::ImplData& CompositorThreadProxyBase::impl() {
  DCHECK(impl_thread_checker_.CalledOnValidThread());
  return impl_;
}

CompositorThreadProxy::~CompositorThreadProxy() {}

void CompositorThreadProxy::SendSwapCompositorFrameOnOwnerThread(
    uint32 surface_id, scoped_ptr<GLFrameData> gl_frame_data) {
  DCHECK(gl_frame_data);

  scoped_ptr<CompositorFrameHandle> frame(new CompositorFrameHandle());
  frame->surface_id = surface_id;
  frame->proxy = this;
  frame->gl_frame_data_ = gl_frame_data.Pass();

  if (!owner().compositor) {
    DidSwapCompositorFrame(surface_id, frame.Pass());
    return;
  }

  owner().compositor->SendSwapCompositorFrameToClient(surface_id,
                                                      frame.Pass());
}

void CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread(
    uint32 surface_id,
    cc::CompositorFrameAck* ack) {
  if (!impl().output) {
    return;
  }

  impl().output->DidSwapBuffers(surface_id, *ack);
}

void CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread(
    uint32 surface_id,
    cc::CompositorFrameAck* ack) {
  if (!impl().output) {
    return;
  }

  impl().output->ReclaimResources(surface_id, *ack);
}

CompositorThreadProxy::CompositorThreadProxy(Compositor* compositor)
    : owner_message_loop_(base::MessageLoopProxy::current()) {
  owner().compositor = compositor;
}

void CompositorThreadProxy::CompositorDestroyed() {
  owner().compositor = NULL;
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
        gl_frame_data->mailbox,
        gl_frame_data->size,
        frame->metadata.device_scale_factor,
        gl_frame_data->sync_point,
        base::Bind(
          &CompositorThreadProxy::SendSwapCompositorFrameOnOwnerThread,
          this, impl().output->surface_id()),
        owner_message_loop_);
  } else {
    DCHECK(frame->software_frame_data);
    NOTREACHED() << "Software mode not supported yet";
  }
}

void CompositorThreadProxy::DidSwapCompositorFrame(
    uint32 surface_id,
    scoped_ptr<CompositorFrameHandle> returned_frame) {
  DCHECK(!returned_frame || returned_frame->proxy == this) <<
      "Frame returned to the wrong compositor";

  cc::CompositorFrameAck* ack = new cc::CompositorFrameAck();
  if (returned_frame) {
    if (returned_frame->gl_frame_data()) {
      scoped_ptr<GLFrameData> gl_frame_data = returned_frame->gl_frame_data_.Pass();
      ack->gl_frame_data.reset(new cc::GLFrameData());
      ack->gl_frame_data->mailbox = gl_frame_data->mailbox();
      ack->gl_frame_data->size = gl_frame_data->size_in_pixels();
    // } else if (returned_frame->software_frame_data()) {
    } else {
      NOTREACHED() << "Frame already returned to compositor";
    }
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, base::Owned(ack)));
}

void CompositorThreadProxy::ReclaimResourcesForFrame(
    CompositorFrameHandle* frame) {
  DCHECK(frame) << "Null frame";
  DCHECK(frame->proxy == this) << "Frame returned to wrong compositor";

  if (!frame->gl_frame_data()) {
    return;
  }

  cc::CompositorFrameAck* ack = new cc::CompositorFrameAck();
  if (frame->gl_frame_data()) {
    scoped_ptr<GLFrameData> gl_frame_data = frame->gl_frame_data_.Pass();
    ack->gl_frame_data.reset(new cc::GLFrameData());
    ack->gl_frame_data->mailbox = gl_frame_data->mailbox();
    ack->gl_frame_data->size = gl_frame_data->size_in_pixels();
  } else {
    NOTREACHED();
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread,
        this, frame->surface_id, base::Owned(ack)));
}

} // namespace oxide
