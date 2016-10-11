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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_SOFTWARE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_SOFTWARE_H_

#include <memory>

#include "base/macros.h"

#include "shared/browser/compositor/oxide_compositor_output_surface.h"

namespace oxide {

class CompositorOutputSurfaceSoftware : public CompositorOutputSurface {
 public:
  CompositorOutputSurfaceSoftware(
      uint32_t surface_id,
      std::unique_ptr<cc::SoftwareOutputDevice> software_device,
      CompositorOutputSurfaceListener* listener);
  ~CompositorOutputSurfaceSoftware() override;

 private:
  // cc::OutputSurface implementation
  void EnsureBackbuffer() override;
  void DiscardBackbuffer() override;
  void BindFramebuffer() override;
  void Reshape(const gfx::Size& size,
               float device_scale_factor,
               const gfx::ColorSpace& color_space,
               bool has_alpha) override;
  void SwapBuffers(cc::OutputSurfaceFrame frame) override;

  // CompositorOutputSurface implementation
  void ReclaimResources(const CompositorFrameAck& ack) override;

  DISALLOW_COPY_AND_ASSIGN(CompositorOutputSurfaceSoftware);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_SOFTWARE_H_
