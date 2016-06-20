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

#include "base/logging.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/output_surface_client.h"

#include "oxide_compositor.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_output_surface_listener.h"

namespace oxide {

CompositorOutputSurface::CompositorOutputSurface(
    uint32_t surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    std::unique_ptr<cc::SoftwareOutputDevice> software_device,
    CompositorOutputSurfaceListener* listener)
    : cc::OutputSurface(context_provider, nullptr, std::move(software_device)),
      listener_(listener),
      surface_id_(surface_id) {}

void CompositorOutputSurface::DoSwapBuffers(
    std::unique_ptr<CompositorFrameData> frame) {
  DCHECK(frame->gl_frame_data || frame->software_frame_data);

  listener_->SwapCompositorFrame(std::move(frame));
  client_->DidSwapBuffers();
}

bool CompositorOutputSurface::BindToClient(cc::OutputSurfaceClient* client) {
  if (!cc::OutputSurface::BindToClient(client)) {
    return false;
  }

  listener_->OutputSurfaceBound(this);
  return true;
}

CompositorOutputSurface::~CompositorOutputSurface() {
  listener_->OutputSurfaceDestroyed(this);
}

void CompositorOutputSurface::DidSwapBuffers() {
  client_->DidSwapBuffersComplete();
}

void CompositorOutputSurface::ReclaimResources(const CompositorFrameAck& ack) {
  cc::CompositorFrameAck unused; // Not used for the GL or software renderer
  cc::OutputSurface::ReclaimResources(&unused);
}

} // namespace oxide
