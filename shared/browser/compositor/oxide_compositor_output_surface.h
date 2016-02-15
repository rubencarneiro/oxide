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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/threading/non_thread_safe.h"
#include "cc/output/output_surface.h"

namespace cc {
class ContextProvider;
}

namespace oxide {

class CompositorFrameAck;
class CompositorFrameData;
class CompositorThreadProxy;

class CompositorOutputSurface : public cc::OutputSurface,
                                public base::NonThreadSafe {
 public:
  virtual ~CompositorOutputSurface();

  uint32_t surface_id() const { return surface_id_; }

  void DidSwapBuffers();
  virtual void ReclaimResources(const CompositorFrameAck& ack);

 protected:
  CompositorOutputSurface(
      uint32_t surface_id,
      scoped_refptr<cc::ContextProvider> context_provider,
      scoped_refptr<CompositorThreadProxy> proxy);
  CompositorOutputSurface(
      uint32_t surface_id,
      scoped_ptr<cc::SoftwareOutputDevice> software_device,
      scoped_refptr<CompositorThreadProxy> proxy);

  scoped_refptr<CompositorThreadProxy> proxy_;

  void DoSwapBuffers(CompositorFrameData* frame);

  // cc::OutputSurface implementation
  bool BindToClient(cc::OutputSurfaceClient* client) override;
  void DetachFromDisplayClient() override;

 private:
  uint32_t surface_id_;

  DISALLOW_COPY_AND_ASSIGN(CompositorOutputSurface);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_H_
