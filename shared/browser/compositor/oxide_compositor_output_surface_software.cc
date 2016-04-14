// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include "oxide_compositor_output_surface_software.h"

#include <utility>

#include "base/logging.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/output_surface_client.h"

#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_software_output_device.h"

namespace oxide {

void CompositorOutputSurfaceSoftware::SwapBuffers(cc::CompositorFrame* frame) {
  scoped_ptr<CompositorFrameData> data(new CompositorFrameData());
  data->software_frame_data = make_scoped_ptr(new SoftwareFrameData());

  static_cast<CompositorSoftwareOutputDevice*>(software_device())
      ->PopulateFrameDataForSwap(data.get());

  DoSwapBuffers(std::move(data));
}

void CompositorOutputSurfaceSoftware::ReclaimResources(
    const CompositorFrameAck& ack) {
  DCHECK_GT(ack.software_frame_id, 0U);
  DCHECK(ack.gl_frame_mailbox.IsZero());

  static_cast<CompositorSoftwareOutputDevice*>(software_device())
      ->ReclaimResources(ack.software_frame_id);
  CompositorOutputSurface::ReclaimResources(ack);
}

CompositorOutputSurfaceSoftware::CompositorOutputSurfaceSoftware(
    uint32_t surface_id,
    scoped_ptr<cc::SoftwareOutputDevice> software_device,
    CompositorOutputSurfaceListener* listener)
    : CompositorOutputSurface(surface_id,
                              std::move(software_device),
                              listener) {}

CompositorOutputSurfaceSoftware::~CompositorOutputSurfaceSoftware() {}

} // namespace oxide
