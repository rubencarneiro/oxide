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
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/gl_frame_data.h"
#include "cc/output/software_frame_data.h"
#include "cc/resources/shared_bitmap.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "content/common/view_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebGestureDevice.h"
#include "ui/events/event.h"

#include "oxide_default_screen_info.h"
#include "oxide_gpu_utils.h"

namespace content {
void RenderWidgetHostViewBase::GetDefaultScreenInfo(
    blink::WebScreenInfo* results) {
  *results = oxide::GetDefaultWebScreenInfo();
}
}

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

bool IsUsingSoftwareCompositing() {
  static bool sw_compositing =
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kDisableGpuCompositing);
  return sw_compositing;
}

}

SoftwareFrameHandle::SoftwareFrameHandle(RenderWidgetHostView* rwhv,
                                         unsigned frame_id,
                                         uint32 surface_id,
                                         scoped_ptr<cc::SharedBitmap> bitmap,
                                         const gfx::Size& size,
                                         float scale)
    : rwhv_(rwhv),
      frame_id_(frame_id),
      surface_id_(surface_id),
      bitmap_(bitmap.Pass()),
      size_in_pixels_(size),
      device_scale_factor_(scale) {}

SoftwareFrameHandle::~SoftwareFrameHandle() {
  if (bitmap_ && rwhv_->host()) {
    cc::CompositorFrameAck ack;
    ack.last_software_frame_id = frame_id_;
    content::RenderWidgetHostImpl::SendReclaimCompositorResources(
        rwhv_->host()->GetRoutingID(),
        surface_id_,
        rwhv_->host()->GetProcess()->GetID(),
        ack);
  }
}

void* SoftwareFrameHandle::GetPixels() {
  return bitmap_->pixels();
}

void SoftwareFrameHandle::WasFreed() {
  bitmap_.reset();
}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {}

void RenderWidgetHostView::OnSwapCompositorFrame(
    uint32 output_surface_id,
    scoped_ptr<cc::CompositorFrame> frame) {
  {
    base::AutoLock lock(compositor_frame_ack_callback_lock_);
    if (!compositor_frame_ack_callback_.is_null()) {
      DLOG(ERROR) << "Received new compositor frame with pending ack";
      host_->GetProcess()->ReceivedBadMessage();
      return;
    }

    compositor_frame_ack_callback_ =
        base::Bind(&RenderWidgetHostView::SendSwapCompositorFrameAck,
                   AsWeakPtr(), output_surface_id);
  }

  if (IsUsingSoftwareCompositing()) {
    DCHECK(!pending_accelerated_frame_ &&
           !current_accelerated_frame_ &&
           !previous_accelerated_frame_);
    DCHECK(!previous_software_frame_);

    if (!frame->software_frame_data) {
      DLOG(ERROR) << "Invalid swap software compositor frame message received";
      host_->GetProcess()->ReceivedBadMessage();
      return;
    }

    previous_software_frame_.swap(current_software_frame_);

    scoped_ptr<cc::SharedBitmap> shared_bitmap =
        content::HostSharedBitmapManager::current()->GetSharedBitmapFromId(
          frame->software_frame_data->size,
          frame->software_frame_data->bitmap_id);
    if (!shared_bitmap) {
      DLOG(ERROR) << "Failed to create shared bitmap for software frame";
      host_->GetProcess()->ReceivedBadMessage();
      return;
    }

    current_software_frame_.reset(
        new SoftwareFrameHandle(this,
                                frame->software_frame_data->id,
                                output_surface_id,
                                shared_bitmap.Pass(),
                                frame->software_frame_data->size,
                                frame->metadata.device_scale_factor));

    if (!ShouldCompositeNewFrame()) {
      DidCommitCompositorFrame();
    } else {
      SwapSoftwareFrame();
    }
    return;
  }

  DCHECK(!current_software_frame_ && !previous_software_frame_);
  DCHECK(!pending_accelerated_frame_ && !previous_accelerated_frame_);

  if (!frame->gl_frame_data || frame->gl_frame_data->mailbox.IsZero()) {
    DLOG(ERROR) << "Invalid swap accelerated compositor frame message received";
    host_->GetProcess()->ReceivedBadMessage();
    return;
  }

  pending_accelerated_frame_ =
      GpuUtils::instance()->GetAcceleratedFrameHandle(
        this, this,
        output_surface_id,
        frame->gl_frame_data->mailbox,
        frame->gl_frame_data->sync_point,
        frame->gl_frame_data->size,
        frame->metadata.device_scale_factor);
}

void RenderWidgetHostView::InitAsPopup(
    content::RenderWidgetHostView* parent_host_view,
    const gfx::Rect& pos) {
  NOTREACHED() << "Popup RenderWidgetHostView's are not supported";
}

void RenderWidgetHostView::InitAsFullscreen(
    content::RenderWidgetHostView* reference_host_view) {
  NOTREACHED() << "Fullscreen RenderWidgetHostView's are not supported";
}

void RenderWidgetHostView::MovePluginWindows(
    const std::vector<content::WebPluginGeometry>& moves) {}

void RenderWidgetHostView::Blur() {}

void RenderWidgetHostView::UpdateCursor(const content::WebCursor& cursor) {
  last_cursor_ = cursor;
  if (!is_loading_) {
    OnUpdateCursor(cursor);
  }
}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {
  if (is_loading != is_loading_) {
    is_loading_ = is_loading;
    if (is_loading) {
      content::WebCursor::CursorInfo busy_cursor_info(blink::WebCursorInfo::TypeWait);
      content::WebCursor busy_cursor(busy_cursor_info);
      OnUpdateCursor(busy_cursor);
    } else {
      OnUpdateCursor(last_cursor_);
    }
  }
}

void RenderWidgetHostView::TextInputStateChanged(
    const ViewHostMsg_TextInputState_Params& params) {}

void RenderWidgetHostView::ImeCancelComposition() {}

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

  if (selection_range_.IsValid()) {
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
}

void RenderWidgetHostView::ScrollOffsetChanged() {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback,
    const SkBitmap::Config config) {
  callback.Run(false, SkBitmap());
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
  DLOG(ERROR) << "Old compositor mode is not supported";
  host_->GetProcess()->ReceivedBadMessage();
}

void RenderWidgetHostView::AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) {
  DLOG(ERROR) << "Old compositor mode is not supported";
  host_->GetProcess()->ReceivedBadMessage();
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

void RenderWidgetHostView::SetScrollOffsetPinning(bool is_pinned_to_left,
                                                  bool is_pinned_to_right) {}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {
  NOTREACHED() << "InitAsChild() isn't used. Please use Init() instead";
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

bool RenderWidgetHostView::CanDispatchToConsumer(
    ui::GestureConsumer* consumer) {
  DCHECK_EQ(this, static_cast<RenderWidgetHostView *>(consumer));
  return true;
}

void RenderWidgetHostView::DispatchGestureEvent(ui::GestureEvent* event) {
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

void RenderWidgetHostView::OnTextureResourcesAvailable(
    AcceleratedFrameHandle* handle) {
  DCHECK_EQ(handle, pending_accelerated_frame_.get());
  DCHECK(!previous_accelerated_frame_);

  previous_accelerated_frame_.swap(current_accelerated_frame_);
  current_accelerated_frame_.swap(pending_accelerated_frame_);

  if (!ShouldCompositeNewFrame()) {
    DidCommitCompositorFrame();
  } else {
    SwapAcceleratedFrame();
  }
}

bool RenderWidgetHostView::ShouldCompositeNewFrame() {
  if (host()->is_hidden()) {
    return false;
  }

  gfx::Rect bounds(GetViewBounds());
  if (bounds.width() <= 0 || bounds.height() <= 0) {
    return false;
  }

  return true;
}

void RenderWidgetHostView::SendSwapCompositorFrameAck(uint32 surface_id) {
  cc::CompositorFrameAck ack;
  if (IsUsingSoftwareCompositing()) {
    if (previous_software_frame_) {
      ack.last_software_frame_id = previous_software_frame_->frame_id();

      previous_software_frame_->WasFreed();
      previous_software_frame_.reset();
    }

  } else {
    ack.gl_frame_data.reset(new cc::GLFrameData());
    if (previous_accelerated_frame_) {
      ack.gl_frame_data->mailbox = previous_accelerated_frame_->mailbox();
      ack.gl_frame_data->sync_point = 0;
      ack.gl_frame_data->size = previous_accelerated_frame_->size_in_pixels();

      previous_accelerated_frame_->WasFreed();
      previous_accelerated_frame_ = NULL;
    }
  }

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      host_->GetRoutingID(),
      surface_id,
      host_->GetProcess()->GetID(),
      ack);
}

// static
void RenderWidgetHostView::SendSwapCompositorFrameAckOnMainThread(
    SendSwapCompositorFrameAckCallback ack) {
  ack.Run();
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
    fling_cancel.sourceDevice = blink::WebGestureDeviceTouchpad;
    host_->ForwardGestureEvent(fling_cancel);
  }

  if (gesture.type == blink::WebInputEvent::Undefined) {
    return;
  }

  host_->ForwardGestureEventWithLatencyInfo(gesture, *event->latency());
}

void RenderWidgetHostView::SwapSoftwareFrame() {
  NOTIMPLEMENTED();
  DidCommitCompositorFrame();
}

void RenderWidgetHostView::SwapAcceleratedFrame() {
  NOTIMPLEMENTED();
  DidCommitCompositorFrame();
}

void RenderWidgetHostView::OnUpdateCursor(const content::WebCursor& cursor) {}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    content::RenderWidgetHostViewBase(),
    host_(content::RenderWidgetHostImpl::From(host)),
    selection_cursor_position_(0),
    selection_anchor_position_(0),
    is_loading_(false),
    gesture_recognizer_(ui::GestureRecognizer::Create()) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  gesture_recognizer_->AddGestureEventHelper(this);

  host_->SetView(this);
}

void RenderWidgetHostView::WasShown() {
  host()->WasShown();
}

void RenderWidgetHostView::WasHidden() {
  host()->WasHidden();
}

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

void RenderWidgetHostView::OnResize() {
  host()->SendScreenRects();
  GetRenderWidgetHost()->WasResized();
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

SoftwareFrameHandle* RenderWidgetHostView::GetCurrentSoftwareFrameHandle() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return current_software_frame_.get();
}

AcceleratedFrameHandle* RenderWidgetHostView::GetCurrentAcceleratedFrameHandle() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return current_accelerated_frame_.get();
}

void RenderWidgetHostView::DidCommitCompositorFrame() {
  base::AutoLock lock(compositor_frame_ack_callback_lock_);

  DCHECK(!compositor_frame_ack_callback_.is_null());

  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    SendSwapCompositorFrameAckOnMainThread(compositor_frame_ack_callback_);
  } else {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&RenderWidgetHostView::SendSwapCompositorFrameAckOnMainThread,
                   compositor_frame_ack_callback_));
  }

  compositor_frame_ack_callback_.Reset();
}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
}

} // namespace oxide
