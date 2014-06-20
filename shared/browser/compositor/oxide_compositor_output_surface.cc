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

#include "oxide_compositor_output_surface.h"

#include "cc/output/output_surface_client.h"

#include "oxide_compositor_thread_proxy.h"

namespace oxide {

bool CompositorOutputSurface::BindToClient(cc::OutputSurfaceClient* client) {
  if (!cc::OutputSurface::BindToClient(client)) {
    return false;
  }

  proxy_->SetOutputSurface(this);
  return true;
}

CompositorOutputSurface::CompositorOutputSurface(
    uint32 surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    scoped_refptr<CompositorThreadProxy> proxy)
    : cc::OutputSurface(context_provider),
      proxy_(proxy),
      surface_id_(surface_id) {}

CompositorOutputSurface::CompositorOutputSurface(
    uint32 surface_id,
    scoped_ptr<cc::SoftwareOutputDevice> software_device,
    scoped_refptr<CompositorThreadProxy> proxy)
    : cc::OutputSurface(software_device.Pass()),
      proxy_(proxy),
      surface_id_(surface_id) {}

void CompositorOutputSurface::SwapBuffers(cc::CompositorFrame* frame) {
  proxy_->SwapCompositorFrame(frame);
  client_->DidSwapBuffers();
}

CompositorOutputSurface::~CompositorOutputSurface() {
  proxy_->SetOutputSurface(NULL);
}

void CompositorOutputSurface::DidSwapBuffers() {
  client_->DidSwapBuffersComplete();
}

void CompositorOutputSurface::ReclaimResources(
    const cc::CompositorFrameAck& ack) {
  cc::OutputSurface::ReclaimResources(&ack);
}

} // namespace oxide
