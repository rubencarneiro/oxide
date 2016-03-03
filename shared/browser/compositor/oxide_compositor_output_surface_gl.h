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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_

#include <deque>
#include <queue>

#include "base/macros.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositor_output_surface.h"

namespace oxide {

class CompositorOutputSurfaceGL : public CompositorOutputSurface {
 public:
  CompositorOutputSurfaceGL(
      uint32_t surface_id,
      scoped_refptr<cc::ContextProvider> context_provider,
      scoped_refptr<CompositorProxy> proxy);
  ~CompositorOutputSurfaceGL();

 private:
  // cc::OutputSurface implementation
  void EnsureBackbuffer() override;
  void DiscardBackbuffer() override;
  void Reshape(const gfx::Size& size,
               float scale_factor,
               bool has_alpha) override;
  void BindFramebuffer() override;
  void SwapBuffers(cc::CompositorFrame* frame) override;

  // CompositorOutputSurface implementation
  void ReclaimResources(const CompositorFrameAck& ack) override;

  struct BufferData {
    BufferData() : texture_id(0), available(true) {}

    uint32_t texture_id;
    gpu::Mailbox mailbox;
    gfx::Size size;
    bool available;
  };

  BufferData& GetBufferDataForMailbox(const gpu::Mailbox& mailbox);
  void DiscardBufferIfPossible(BufferData* buffer);

  BufferData* back_buffer_;
  std::array<BufferData, 2> buffers_;

  bool is_backbuffer_discarded_;
  uint32_t fbo_;

  DISALLOW_COPY_AND_ASSIGN(CompositorOutputSurfaceGL);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_GL_H_
