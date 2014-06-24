// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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
#include "base/memory/scoped_vector.h"
#include "base/memory/weak_ptr.h"
#include "cc/layers/delegated_frame_resource_collection.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/common/cursors/webcursor.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/gestures/gesture_recognizer.h"
#include "ui/events/gestures/gesture_types.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/oxide_renderer_frame_evictor_client.h"

namespace cc {
class DelegatedFrameProvider;
class DelegatedRendererLayer;
}

namespace content {
class RenderWidgetHostImpl;
}

namespace ui {
class GestureRecognizer;
class TouchEvent;
}

namespace oxide {

class Compositor;
class CompositorFrameHandle;
class WebView;

class RenderWidgetHostView : public content::RenderWidgetHostViewBase,
                             public ui::GestureEventHelper,
                             public ui::GestureConsumer,
                             public CompositorClient,
                             public RendererFrameEvictorClient,
                             public cc::DelegatedFrameResourceCollectionClient,
                             public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  virtual ~RenderWidgetHostView();

  virtual void Init(WebView* view) = 0;

  content::RenderWidgetHostImpl* host() const { return host_; }

  CompositorFrameHandle* GetCompositorFrameHandle();
  void DidCommitCompositorFrame();

  // content::RenderWidgetHostView implementation
  content::RenderWidgetHost* GetRenderWidgetHost() const FINAL;

  void SetBounds(const gfx::Rect& rect) FINAL;

 protected:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);

  // content::RenderWidgetHostViewBase implementation
  void WasShown() FINAL;
  void WasHidden() FINAL;

  // =================
  void OnFocus();
  void OnBlur();
  void OnResize();

  gfx::Rect caret_rect() const { return caret_rect_; }
  size_t selection_cursor_position() const {
    return selection_cursor_position_;
  }
  size_t selection_anchor_position() const {
    return selection_anchor_position_;
  }

  void HandleTouchEvent(const ui::TouchEvent& event);

 private:
  // content::RenderWidgetHostViewBase implementation
  virtual void FocusedNodeChanged(bool is_editable_node) OVERRIDE;

  void OnSwapCompositorFrame(uint32 output_surface_id,
                             scoped_ptr<cc::CompositorFrame> frame) FINAL;

  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) FINAL;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) FINAL;

  void MovePluginWindows(
      const std::vector<content::WebPluginGeometry>& moves) FINAL;

  virtual void Blur() OVERRIDE;

  void UpdateCursor(const content::WebCursor& cursor) FINAL;
  void SetIsLoading(bool is_loading) FINAL;

  virtual void TextInputTypeChanged(ui::TextInputType type,
                                    ui::TextInputMode mode,
                                    bool can_compose_inline) OVERRIDE;
  virtual void ImeCancelComposition() OVERRIDE;

  void RenderProcessGone(base::TerminationStatus status, int error_code) FINAL;

  void Destroy() FINAL;

  void SetTooltipText(const base::string16& tooltip_text) FINAL;

  void SelectionBoundsChanged(
      const ViewHostMsg_SelectionBounds_Params& params) FINAL;

  void ScrollOffsetChanged() FINAL;

  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      const base::Callback<void(bool, const SkBitmap&)>& callback,
      const SkBitmap::Config config) FINAL;

  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(bool)>& callback) FINAL;
  bool CanCopyToVideoFrame() const FINAL;

  void OnAcceleratedCompositingStateChange() FINAL;
  void AcceleratedSurfaceInitialized(int host_id, int route_id) FINAL;
  void AcceleratedSurfaceBuffersSwapped(
      const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfaceSuspend() FINAL;
  void AcceleratedSurfaceRelease() FINAL;

  bool HasAcceleratedSurface(const gfx::Size& desired_size) FINAL;

  gfx::GLSurfaceHandle GetCompositingSurface() FINAL;

  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) FINAL;

  void SetScrollOffsetPinning(bool is_pinned_to_left,
                              bool is_pinned_to_right) FINAL;

  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) FINAL;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeViewId GetNativeViewId() const FINAL;
  gfx::NativeViewAccessible GetNativeViewAccessible() FINAL;

  virtual void Focus() OVERRIDE;

  bool IsSurfaceAvailableForCopy() const FINAL;

  bool LockMouse() FINAL;
  void UnlockMouse() FINAL;

  // ui::GestureEventHelper implementation
  bool CanDispatchToConsumer(ui::GestureConsumer* consumer) FINAL;
  void DispatchGestureEvent(ui::GestureEvent* event) FINAL;
  void DispatchCancelTouchEvent(ui::TouchEvent* event) FINAL;

  // cc::DelegatedFrameResourceCollectionClient implementation
  void UnusedResourcesAreAvailable() FINAL;

  // CompositorClient implementation
  void CompositorDidCommit() FINAL;
  void CompositorSwapFrame(uint32 surface_id,
                           scoped_ptr<CompositorFrameHandle> frame) FINAL;

  // RendererFrameEvictorClient implemenetation
  void EvictCurrentFrame() FINAL;

  // ===================

  void DestroyDelegatedContent();
  void SendDelegatedFrameAck(uint32 surface_id);
  void SendReturnedDelegatedResources();
  void RunAckCallbacks();

  void ProcessGestures(ui::GestureRecognizer::Gestures* gestures);
  void ForwardGestureEventToRenderer(ui::GestureEvent* event);

  virtual void OnCompositorSwapFrame() = 0;

  virtual void OnUpdateCursor(const content::WebCursor& cursor);

  content::RenderWidgetHostImpl* host_;

  gfx::GLSurfaceHandle shared_surface_handle_;

  // XXX: Move compositor_ to WebView (https://launchpad.net/bugs/1312081)
  scoped_ptr<Compositor> compositor_;

  scoped_refptr<cc::DelegatedFrameResourceCollection> resource_collection_;
  scoped_refptr<cc::DelegatedFrameProvider> frame_provider_;
  scoped_refptr<cc::DelegatedRendererLayer> layer_;

  // The output surface ID for the last frame from the renderer
  uint32 last_output_surface_id_;

  std::queue<base::Closure> ack_callbacks_;

  gfx::Size last_frame_size_dip_;

  scoped_ptr<CompositorFrameHandle> current_compositor_frame_;
  ScopedVector<CompositorFrameHandle> previous_compositor_frames_;
  std::queue<uint32> received_surface_ids_;

  bool frame_is_evicted_;

  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;

  bool is_loading_;
  content::WebCursor last_cursor_;

  scoped_ptr<ui::GestureRecognizer> gesture_recognizer_;
  blink::WebTouchEvent touch_event_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
