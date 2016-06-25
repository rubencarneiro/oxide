// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
#define _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_

#include <memory>
#include <queue>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/resources/returned_resource.h"
#include "cc/surfaces/surface_factory.h"
#include "cc/surfaces/surface_factory_client.h"
#include "cc/surfaces/surface_id.h"
#include "content/browser/renderer_host/render_widget_host_view_oxide.h" // nogncheck
#include "content/common/cursors/webcursor.h" // nogncheck
#include "ui/gfx/geometry/size.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/input/oxide_ime_bridge_impl.h"
#include "shared/browser/oxide_gesture_provider.h"

namespace cc {
class SurfaceFactory;
class SurfaceIdAllocator;
class SurfaceLayer;
}

namespace content {
struct ContextMenuParams;
class RenderWidgetHostImpl;
}

namespace ui {
class MotionEvent;
class TouchHandleDrawable;
class TouchSelectionController;
}

namespace oxide {

class RenderWidgetHostViewContainer;

class RenderWidgetHostView
    : public content::RenderWidgetHostViewOxide,
      public CompositorObserver,
      public GestureProviderClient,
      public cc::SurfaceFactoryClient,
      public ui::TouchSelectionControllerClient,
      public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  RenderWidgetHostView(content::RenderWidgetHostImpl* render_widget_host);
  ~RenderWidgetHostView();

  void SetContainer(RenderWidgetHostViewContainer* container);

  ImeBridgeImpl* ime_bridge() { return &ime_bridge_; }

  const base::string16& selection_text() const {
    return selection_text_;
  }

  const gfx::Range& selection_range() const {
    return selection_range_;
  }

  const cc::CompositorFrameMetadata& last_submitted_frame_metadata() const {
    return last_submitted_frame_metadata_;
  }

  const content::WebCursor& current_cursor() const { return current_cursor_; }

  void HandleTouchEvent(const ui::MotionEvent& event);
  void ResetGestureDetection();

  void Blur();

  // content::RenderWidgetHostView implementation
  content::RenderWidgetHost* GetRenderWidgetHost() const override;
  void SetSize(const gfx::Size& size) override;
  void SetBounds(const gfx::Rect& rect) override;
  void Focus() override;
  void Show() override;
  void Hide() override;

  ui::TouchSelectionController* selection_controller() const {
    return selection_controller_.get();
  }

  void OnUserInput() const;

  bool HandleContextMenu(const content::ContextMenuParams& params);

 private:
  // content::RenderWidgetHostViewOxide implementation
  void OnSelectionBoundsChanged(const gfx::Rect& anchor_rect,
                                const gfx::Rect& focus_rect,
                                bool is_anchor_first) override;

  // content::RenderWidgetHostViewBase implementation
  void SelectionChanged(const base::string16& text,
                        size_t offset,
                        const gfx::Range& range) override;
  gfx::Size GetPhysicalBackingSize() const override;
  bool DoTopControlsShrinkBlinkSize() const override;
  float GetTopControlsHeight() const override;
  void FocusedNodeChanged(bool is_editable_node) override;
  void OnSwapCompositorFrame(
      uint32_t output_surface_id,
      std::unique_ptr<cc::CompositorFrame> frame) override;
  void ClearCompositorFrame() override;
  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) override;
  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) override;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) override;
  void UpdateCursor(const content::WebCursor& cursor) override;
  void SetIsLoading(bool is_loading) override;
  void TextInputStateChanged(const content::TextInputState& params) override;
  void ImeCancelComposition() override;
  void RenderProcessGone(base::TerminationStatus status,
                         int error_code) override;
  void Destroy() override;
  void SetTooltipText(const base::string16& tooltip_text) override;
  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      const content::ReadbackRequestCallback& callback,
      const SkColorType color_type) override;
  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(const gfx::Rect&, bool)>& callback) override;
  bool CanCopyToVideoFrame() const override;
  bool HasAcceleratedSurface(const gfx::Size& desired_size) override;
  void GetScreenInfo(blink::WebScreenInfo* results) override;
  gfx::Rect GetBoundsInRootWindow() override;
  void ShowDisambiguationPopup(const gfx::Rect& rect_pixels,
                               const SkBitmap& zoomed_bitmap) override;
  void LockCompositingSurface() override;
  void UnlockCompositingSurface() override;
  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) override;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) override;
  gfx::Vector2dF GetLastScrollOffset() const override;
  gfx::NativeView GetNativeView() const override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  bool HasFocus() const override;
  bool IsSurfaceAvailableForCopy() const override;
  bool IsShowing() override;
  gfx::Rect GetViewBounds() const override;
  bool LockMouse() override;
  void UnlockMouse() override;

  // CompositorObserver implementation
  void CompositorDidCommit() override;
  void CompositorWillRequestSwapFrame() override;
  void CompositorEvictResources() override;

  // GestureProviderClient implementation
  void OnGestureEvent(const blink::WebGestureEvent& event) override;

  // cc::SurfaceFactoryClient implementation
  void ReturnResources(const cc::ReturnedResourceArray& resources) override;
  void SetBeginFrameSource(cc::BeginFrameSource* begin_frame_source) override;

  // ui::TouchSelectionControllerClient implementation
  bool SupportsAnimation() const override;
  void SetNeedsAnimate() override;
  void MoveCaret(const gfx::PointF& position) override;
  void MoveRangeSelectionExtent(const gfx::PointF& extent) override;
  void SelectBetweenCoordinates(const gfx::PointF& base,
                                const gfx::PointF& extent) override;
  void OnSelectionEvent(ui::SelectionEventType event) override;
  std::unique_ptr<ui::TouchHandleDrawable> CreateDrawable() override;

  // ===================

  void UpdateCurrentCursor();

  void DestroyDelegatedContent();
  void SendDelegatedFrameAck(uint32_t surface_id);
  void SendReturnedDelegatedResources();
  void RunAckCallbacks(cc::SurfaceDrawStatus status);
  void AttachLayer();
  void DetachLayer();

  bool HandleGestureForTouchSelection(const blink::WebGestureEvent& event) const;
  void NotifyTouchSelectionChanged(bool insertion_handle_tapped);

  content::RenderWidgetHostImpl* host_;

  RenderWidgetHostViewContainer* container_;

  scoped_refptr<cc::SurfaceLayer> layer_;
  std::unique_ptr<cc::SurfaceIdAllocator> id_allocator_;
  std::unique_ptr<cc::SurfaceFactory> surface_factory_;
  cc::SurfaceId surface_id_;
  cc::ReturnedResourceArray surface_returned_resources_;

  // The output surface ID for the last frame from the renderer
  uint32_t last_output_surface_id_;

  std::queue<base::Closure> ack_callbacks_;

  gfx::Size last_frame_size_dip_;

  ImeBridgeImpl ime_bridge_;

  bool is_loading_;
  content::WebCursor web_cursor_;
  content::WebCursor current_cursor_;

  bool is_showing_;
  gfx::Size last_size_;

  cc::CompositorFrameMetadata last_submitted_frame_metadata_;
  cc::CompositorFrameMetadata committed_frame_metadata_;
  cc::CompositorFrameMetadata displayed_frame_metadata_;

  bool top_controls_shrink_blink_size_;

  std::unique_ptr<GestureProvider> gesture_provider_;

  std::unique_ptr<ui::TouchSelectionController> selection_controller_;
  bool handle_drag_in_progress_;

  base::WeakPtrFactory<RenderWidgetHostView> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
