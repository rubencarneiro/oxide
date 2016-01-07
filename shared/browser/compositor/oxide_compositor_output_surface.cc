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

#include "oxide_compositor_output_surface.h"

#include <utility>

#include "cc/output/compositor_frame_ack.h"
#include "cc/output/output_surface_client.h"

#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_thread_proxy.h"

namespace oxide {

bool CompositorOutputSurface::BindToClient(cc::OutputSurfaceClient* client) {
  DCHECK(CalledOnValidThread());
  if (!cc::OutputSurface::BindToClient(client)) {
    return false;
  }

  proxy_->SetOutputSurface(this);
  return true;
}

void CompositorOutputSurface::DetachFromClient() {
  DCHECK(CalledOnValidThread());
  proxy_->SetOutputSurface(nullptr);
  cc::OutputSurface::DetachFromClient();
  DetachFromThread();
}

CompositorOutputSurface::CompositorOutputSurface(
    uint32_t surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    scoped_refptr<CompositorThreadProxy> proxy)
    : cc::OutputSurface(context_provider),
      proxy_(proxy),
      surface_id_(surface_id) {
  DetachFromThread();
}

CompositorOutputSurface::CompositorOutputSurface(
    uint32_t surface_id,
    scoped_ptr<cc::SoftwareOutputDevice> software_device,
    scoped_refptr<CompositorThreadProxy> proxy)
    : cc::OutputSurface(std::move(software_device)),
      proxy_(proxy),
      surface_id_(surface_id) {
  DetachFromThread();
}

void CompositorOutputSurface::DoSwapBuffers(CompositorFrameData* frame) {
  DCHECK(CalledOnValidThread());
  DCHECK(frame->gl_frame_data || frame->software_frame_data);

  frame->surface_id = surface_id();

  proxy_->SwapCompositorFrame(frame);
  client_->DidSwapBuffers();
}

CompositorOutputSurface::~CompositorOutputSurface() {}

void CompositorOutputSurface::DidSwapBuffers() {
  DCHECK(CalledOnValidThread());
  client_->DidSwapBuffersComplete();
}

void CompositorOutputSurface::ReclaimResources(const CompositorFrameAck& ack) {
  DCHECK(CalledOnValidThread());
  cc::CompositorFrameAck unused; // Not used for the GL or software renderer
  cc::OutputSurface::ReclaimResources(&unused);
}

} // namespace oxide
