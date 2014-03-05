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

#include "oxide_render_widget_host_view.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/scoped_vector.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/view_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "ui/events/event.h"

#include "oxide_gpu_utils.h"

namespace oxide {

namespace {

void UpdateWebTouchEventAfterDispatch(blink::WebTouchEvent* event,
                                      blink::WebTouchPoint* point) {
  if (point->state != blink::WebTouchPoint::StateReleased &&
      point->state != blink::WebTouchPoint::StateCancelled) {
    return;
  }
  --event->touchesLength;
  for (unsigned i = point - event->touches;
       i < event->touchesLength;
       ++i) {
    event->touches[i] = event->touches[i + 1];
  }
}

bool ShouldSendPinchGesture() {
  static bool pinch_allowed =
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnableViewport) ||
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnablePinch);
  return pinch_allowed;
}

}

void RenderWidgetHostView::Paint(const gfx::Rect& dirty_rect) {}

void RenderWidgetHostView::BuffersSwapped() {
  AcknowledgeBuffersSwapped(true);
}

void RenderWidgetHostView::SendAcknowledgeBufferPresent(
    int32 route_id,
    int gpu_host_id,
    const gpu::Mailbox& mailbox,
    bool skipped) {
  {
    base::AutoLock lock(acknowledge_buffer_present_callback_lock_);
    acknowledge_buffer_present_callback_.Reset();
  }

  AcceleratedSurfaceMsg_BufferPresented_Params ack;
  ack.sync_point = 0;
  if (skipped) {
    ack.mailbox = mailbox;
  } else {
    std::swap(backbuffer_texture_handle_, frontbuffer_texture_handle_);
  }

  content::RenderWidgetHostImpl::AcknowledgeBufferPresent(
      route_id,
      gpu_host_id,
      ack);
}

// static
void RenderWidgetHostView::SendAcknowledgeBufferPresentOnMainThread(
    AcknowledgeBufferPresentCallback ack,
    bool skipped) {
  ack.Run(skipped);
}

bool RenderWidgetHostView::IsInBufferSwap() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return !acknowledge_buffer_present_callback_.is_null();
}

void RenderWidgetHostView::ProcessGestures(
    ui::GestureRecognizer::Gestures* gestures) {
  if (!gestures) {
    return;
  }

  for (ui::GestureRecognizer::Gestures::iterator it = gestures->begin();
       it != gestures->end(); ++it) {
    ForwardGestureEventToRenderer(*it);
  }
}

void RenderWidgetHostView::ForwardGestureEventToRenderer(
    ui::GestureEvent* event) {
  if ((event->type() == ui::ET_GESTURE_PINCH_BEGIN ||
       event->type() == ui::ET_GESTURE_PINCH_UPDATE ||
       event->type() == ui::ET_GESTURE_PINCH_END) &&
      !ShouldSendPinchGesture()) {
    return;
  }

  blink::WebGestureEvent gesture(
      content::MakeWebGestureEventFromUIEvent(*event));
  gesture.x = event->x();
  gesture.y = event->y();
  gesture.globalX = event->root_location().x();
  gesture.globalY = event->root_location().y();

  if (event->type() == ui::ET_GESTURE_TAP_DOWN) {
    // Webkit does not stop a fling-scroll on tap-down. So explicitly send an
    // event to stop any in-progress flings.
    blink::WebGestureEvent fling_cancel = gesture;
    fling_cancel.type = blink::WebInputEvent::GestureFlingCancel;
    fling_cancel.sourceDevice = blink::WebGestureEvent::Touchscreen;
    host_->ForwardGestureEvent(fling_cancel);
  }

  if (gesture.type == blink::WebInputEvent::Undefined) {
    return;
  }

  host_->ForwardGestureEventWithLatencyInfo(gesture, *event->latency());
}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    content::RenderWidgetHostViewBase(),
    is_hidden_(false),
    host_(content::RenderWidgetHostImpl::From(host)),
    frontbuffer_texture_handle_(NULL),
    backbuffer_texture_handle_(NULL),
    selection_cursor_position_(0),
    selection_anchor_position_(0),
    gesture_recognizer_(ui::GestureRecognizer::Create()) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  textures_[0] = GpuUtils::instance()->CreateTextureHandle();
  textures_[1] = GpuUtils::instance()->CreateTextureHandle();
  if (textures_[0]) {
    frontbuffer_texture_handle_ = textures_[0];
    backbuffer_texture_handle_ = textures_[1];
  }

  gesture_recognizer_->AddGestureEventHelper(this);

  host_->SetView(this);
}

void RenderWidgetHostView::AcknowledgeBuffersSwapped(bool skipped) {
  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    // Don't need lock on UI thread, as this is in the only thread that
    // modifies it
    if (!acknowledge_buffer_present_callback_.is_null()) {
      SendAcknowledgeBufferPresentOnMainThread(
          acknowledge_buffer_present_callback_, skipped);
    }
    return;
  }

  base::AutoLock lock(acknowledge_buffer_present_callback_lock_);

  if (acknowledge_buffer_present_callback_.is_null()) {
    return;
  }

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresentOnMainThread,
                 acknowledge_buffer_present_callback_, skipped));  
}

void RenderWidgetHostView::HandleTouchEvent(const ui::TouchEvent& event) {
  if (host_->ShouldForwardTouchEvent()) {
    blink::WebTouchPoint* point =
        content::UpdateWebTouchEventFromUIEvent(event, &touch_event_);
    if (point) {
      host_->ForwardTouchEventWithLatencyInfo(touch_event_, ui::LatencyInfo());
      UpdateWebTouchEventAfterDispatch(&touch_event_, point);
    }
  } else {
    scoped_ptr<ui::GestureRecognizer::Gestures> gestures(
        gesture_recognizer_->ProcessTouchEventForGesture(event, ui::ER_UNHANDLED, this));
    ProcessGestures(gestures.get());
  }
}

RenderWidgetHostView::~RenderWidgetHostView() {}

void RenderWidgetHostView::InitAsPopup(
    content::RenderWidgetHostView* parent_host_view,
    const gfx::Rect& pos) {
  NOTIMPLEMENTED() <<
      "InitAsPopup() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::InitAsFullscreen(
    content::RenderWidgetHostView* reference_host_view) {
  NOTIMPLEMENTED() <<
      "InitAsFullScreen() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::WasShown() {
  if (!is_hidden_) {
    return;
  }

  is_hidden_ = false;

  host()->WasShown();
}

void RenderWidgetHostView::WasHidden() {
  if (is_hidden_) {
    return;
  }

  is_hidden_ = true;

  host()->WasHidden();
}

void RenderWidgetHostView::MovePluginWindows(
    const gfx::Vector2d& scroll_offset,
    const std::vector<content::WebPluginGeometry>& moves) {}

void RenderWidgetHostView::Blur() {}

void RenderWidgetHostView::UpdateCursor(const WebCursor& cursor) {}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {}

void RenderWidgetHostView::TextInputTypeChanged(ui::TextInputType type,
                                                ui::TextInputMode mode,
                                                bool can_compose_inline) {}

void RenderWidgetHostView::ImeCancelComposition() {}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::DidUpdateBackingStore(
    const gfx::Rect& scroll_rect,
    const gfx::Vector2d& scroll_delta,
    const std::vector<gfx::Rect>& copy_rects,
    const std::vector<ui::LatencyInfo>& latency_info) {
  if (is_hidden_) {
    return;
  }

  Paint(scroll_rect);

  for (size_t i = 0; i < copy_rects.size(); ++i) {
    gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
    if (rect.IsEmpty()) {
      continue;
    }

    Paint(rect);
  }
}

void RenderWidgetHostView::RenderProcessGone(base::TerminationStatus status,
                                             int error_code) {
  Destroy();
}

void RenderWidgetHostView::Destroy() {
  host_ = NULL;
  delete this;
}

void RenderWidgetHostView::SetTooltipText(const base::string16& tooltip_text) {}

void RenderWidgetHostView::SelectionBoundsChanged(
    const ViewHostMsg_SelectionBounds_Params& params) {
  caret_rect_ = gfx::UnionRects(params.anchor_rect, params.focus_rect);

  if (params.is_anchor_first) {
    selection_cursor_position_ =
        selection_range_.GetMax() - selection_text_offset_;
    selection_anchor_position_ =
        selection_range_.GetMin() - selection_text_offset_;
  } else {
    selection_cursor_position_ =
        selection_range_.GetMin() - selection_text_offset_;
    selection_anchor_position_ =
        selection_range_.GetMax() - selection_text_offset_;
  }
}

void RenderWidgetHostView::ScrollOffsetChanged() {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback,
    const SkBitmap::Config config) {
  GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
}

void RenderWidgetHostView::CopyFromCompositingSurfaceToVideoFrame(
    const gfx::Rect& src_subrect,
    const scoped_refptr<media::VideoFrame>& target,
    const base::Callback<void(bool)>& callback) {
  NOTIMPLEMENTED();
  callback.Run(false);
}

bool RenderWidgetHostView::CanCopyToVideoFrame() const {
  return false;
}

void RenderWidgetHostView::OnAcceleratedCompositingStateChange() {}

void RenderWidgetHostView::AcceleratedSurfaceInitialized(
    int host_id, int route_id) {}

void RenderWidgetHostView::AcceleratedSurfaceBuffersSwapped(
    const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
    int gpu_host_id) {
  DCHECK(!IsInBufferSwap());

  backbuffer_texture_handle_->Consume(params_in_pixel.mailbox,
                                      params_in_pixel.size);

  {
    base::AutoLock lock(acknowledge_buffer_present_callback_lock_);
    acknowledge_buffer_present_callback_ =
        base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresent,
                   AsWeakPtr(),
                   params_in_pixel.route_id,
                   gpu_host_id,
                   params_in_pixel.mailbox);
  }

  if (params_in_pixel.size != GetPhysicalBackingSize()) {
    AcknowledgeBuffersSwapped(true);
    return;
  }

  BuffersSwapped();
}

void RenderWidgetHostView::AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) {
  DCHECK(!IsInBufferSwap());
  NOTIMPLEMENTED() << "PostSubBuffer is not implemented";

  {
    base::AutoLock lock(acknowledge_buffer_present_callback_lock_);
    acknowledge_buffer_present_callback_ =
        base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresent,
                   AsWeakPtr(),
                   params_in_pixel.route_id,
                   gpu_host_id,
                   params_in_pixel.mailbox);
  }
  AcknowledgeBuffersSwapped(true);
}

void RenderWidgetHostView::AcceleratedSurfaceSuspend() {}

void RenderWidgetHostView::AcceleratedSurfaceRelease() {}

bool RenderWidgetHostView::HasAcceleratedSurface(
    const gfx::Size& desired_size) {
  return false;
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface() {
  if (shared_surface_handle_.is_null()) {
    shared_surface_handle_ = GpuUtils::instance()->GetSharedSurfaceHandle();
  }

  return shared_surface_handle_;
}

void RenderWidgetHostView::ProcessAckedTouchEvent(
    const content::TouchEventWithLatencyInfo& touch,
    content::InputEventAckState ack_result) {
  ScopedVector<ui::TouchEvent> events;
  if (!content::MakeUITouchEventsFromWebTouchEvents(
      touch, &events, content::LOCAL_COORDINATES)) {
    return;
  }

  ui::EventResult result =
      (ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED) ?
        ui::ER_HANDLED : ui::ER_UNHANDLED;
  for (ScopedVector<ui::TouchEvent>::iterator it = events.begin(),
       end = events.end(); it != end; ++it)  {
    ui::TouchEvent* event = *it;
    scoped_ptr<ui::GestureRecognizer::Gestures> gestures(
        gesture_recognizer_->ProcessTouchEventForGesture(*event, result, this));
    ProcessGestures(gestures.get());
  }
}

void RenderWidgetHostView::SetHasHorizontalScrollbar(
    bool has_horizontal_scrollbar) {}

void RenderWidgetHostView::SetScrollOffsetPinning(bool is_pinned_to_left,
                                                  bool is_pinned_to_right) {}

void RenderWidgetHostView::OnAccessibilityEvents(
    const std::vector<AccessibilityHostMsg_EventParams>& params) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  host()->SendScreenRects();
  GetRenderWidgetHost()->WasResized();
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
}

gfx::NativeView RenderWidgetHostView::GetNativeView() const {
  return NULL;
}

gfx::NativeViewId RenderWidgetHostView::GetNativeViewId() const {
  return 0;
}

gfx::NativeViewAccessible RenderWidgetHostView::GetNativeViewAccessible() {
  return NULL;
}

void RenderWidgetHostView::Focus() {}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::OnFocus() {
  host()->GotFocus();
  GetRenderWidgetHost()->SetActive(true);

  // XXX: Should we have a run-time check to see if this is required?
  host()->SetInputMethodActive(true);
}

void RenderWidgetHostView::OnBlur() {
  host()->SetInputMethodActive(false);

  GetRenderWidgetHost()->SetActive(false);
  GetRenderWidgetHost()->Blur();
}

TextureHandle* RenderWidgetHostView::GetCurrentTextureHandle() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if (IsInBufferSwap()) {
    return backbuffer_texture_handle_;
  }

  return frontbuffer_texture_handle_;
}

bool RenderWidgetHostView::CanDispatchToConsumer(
    ui::GestureConsumer* consumer) {
  DCHECK_EQ(this, static_cast<RenderWidgetHostView *>(consumer));
  return true;
}

void RenderWidgetHostView::DispatchPostponedGestureEvent(
    ui::GestureEvent* event) {
  ForwardGestureEventToRenderer(event);
}

void RenderWidgetHostView::DispatchCancelTouchEvent(ui::TouchEvent* event) {
  if (!host_->ShouldForwardTouchEvent()) {
    return;
  }

  DCHECK_EQ(event->type(), ui::ET_TOUCH_CANCELLED);
  blink::WebTouchEvent cancel_event;
  cancel_event.type = blink::WebInputEvent::TouchCancel;
  cancel_event.timeStampSeconds = event->time_stamp().InSecondsF();
  host_->ForwardTouchEventWithLatencyInfo(
      cancel_event, *event->latency());
}

} // namespace oxide
