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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/threading/non_thread_safe.h"
#include "cc/trees/layer_tree_host_client.h"
#include "cc/trees/layer_tree_host_single_thread_client.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositor_proxy.h"

namespace cc {
class Layer;
class LayerTreeHost;
class OnscreenDisplayClient;
class SurfaceIdAllocator;
}

namespace gfx {
class Size;
}

namespace oxide {

class CompositorClient;
class CompositorFrameData;
class CompositorFrameHandle;
class CompositorObserver;

class Compositor : public cc::LayerTreeHostClient,
                   public cc::LayerTreeHostSingleThreadClient,
                   public CompositorProxyClient,
                   public base::NonThreadSafe {
 public:
  typedef std::vector<scoped_refptr<CompositorFrameHandle>> FrameHandleVector;

  static scoped_ptr<Compositor> Create(CompositorClient* client);
  ~Compositor() override;

  bool IsActive() const;

  void SetVisibility(bool visible);
  void SetDeviceScaleFactor(float scale);
  void SetViewportSize(const gfx::Size& bounds);
  void SetRootLayer(scoped_refptr<cc::Layer> layer);

  void DidSwapCompositorFrame(uint32_t surface_id,
                              FrameHandleVector returned_frames);

 private:
  friend class CompositorObserver;

  Compositor(CompositorClient* client);

  scoped_ptr<cc::OutputSurface> CreateOutputSurface();

  void AddObserver(CompositorObserver* observer);
  void RemoveObserver(CompositorObserver* observer);

  // cc::LayerTreeHostClient implementation
  void WillBeginMainFrame() override;
  void BeginMainFrame(const cc::BeginFrameArgs& args) override;
  void BeginMainFrameNotExpectedSoon() override;
  void DidBeginMainFrame() override;
  void UpdateLayerTreeHost() override;
  void ApplyViewportDeltas(const gfx::Vector2dF& inner_delta,
                           const gfx::Vector2dF& outer_delta,
                           const gfx::Vector2dF& elastic_overscroll_delta,
                           float page_scale,
                           float top_controls_delta) override;
  void RequestNewOutputSurface() override;
  void DidInitializeOutputSurface() override;
  void DidFailToInitializeOutputSurface() override;
  void WillCommit() override;
  void DidCommit() override;
  void DidCommitAndDrawFrame() override;
  void DidCompleteSwapBuffers() override;
  void RecordFrameTimingEvents(
      scoped_ptr<cc::FrameTimingTracker::CompositeTimingSet> composite_events,
      scoped_ptr<cc::FrameTimingTracker::MainFrameTimingSet> main_frame_events) override;
  void DidCompletePageScaleAnimation() override;

  // cc::LayerTreeHostSingleThreadClient implementation
  void DidPostSwapBuffers() override;
  void DidAbortSwapBuffers() override;

  // CompositorProxyClient
  void SwapCompositorFrameFromProxy(scoped_ptr<CompositorFrameData> frame);
  
  CompositorClient* client_;

  scoped_refptr<CompositorProxy> proxy_;

  scoped_ptr<cc::SurfaceIdAllocator> surface_id_allocator_;

  scoped_ptr<cc::OnscreenDisplayClient> display_client_;

  scoped_ptr<cc::LayerTreeHost> layer_tree_host_;

  int num_failed_recreate_attempts_;

  gfx::Size size_;
  float device_scale_factor_;

  scoped_refptr<cc::Layer> root_layer_;

  uint32_t next_output_surface_id_;

  base::ObserverList<CompositorObserver> observers_;

  int pending_swaps_;

  base::WeakPtrFactory<Compositor> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_
