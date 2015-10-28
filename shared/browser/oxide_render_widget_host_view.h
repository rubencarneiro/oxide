// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include <queue>
#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "cc/layers/delegated_frame_resource_collection.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/common/cursors/webcursor.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/input/oxide_ime_bridge_impl.h"
#include "shared/browser/oxide_gesture_provider.h"
#include "shared/browser/oxide_renderer_frame_evictor_client.h"
#include "shared/port/content/browser/render_widget_host_view_oxide.h"

namespace cc {
class DelegatedFrameProvider;
class DelegatedRendererLayer;
}

namespace content {
class RenderWidgetHostImpl;
}

namespace ui {
class MotionEvent;
}

namespace oxide {

class RenderWidgetHostViewContainer;

class RenderWidgetHostView final :
    public content::RenderWidgetHostViewOxide,
    public CompositorObserver,
    public GestureProviderClient,
    public RendererFrameEvictorClient,
    public cc::DelegatedFrameResourceCollectionClient,
    public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);
  ~RenderWidgetHostView();

  content::RenderWidgetHostImpl* host() const { return host_; }

  void SetContainer(RenderWidgetHostViewContainer* container);

  ImeBridgeImpl* ime_bridge() { return &ime_bridge_; }

  const base::string16& selection_text() const {
    return selection_text_;
  }

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }

  const content::WebCursor& current_cursor() const { return current_cursor_; }

  void HandleTouchEvent(const ui::MotionEvent& event);
  void ResetGestureDetection();

  void Blur();

  // content::RenderWidgetHostView implementation
  content::RenderWidgetHost* GetRenderWidgetHost() const final;
  void SetSize(const gfx::Size& size) final;
  void SetBounds(const gfx::Rect& rect) final;
  void Focus() final;

 private:
  // content::RenderWidgetHostViewOxide implementation
  void OnTextInputStateChanged(ui::TextInputType type,
                               bool show_ime_if_needed) final;
  void OnSelectionBoundsChanged(const gfx::Rect& anchor_rect,
                                const gfx::Rect& focus_rect,
                                bool is_anchor_first) final;

  // content::RenderWidgetHostViewBase implementation
  void SelectionChanged(const base::string16& text,
                        size_t offset,
                        const gfx::Range& range) final;
  gfx::Size GetPhysicalBackingSize() const final;
  bool DoTopControlsShrinkBlinkSize() const final;
  float GetTopControlsHeight() const final;
  void FocusedNodeChanged(bool is_editable_node) final;
  void OnSwapCompositorFrame(uint32 output_surface_id,
                             scoped_ptr<cc::CompositorFrame> frame) final;
  void ClearCompositorFrame() final;
  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) final;
  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) final;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) final;
  void MovePluginWindows(
      const std::vector<content::WebPluginGeometry>& moves) final;
  void UpdateCursor(const content::WebCursor& cursor) final;
  void SetIsLoading(bool is_loading) final;
  void ImeCancelComposition() final;
  void RenderProcessGone(base::TerminationStatus status, int error_code) final;
  void Destroy() final;
  void SetTooltipText(const base::string16& tooltip_text) final;
  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      const content::ReadbackRequestCallback& callback,
      const SkColorType color_type) final;
  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(bool)>& callback) final;
  bool CanCopyToVideoFrame() const final;
  bool HasAcceleratedSurface(const gfx::Size& desired_size) final;
  void GetScreenInfo(blink::WebScreenInfo* results) final;
  bool GetScreenColorProfile(std::vector<char>* color_profile) final;
  gfx::Rect GetBoundsInRootWindow() final;
  void ShowDisambiguationPopup(const gfx::Rect& rect_pixels,
                               const SkBitmap& zoomed_bitmap) final;
  void LockCompositingSurface() final;
  void UnlockCompositingSurface() final;
  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) final;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) final;
  gfx::Vector2dF GetLastScrollOffset() const final;
  gfx::NativeView GetNativeView() const final;
  gfx::NativeViewId GetNativeViewId() const final;
  gfx::NativeViewAccessible GetNativeViewAccessible() final;
  bool HasFocus() const final;
  bool IsSurfaceAvailableForCopy() const final;
  void Show() final;
  void Hide() final;
  bool IsShowing() final;
  gfx::Rect GetViewBounds() const final;
  bool LockMouse() final;
  void UnlockMouse() final;

  // CompositorObserver implementation
  void CompositorDidCommit() final;

  // GestureProviderClient implementation
  void OnGestureEvent(const blink::WebGestureEvent& event) final;

  // RendererFrameEvictorClient implemenetation
  void EvictCurrentFrame() final;

  // cc::DelegatedFrameResourceCollectionClient implementation
  void UnusedResourcesAreAvailable() final;

  // ===================

  void UpdateCurrentCursor();

  void DestroyDelegatedContent();
  void SendDelegatedFrameAck(uint32 surface_id);
  void SendReturnedDelegatedResources();
  void RunAckCallbacks();
  void AttachLayer();
  void DetachLayer();

  content::RenderWidgetHostImpl* host_;

  RenderWidgetHostViewContainer* container_;

  gfx::GLSurfaceHandle shared_surface_handle_;

  scoped_refptr<cc::DelegatedFrameResourceCollection> resource_collection_;
  scoped_refptr<cc::DelegatedFrameProvider> frame_provider_;
  scoped_refptr<cc::DelegatedRendererLayer> layer_;

  // The output surface ID for the last frame from the renderer
  uint32 last_output_surface_id_;

  std::queue<base::Closure> ack_callbacks_;

  gfx::Size last_frame_size_dip_;

  bool frame_is_evicted_;

  ImeBridgeImpl ime_bridge_;

  bool is_loading_;
  content::WebCursor web_cursor_;
  content::WebCursor current_cursor_;

  bool is_showing_;
  gfx::Size last_size_;

  cc::CompositorFrameMetadata compositor_frame_metadata_;

  bool top_controls_shrink_blink_size_;

  scoped_ptr<GestureProvider> gesture_provider_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
