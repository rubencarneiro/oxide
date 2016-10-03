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

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "cc/trees/layer_tree_host_client.h"
#include "cc/trees/layer_tree_host_single_thread_client.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"
#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/compositor/oxide_compositor_frame_collector.h"
#include "shared/browser/compositor/oxide_compositor_output_surface_listener.h"
#include "shared/browser/compositor/oxide_mailbox_buffer_map.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace cc {
class Display;
class Layer;
class LayerTreeHostInterface;
class SurfaceIdAllocator;
}

namespace gfx {
class Size;
}

namespace oxide {

class CompositorFrameData;
class CompositorFrameHandle;
class CompositorObserver;

class Compositor : public cc::LayerTreeHostClient,
                   public cc::LayerTreeHostSingleThreadClient,
                   public CompositorOutputSurfaceListener,
                   public CompositorFrameCollector {
 public:
  static std::unique_ptr<Compositor> Create(CompositorClient* client);
  ~Compositor() override;

  void SetVisibility(bool visible);
  void SetDeviceScaleFactor(float scale);
  void SetViewportSize(const gfx::Size& bounds);
  void SetRootLayer(scoped_refptr<cc::Layer> layer);
  void SetNeedsRedraw();

 private:
  friend class CompositorObserver;

  Compositor(CompositorClient* client);

  bool SurfaceIdIsCurrent(uint32_t surface_id);

  void DidCompleteGLFrame(uint32_t surface_id,
                          std::unique_ptr<CompositorFrameData> frame);
  void ContinueSwapGLFrame(uint32_t surface_id,
                           std::unique_ptr<CompositorFrameData> frame);
  void QueueGLFrameSwap(std::unique_ptr<CompositorFrameData> frame);
  void DispatchQueuedGLFrameSwaps();

  void SendSwapCompositorFrameToClient(std::unique_ptr<CompositorFrameData> frame);

  using FrameHandleVector = std::vector<scoped_refptr<CompositorFrameHandle>>;
  static void SwapCompositorFrameAckFromClientThunk(
      base::WeakPtr<Compositor> compositor,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner,
      uint32_t surface_id,
      FrameHandleVector returned_frames);
  void SwapCompositorFrameAckFromClient(uint32_t surface_id,
                                        FrameHandleVector returned_frames);

  void OutputSurfaceChanged();

  std::unique_ptr<cc::OutputSurface> CreateOutputSurface();

  void AddObserver(CompositorObserver* observer);
  void RemoveObserver(CompositorObserver* observer);

  void EnsureLayerTreeHost();
  void MaybeEvictLayerTreeHost();

  // Response from CompositorUtils::GetTextureFromMailbox
  void GetTextureFromMailboxResponse(uint32_t surface_id,
                                     const gpu::Mailbox& mailbox,
                                     GLuint texture);
  // Response from CompositorUtils::CreateEGLImageFromMailbox
  static void CreateEGLImageFromMailboxResponseThunk(
      base::WeakPtr<Compositor> compositor,
      uint32_t surface_id,
      const gpu::Mailbox& mailbox,
      EGLImageKHR egl_image);
  void CreateEGLImageFromMailboxResponse(uint32_t surface_id,
                                         const gpu::Mailbox& mailbox,
                                         EGLImageKHR egl_image);

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
  void DidCompletePageScaleAnimation() override;

  // cc::LayerTreeHostSingleThreadClient implementation
  void DidPostSwapBuffers() override;
  void DidAbortSwapBuffers() override;

  // CompositorOutputSurfaceListener implementation
  void OutputSurfaceBound(CompositorOutputSurface* output_surface) override;
  void OutputSurfaceDestroyed(CompositorOutputSurface* output_surface) override;
  void MailboxBufferCreated(const gpu::Mailbox& mailbox,
                            uint64_t sync_point) override;
  void MailboxBufferDestroyed(const gpu::Mailbox& mailbox) override;
  void SwapCompositorFrame(std::unique_ptr<CompositorFrameData> frame) override;
  void AllFramesReturnedFromClient() override;

  // CompositorFrameCollector implementation
  void ReclaimResourcesForFrame(uint32_t surface_id,
                                CompositorFrameData* frame) override;

  CompositingMode mode_;

  CompositorClient* client_;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  CompositorOutputSurface* output_surface_;

  MailboxBufferMap mailbox_buffer_map_;

  std::queue<std::unique_ptr<CompositorFrameData>> queued_gl_frame_swaps_;

  // Needs to outlive |display_client_|, as its destructor results in our
  // OutputSurface calling in to OutputSurfaceDestroyed
  CompositorClient::SwapAckCallback swap_ack_callback_;

  // Both of these need to outlive |layer_tree_host_|
  std::unique_ptr<cc::SurfaceIdAllocator> surface_id_allocator_;
  std::unique_ptr<cc::Display> display_;

  bool layer_tree_host_eviction_pending_;
  bool can_evict_layer_tree_host_;

  std::unique_ptr<cc::LayerTreeHostInterface> layer_tree_host_;

  int num_failed_recreate_attempts_;

  bool visible_;
  gfx::Size size_;
  float device_scale_factor_;

  scoped_refptr<cc::Layer> root_layer_;

  uint32_t next_output_surface_id_;

  base::ObserverList<CompositorObserver> observers_;

  int pending_swaps_;
  int frames_waiting_for_completion_;
  int mailbox_resource_fetches_in_progress_;

  base::WeakPtrFactory<Compositor> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Compositor);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_H_
