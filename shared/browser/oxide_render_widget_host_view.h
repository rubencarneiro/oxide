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

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/gestures/gesture_recognizer.h"
#include "ui/events/gestures/gesture_types.h"
#include "ui/gfx/rect.h"

typedef unsigned int GLuint;

namespace content {
class RenderWidgetHostImpl;
}

namespace ui {
class GestureRecognizer;
class TouchEvent;
}

namespace oxide {

class TextureHandle;

class RenderWidgetHostView : public content::RenderWidgetHostViewBase,
                             public ui::GestureEventHelper,
                             public ui::GestureConsumer,
                             public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  virtual ~RenderWidgetHostView();

  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) FINAL;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) FINAL;

  void WasShown() FINAL;
  void WasHidden() FINAL;

  void MovePluginWindows(
      const gfx::Vector2d& scroll_offset,
      const std::vector<content::WebPluginGeometry>& moves) FINAL;

  virtual void Blur() OVERRIDE;

  void UpdateCursor(const WebCursor& cursor) FINAL;

  void SetIsLoading(bool is_loading) FINAL;

  virtual void TextInputTypeChanged(ui::TextInputType type,
                                    ui::TextInputMode mode,
                                    bool can_compose_inline) OVERRIDE;
  virtual void ImeCancelComposition() OVERRIDE;
  virtual void FocusedNodeChanged(bool is_editable_node) OVERRIDE;
  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) FINAL;

  void DidUpdateBackingStore(
      const gfx::Rect& scroll_rect,
      const gfx::Vector2d& scroll_delta,
      const std::vector<gfx::Rect>& copy_rects,
      const std::vector<ui::LatencyInfo>& latency_info) FINAL;

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

  void SetHasHorizontalScrollbar(bool has_horizontal_scrollbar) FINAL;
  void SetScrollOffsetPinning(bool is_pinned_to_left,
                              bool is_pinned_to_right) FINAL;

  void OnAccessibilityEvents(
      const std::vector<AccessibilityHostMsg_EventParams>& params) FINAL;

  void InitAsChild(gfx::NativeView parent_view) FINAL;

  content::RenderWidgetHost* GetRenderWidgetHost() const FINAL;
  content::RenderWidgetHostImpl* host() const { return host_; }

  void SetSize(const gfx::Size& size) OVERRIDE;
  void SetBounds(const gfx::Rect& rect) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeViewId GetNativeViewId() const FINAL;
  gfx::NativeViewAccessible GetNativeViewAccessible() FINAL;

  void Focus() OVERRIDE;

  bool IsSurfaceAvailableForCopy() const FINAL;

  bool LockMouse() FINAL;
  void UnlockMouse() FINAL;

  void OnFocus();
  void OnBlur();

  TextureHandle* GetCurrentTextureHandle();

  bool CanDispatchToConsumer(ui::GestureConsumer* consumer) FINAL;
  void DispatchPostponedGestureEvent(ui::GestureEvent* event) FINAL;
  void DispatchCancelTouchEvent(ui::TouchEvent* event) FINAL;

 protected:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);

  void AcknowledgeBuffersSwapped(bool skipped);

  gfx::Rect caret_rect() const { return caret_rect_; }
  size_t selection_cursor_position() const {
    return selection_cursor_position_;
  }
  size_t selection_anchor_position() const {
    return selection_anchor_position_;
  }

  void HandleTouchEvent(const ui::TouchEvent& event);

 private:
  typedef base::Callback<void(bool)> AcknowledgeBufferPresentCallback;

  virtual void Paint(const gfx::Rect& dirty_rect);
  virtual void BuffersSwapped();
  void SendAcknowledgeBufferPresent(int32 route_id,
                                    int gpu_host_id,
                                    const gpu::Mailbox& mailbox,
                                    bool skipped);
  static void SendAcknowledgeBufferPresentOnMainThread(
      AcknowledgeBufferPresentCallback ack,
      bool skipped);
  bool IsInBufferSwap();

  void ProcessGestures(ui::GestureRecognizer::Gestures* gestures);
  void ForwardGestureEventToRenderer(ui::GestureEvent* event);

  bool is_hidden_;

  content::RenderWidgetHostImpl* host_;

  gfx::GLSurfaceHandle shared_surface_handle_;

  base::Lock acknowledge_buffer_present_callback_lock_;
  AcknowledgeBufferPresentCallback acknowledge_buffer_present_callback_;

  scoped_refptr<TextureHandle> textures_[2];
  TextureHandle* frontbuffer_texture_handle_;
  TextureHandle* backbuffer_texture_handle_;

  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;

  scoped_ptr<ui::GestureRecognizer> gesture_recognizer_;
  blink::WebTouchEvent touch_event_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
