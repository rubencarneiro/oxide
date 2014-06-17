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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_

#include <deque>
#include <queue>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/size.h"

#include "shared/browser/compositor/oxide_compositor_output_surface.h"

namespace oxide {

class CompositorOutputSurfaceGL FINAL : public CompositorOutputSurface {
 public:
  CompositorOutputSurfaceGL(
      uint32 surface_id,
      scoped_refptr<cc::ContextProvider> context_provider,
      scoped_refptr<CompositorThreadProxy> proxy);
  ~CompositorOutputSurfaceGL();

 private:
  // cc::OutputSurface implementation
  void EnsureBackbuffer() FINAL;
  void DiscardBackbuffer() FINAL;
  void Reshape(const gfx::Size& size, float scale_factor) FINAL;
  void BindFramebuffer() FINAL;
  void SwapBuffers(cc::CompositorFrame* frame) FINAL;

  // CompositorOutputSurface implementation
  void DidSwapBuffers(uint32 surface_id,
                      const cc::CompositorFrameAck& ack) FINAL;
  void ReclaimResources(uint32 surface_id,
                        const cc::CompositorFrameAck& ack) FINAL;

  void DoReclaim(const cc::CompositorFrameAck& ack);

  struct OutputFrameData {
    OutputFrameData() : texture_id(0), sync_point(0) {}

    uint32 texture_id;
    gpu::Mailbox mailbox;
    gfx::Size size;
    uint32 sync_point;
  };

  OutputFrameData backing_texture_;
  std::deque<OutputFrameData> pending_textures_;
  std::queue<OutputFrameData> returned_textures_;

  bool is_backbuffer_discarded_;
  uint32 fbo_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(CompositorOutputSurfaceGL);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_
