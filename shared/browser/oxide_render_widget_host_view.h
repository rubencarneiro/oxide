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
#include "cc/surfaces/frame_sink_id.h"
#include "cc/surfaces/local_surface_id.h"
#include "cc/surfaces/surface_factory_client.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h" // nogncheck
#include "content/browser/renderer_host/text_input_manager.h" // nogncheck
#include "content/common/cursors/webcursor.h" // nogncheck
#include "ui/gfx/geometry/size.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/browser_object_weak_ptrs.h"
#include "shared/browser/oxide_gesture_provider.h"

namespace cc {
class SurfaceFactory;
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
    : public content::RenderWidgetHostViewBase,
      public CompositorObserver,
      public GestureProviderClient,
      public cc::SurfaceFactoryClient,
      public content::TextInputManager::Observer,
      public ui::TouchSelectionControllerClient,
      public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  RenderWidgetHostView(content::RenderWidgetHostImpl* render_widget_host);
  ~RenderWidgetHostView();

  void SetContainer(RenderWidgetHostViewContainer* container);

  const cc::CompositorFrameMetadata& last_drawn_frame_metadata() const {
    return last_drawn_frame_metadata_;
  }

  const content::WebCursor& current_cursor() const { return current_cursor_; }

  void HandleTouchEvent(const ui::MotionEvent& event);
  void ResetGestureDetection();

  void Blur();

  content::RenderWidgetHostViewBase* GetFocusedViewForTextSelection() const;

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

  bool selection_handle_drag_in_progress() const {
    return selection_handle_drag_in_progress_;
  }

  void OnUserInput() const;

 private:
  // content::RenderWidgetHostViewBase implementation
  gfx::Size GetPhysicalBackingSize() const override;
  bool DoBrowserControlsShrinkBlinkSize() const override;
  float GetTopControlsHeight() const override;
  void FocusedNodeChanged(bool is_editable_node,
                          const gfx::Rect& node_bounds_in_screen) override;
  void DidCreateNewRendererCompositorFrameSink(
      cc::mojom::MojoCompositorFrameSinkClient* renderer_compositor_frame_sink) override;
  void SubmitCompositorFrame(const cc::LocalSurfaceId& local_surface_id,
                             cc::CompositorFrame frame) override;
  void ClearCompositorFrame() override;
  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) override;
  cc::FrameSinkId GetFrameSinkId() override;
  cc::FrameSinkId FrameSinkIdAtPoint(cc::SurfaceHittestDelegate* delegate,
                                     const gfx::Point& point,
                                     gfx::Point* transformed_point) override;
  void ProcessMouseEvent(const blink::WebMouseEvent& event,
                         const ui::LatencyInfo& latency) override;
  void ProcessMouseWheelEvent(const blink::WebMouseWheelEvent& event,
                              const ui::LatencyInfo& latency) override;
  void ProcessTouchEvent(const blink::WebTouchEvent& event,
                         const ui::LatencyInfo& latency) override;
  void ProcessGestureEvent(const blink::WebGestureEvent& event,
                           const ui::LatencyInfo& latency) override;
  bool TransformPointToLocalCoordSpace(
      const gfx::Point& point,
      const cc::SurfaceId& original_surface,
      gfx::Point* transformed_point) override;
  bool TransformPointToCoordSpaceForView(
      const gfx::Point& point,
      content::RenderWidgetHostViewBase* target_view,
      gfx::Point* transformed_point) override;
  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) override;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) override;
  void UpdateCursor(const content::WebCursor& cursor) override;
  void SetIsLoading(bool is_loading) override;
  void RenderProcessGone(base::TerminationStatus status,
                         int error_code) override;
  void Destroy() override;
  void SetTooltipText(const base::string16& tooltip_text) override;
  bool HasAcceleratedSurface(const gfx::Size& desired_size) override;
  gfx::Rect GetBoundsInRootWindow() override;
  void ShowDisambiguationPopup(const gfx::Rect& rect_pixels,
                               const SkBitmap& zoomed_bitmap) override;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) override;
  gfx::Vector2dF GetLastScrollOffset() const override;
  gfx::NativeView GetNativeView() const override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  bool HasFocus() const override;
  bool IsSurfaceAvailableForCopy() const override;
  bool IsShowing() override;
  gfx::Rect GetViewBounds() const override;
  void SetBackgroundColor(SkColor color) override;
  SkColor background_color() const override;
  bool LockMouse() override;
  void UnlockMouse() override;
  void SetNeedsBeginFrames(bool needs_begin_frames) override;

  // CompositorObserver implementation
  void CompositorEvictResources() override;

  // GestureProviderClient implementation
  void OnGestureEvent(blink::WebGestureEvent event) override;

  // cc::SurfaceFactoryClient implementation
  void ReturnResources(const cc::ReturnedResourceArray& resources) override;
  void SetBeginFrameSource(cc::BeginFrameSource* begin_frame_source) override;

  // content::TextInputManager::Observer implementation
  void OnUpdateTextInputStateCalled(
      content::TextInputManager* text_input_manager,
      content::RenderWidgetHostViewBase* updated_view,
      bool did_update_state) override;
  void OnImeCancelComposition(
      content::TextInputManager* text_input_manager,
      content::RenderWidgetHostViewBase* updated_view) override;
  void OnSelectionBoundsChanged(
      content::TextInputManager* text_input_manager,
      content::RenderWidgetHostViewBase* updated_view) override;
  void OnTextSelectionChanged(
      content::TextInputManager* text_input_manager,
      content::RenderWidgetHostViewBase* updated_view) override;

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
  void SendDelegatedFrameAck();
  void SendReturnedDelegatedResources();
  void SurfaceDrawn(const cc::LocalSurfaceId& id,
                    cc::CompositorFrameMetadata metadata);
  void RunAckCallbacks();
  void AttachLayer();
  void DetachLayer();

  void HandleGestureForTouchSelection(const blink::WebGestureEvent& event) const;

  content::RenderWidgetHostImpl* host_;

  RenderWidgetHostViewContainer* container_;

  cc::mojom::MojoCompositorFrameSinkClient* renderer_compositor_frame_sink_;

  scoped_refptr<cc::SurfaceLayer> layer_;
  cc::FrameSinkId frame_sink_id_;
  std::unique_ptr<cc::SurfaceFactory> surface_factory_;
  cc::LocalSurfaceId local_surface_id_;
  cc::ReturnedResourceArray surface_returned_resources_;

  cc::CompositorFrameMetadata last_drawn_frame_metadata_;

  std::queue<base::Closure> ack_callbacks_;

  bool is_loading_;
  content::WebCursor web_cursor_;
  content::WebCursor current_cursor_;

  bool is_showing_;
  gfx::Size last_size_;

  bool browser_controls_shrink_blink_size_;

  std::unique_ptr<GestureProvider> gesture_provider_;

  std::unique_ptr<ui::TouchSelectionController> selection_controller_;
  bool selection_handle_drag_in_progress_;

  base::WeakPtrFactory<RenderWidgetHostView> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
