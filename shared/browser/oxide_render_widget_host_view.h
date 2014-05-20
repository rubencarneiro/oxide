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
#include "content/common/cursors/webcursor.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/gestures/gesture_recognizer.h"
#include "ui/events/gestures/gesture_types.h"
#include "ui/gfx/rect.h"

#include "shared/browser/oxide_gpu_utils.h"

typedef unsigned int GLuint;

namespace cc {
class SharedBitmap;
}

namespace content {
class RenderWidgetHostImpl;
}

namespace ui {
class GestureRecognizer;
class TouchEvent;
}

namespace oxide {

class AcceleratedFrameHandle;
class RenderWidgetHostView;
class WebView;

class SoftwareFrameHandle FINAL {
 public:
  SoftwareFrameHandle(RenderWidgetHostView* rwhv,
                      unsigned frame_id,
                      uint32 surface_id,
                      scoped_ptr<cc::SharedBitmap> bitmap,
                      const gfx::Size& size,
                      float scale);
  ~SoftwareFrameHandle();

  void* GetPixels();
  unsigned frame_id() const { return frame_id_; }
  gfx::Size size_in_pixels() const { return size_in_pixels_; }
  float device_scale_factor() const { return device_scale_factor_; }

  void WasFreed();

 private:
  RenderWidgetHostView* rwhv_;
  unsigned frame_id_;
  uint32 surface_id_;
  scoped_ptr<cc::SharedBitmap> bitmap_;
  gfx::Size size_in_pixels_;
  float device_scale_factor_;
};

class RenderWidgetHostView : public content::RenderWidgetHostViewBase,
                             public ui::GestureEventHelper,
                             public ui::GestureConsumer,
                             public AcceleratedFrameHandle::Client,
                             public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  virtual ~RenderWidgetHostView();

  virtual void Init(WebView* view) = 0;

  content::RenderWidgetHostImpl* host() const { return host_; }

  SoftwareFrameHandle* GetCurrentSoftwareFrameHandle();
  AcceleratedFrameHandle* GetCurrentAcceleratedFrameHandle();

  void DidCommitCompositorFrame();

  // content::RenderWidgetHostView
  content::RenderWidgetHost* GetRenderWidgetHost() const FINAL;

  void SetBounds(const gfx::Rect& rect) FINAL;

 protected:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);

  // content::RenderWidgetHostViewBase
  void WasShown() FINAL;
  void WasHidden() FINAL;

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
  typedef base::Callback<void(void)> SendSwapCompositorFrameAckCallback;

  // content::RenderWidgetHostViewBase
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

  // content::RenderWidgetHostView
  void InitAsChild(gfx::NativeView parent_view) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeViewId GetNativeViewId() const FINAL;
  gfx::NativeViewAccessible GetNativeViewAccessible() FINAL;

  virtual void Focus() OVERRIDE;

  bool IsSurfaceAvailableForCopy() const FINAL;

  bool LockMouse() FINAL;
  void UnlockMouse() FINAL;

  // ui::GestureEventHelper
  bool CanDispatchToConsumer(ui::GestureConsumer* consumer) FINAL;
  void DispatchGestureEvent(ui::GestureEvent* event) FINAL;
  void DispatchCancelTouchEvent(ui::TouchEvent* event) FINAL;

  // AcceleratedFrameHandle::Client
  void OnTextureResourcesAvailable(AcceleratedFrameHandle* handle) FINAL;

  // ===================

  bool ShouldCompositeNewFrame();

  void SendSwapCompositorFrameAck(uint32 surface_id);
  static void SendSwapCompositorFrameAckOnMainThread(
      SendSwapCompositorFrameAckCallback ack);

  void ProcessGestures(ui::GestureRecognizer::Gestures* gestures);
  void ForwardGestureEventToRenderer(ui::GestureEvent* event);

  virtual void SwapSoftwareFrame();
  virtual void SwapAcceleratedFrame();

  virtual void OnUpdateCursor(const content::WebCursor& cursor);

  bool is_hidden_;

  content::RenderWidgetHostImpl* host_;

  gfx::GLSurfaceHandle shared_surface_handle_;

  base::Lock compositor_frame_ack_callback_lock_;
  SendSwapCompositorFrameAckCallback compositor_frame_ack_callback_;

  scoped_refptr<AcceleratedFrameHandle> pending_accelerated_frame_;
  scoped_refptr<AcceleratedFrameHandle> current_accelerated_frame_;
  scoped_refptr<AcceleratedFrameHandle> previous_accelerated_frame_;
  scoped_ptr<SoftwareFrameHandle> current_software_frame_;
  scoped_ptr<SoftwareFrameHandle> previous_software_frame_;

  cc::CompositorFrameMetadata frame_metadata_;

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
