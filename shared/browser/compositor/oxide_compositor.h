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
#include "base/memory/weak_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "cc/trees/layer_tree_host_client.h"
#include "ui/gfx/geometry/size.h"

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

class Compositor final : public cc::LayerTreeHostClient,
                         public base::NonThreadSafe  {
 public:
  typedef std::vector<scoped_refptr<CompositorFrameHandle> > FrameHandleVector;

  static scoped_ptr<Compositor> Create(CompositorClient* client);
  ~Compositor();

  bool IsActive() const;

  void SetVisibility(bool visible);
  void SetDeviceScaleFactor(float scale);
  void SetViewportSize(const gfx::Size& bounds);
  void SetRootLayer(scoped_refptr<cc::Layer> layer);

  void DidSwapCompositorFrame(uint32 surface_id,
                              FrameHandleVector* returned_frames);

 private:
  friend class CompositorLock;
  friend class CompositorThreadProxy;

  Compositor(CompositorClient* client);

  void SendSwapCompositorFrameToClient(uint32 surface_id,
                                       CompositorFrameHandle* frame);

  void LockCompositor();
  void UnlockCompositor();

  scoped_ptr<cc::OutputSurface> CreateOutputSurface();

  // cc::LayerTreeHostClient implementation
  void WillBeginMainFrame() final;
  void BeginMainFrame(const cc::BeginFrameArgs& args) final;
  void BeginMainFrameNotExpectedSoon() final;
  void DidBeginMainFrame() final;
  void Layout() final;
  void ApplyViewportDeltas(const gfx::Vector2dF& inner_delta,
                           const gfx::Vector2dF& outer_delta,
                           const gfx::Vector2dF& elastic_overscroll_delta,
                           float page_scale,
                           float top_controls_delta) final;
  void ApplyViewportDeltas(const gfx::Vector2d& scroll_delta,
                           float page_scale,
                           float top_controls_delta) final;
  void RequestNewOutputSurface() final;
  void DidInitializeOutputSurface() final;
  void DidFailToInitializeOutputSurface() final;
  void WillCommit() final;
  void DidCommit() final;
  void DidCommitAndDrawFrame() final;
  void DidCompleteSwapBuffers() final;
  void DidCompletePageScaleAnimation() final;

  CompositorClient* client_;

  int num_failed_recreate_attempts_;

  gfx::Size size_;
  float device_scale_factor_;

  scoped_refptr<cc::Layer> root_layer_;

  scoped_ptr<cc::LayerTreeHost> layer_tree_host_;
  scoped_refptr<CompositorThreadProxy> proxy_;

  uint32 next_output_surface_id_;

  uint32 lock_count_;

  base::WeakPtrFactory<Compositor> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

class CompositorLock final {
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
