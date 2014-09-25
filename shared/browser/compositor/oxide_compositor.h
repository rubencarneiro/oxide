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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/threading/non_thread_safe.h"
#include "cc/trees/layer_tree_host_client.h"
#include "ui/gfx/size.h"

namespace cc {
class Layer;
class LayerTreeHost;
}

namespace gfx {
class Size;
}

namespace oxide {

class CompositorClient;
class CompositorFrameHandle;
class CompositorLock;
class CompositorThreadProxy;

class Compositor FINAL : public cc::LayerTreeHostClient,
                         public base::NonThreadSafe  {
 public:
  typedef std::vector<scoped_refptr<CompositorFrameHandle> > FrameHandleVector;

  static scoped_ptr<Compositor> Create(CompositorClient* client, bool software);
  ~Compositor();

  bool IsActive() const;

  void SetVisibility(bool visible);
  void SetDeviceScaleFactor(float scale);
  void SetViewportSize(const gfx::Size& bounds);
  void SetRootLayer(scoped_refptr<cc::Layer> layer);

  void DidSwapCompositorFrame(uint32 surface_id,
                              FrameHandleVector& returned_frames);

 private:
  friend class CompositorLock;
  friend class CompositorThreadProxy;

  Compositor(CompositorClient* client, bool software);

  void SendSwapCompositorFrameToClient(uint32 surface_id,
                                       CompositorFrameHandle* frame);

  void LockCompositor();
  void UnlockCompositor();

  // cc::LayerTreeHostClient implementation
  void WillBeginMainFrame(int frame_id) FINAL;
  void BeginMainFrame(const cc::BeginFrameArgs& args) FINAL;
  void DidBeginMainFrame() FINAL;
  void Layout() FINAL;
  void ApplyViewportDeltas(const gfx::Vector2d& scroll_delta,
                           float page_scale,
                           float top_controls_delta) FINAL;
  scoped_ptr<cc::OutputSurface> CreateOutputSurface(bool fallback) FINAL;
  void DidInitializeOutputSurface() FINAL;
  void WillCommit() FINAL;
  void DidCommit() FINAL;
  void DidCommitAndDrawFrame() FINAL;
  void DidCompleteSwapBuffers() FINAL;

  CompositorClient* client_;
  bool use_software_;

  gfx::Size size_;
  float device_scale_factor_;

  scoped_refptr<cc::Layer> root_layer_;

  scoped_ptr<cc::LayerTreeHost> layer_tree_host_;
  scoped_refptr<CompositorThreadProxy> proxy_;

  uint32 next_output_surface_id_;

  uint32 lock_count_;

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

class CompositorLock FINAL {
 public:
  CompositorLock(Compositor* compositor)
      : compositor_(compositor) {
    if (compositor_) {
      compositor_->LockCompositor();
    }
  }

  ~CompositorLock() {
    if (compositor_) {
      compositor_->UnlockCompositor();
    }
  }

 private:
  Compositor* compositor_;

  DISALLOW_COPY_AND_ASSIGN(CompositorLock);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_
