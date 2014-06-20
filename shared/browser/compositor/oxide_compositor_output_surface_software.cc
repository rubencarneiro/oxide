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

#include "oxide_compositor_output_surface_software.h"

#include "base/logging.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/output_surface_client.h"

#include "oxide_compositor_thread_proxy.h"

namespace oxide {

void CompositorOutputSurfaceSoftware::SwapBuffers(cc::CompositorFrame* frame) {
  DCHECK(frame->software_frame_data);
  DCHECK_NE(frame->software_frame_data->id, 0);
  DCHECK(!frame->software_frame_data->size.IsEmpty());
  DCHECK(frame->software_frame_data->size == surface_size_);
  CompositorOutputSurface::SwapBuffers(frame);
}

CompositorOutputSurfaceSoftware::CompositorOutputSurfaceSoftware(
    uint32 surface_id,
    scoped_ptr<cc::SoftwareOutputDevice> software_device,
    scoped_refptr<CompositorThreadProxy> proxy)
    : CompositorOutputSurface(surface_id, software_device.Pass(), proxy) {}

CompositorOutputSurfaceSoftware::~CompositorOutputSurfaceSoftware() {}

} // namespace oxide
