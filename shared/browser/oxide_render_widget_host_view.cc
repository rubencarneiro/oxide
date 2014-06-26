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
#include "cc/layers/delegated_frame_provider.h"
#include "cc/layers/delegated_renderer_layer.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/delegated_frame_data.h"
#include "cc/quads/render_pass.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/view_messages.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/content_switches.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebGestureDevice.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gl/gl_implementation.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_utils.h"

#include "oxide_browser_process_main.h"
#include "oxide_default_screen_info.h"
#include "oxide_renderer_frame_evictor.h"
#include "oxide_web_view.h"

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

}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  if (!web_view_) {
    return gfx::Size();
  }

  return web_view_->GetContainerSizePix();
}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {}

void RenderWidgetHostView::OnSwapCompositorFrame(
    uint32 output_surface_id,
    scoped_ptr<cc::CompositorFrame> frame) {
  if (!frame->delegated_frame_data) {
    DLOG(ERROR) << "Non delegated renderer path is not supported";
    host_->GetProcess()->ReceivedBadMessage();
    return;
  }

  scoped_ptr<cc::DelegatedFrameData> frame_data =
      frame->delegated_frame_data.Pass();

  if (frame_data->render_pass_list.empty()) {
    DLOG(ERROR) << "Invalid delegated frame";
    host_->GetProcess()->ReceivedBadMessage();
    return;
  }

  Compositor* compositor = web_view_ ? web_view_->compositor() : NULL;
  CompositorLock lock(compositor);

  if (output_surface_id != last_output_surface_id_) {
    resource_collection_->SetClient(NULL);
    if (resource_collection_->LoseAllResources()) {
      SendReturnedDelegatedResources();
    }
    resource_collection_ = new cc::DelegatedFrameResourceCollection();
    resource_collection_->SetClient(this);

    DestroyDelegatedContent();

    // XXX(chrisccoulson): Should we clear ack_callbacks_ here as well?

    last_output_surface_id_ = output_surface_id;
  }

  base::Closure ack_callback =
      base::Bind(&RenderWidgetHostView::SendDelegatedFrameAck,
                 AsWeakPtr(), output_surface_id);
  ack_callbacks_.push(ack_callback);

  float device_scale_factor = frame->metadata.device_scale_factor;
  cc::RenderPass* root_pass = frame_data->render_pass_list.back();

  gfx::Size frame_size = root_pass->output_rect.size();
  gfx::Size frame_size_dip = gfx::ToFlooredSize(
      gfx::ScaleSize(frame_size, 1.0f / device_scale_factor));

  gfx::Rect damage_rect_dip = gfx::ToEnclosingRect(
      gfx::ScaleRect(root_pass->damage_rect, 1.0f / device_scale_factor));

  if (frame_size.IsEmpty()) {
    DestroyDelegatedContent();
  } else {
    if (!frame_provider_ || frame_size != frame_provider_->frame_size() ||
        frame_size_dip != last_frame_size_dip_) {
      DetachLayer();
      frame_provider_ = new cc::DelegatedFrameProvider(resource_collection_,
                                                       frame_data.Pass());
      layer_ = cc::DelegatedRendererLayer::Create(frame_provider_);
      AttachLayer();
    } else {
      frame_provider_->SetFrameData(frame_data.Pass());
    }
  }

  last_frame_size_dip_ = frame_size_dip;

  if (layer_) {
    layer_->SetIsDrawable(true);
    layer_->SetContentsOpaque(true);
    layer_->SetBounds(frame_size_dip);
    layer_->SetNeedsDisplayRect(damage_rect_dip);
  }

  if (frame_is_evicted_) {
    frame_is_evicted_ = false;
    RendererFrameEvictor::GetInstance()->AddFrame(this, !host_->is_hidden());
  }

  if (web_view_) {
    web_view_->GotNewCompositorFrameMetadata(frame->metadata);
  }

  if (!compositor || !compositor->IsActive()) {
    RunAckCallbacks();
  }
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

void RenderWidgetHostView::GetScreenInfo(blink::WebScreenInfo* result) {
  if (!web_view_) {
    *result = GetDefaultWebScreenInfo();
    return;
  }

  *result = web_view_->GetScreenInfo();
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow() {
  return GetViewBounds();
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface() {
  if (shared_surface_handle_.is_null()) {
    shared_surface_handle_ =
        CompositorUtils::GetInstance()->GetSharedSurfaceHandle();
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

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

gfx::Rect RenderWidgetHostView::GetViewBounds() const {
  if (!web_view_) {
    return gfx::Rect(last_size_);
  }

  return web_view_->GetContainerBoundsDip();
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

void RenderWidgetHostView::UnusedResourcesAreAvailable() {
  if (ack_callbacks_.empty()) {
    SendReturnedDelegatedResources();
  }
}

void RenderWidgetHostView::EvictCurrentFrame() {
  frame_is_evicted_ = true;
  DestroyDelegatedContent();

  if (web_view_) {
    web_view_->EvictCurrentFrame();
  }
}

void RenderWidgetHostView::DestroyDelegatedContent() {
  DetachLayer();
  frame_provider_ = NULL;
  layer_ = NULL;
}

void RenderWidgetHostView::SendDelegatedFrameAck(uint32 surface_id) {
  cc::CompositorFrameAck ack;
  resource_collection_->TakeUnusedResourcesForChildCompositor(&ack.resources);

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      host_->GetRoutingID(),
      surface_id,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::SendReturnedDelegatedResources() {
  cc::CompositorFrameAck ack;
  resource_collection_->TakeUnusedResourcesForChildCompositor(&ack.resources);

  DCHECK(!ack.resources.empty());

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      host_->GetRoutingID(),
      last_output_surface_id_,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::RunAckCallbacks() {
  while (!ack_callbacks_.empty()) {
    ack_callbacks_.front().Run();
    ack_callbacks_.pop();
  }
}

void RenderWidgetHostView::AttachLayer() {
  if (!web_view_) {
    return;
  }
  if (!layer_) {
    return;
  }

  web_view_->compositor()->SetRootLayer(layer_);
}

void RenderWidgetHostView::DetachLayer() {
  if (!web_view_) {
    return;
  }
  if (!layer_) {
    return;
  }

  web_view_->compositor()->SetRootLayer(scoped_refptr<cc::Layer>());
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

void RenderWidgetHostView::OnUpdateCursor(const content::WebCursor& cursor) {}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    content::RenderWidgetHostViewBase(),
    host_(content::RenderWidgetHostImpl::From(host)),
    web_view_(NULL),
    resource_collection_(new cc::DelegatedFrameResourceCollection()),
    last_output_surface_id_(0),
    frame_is_evicted_(true),
    selection_cursor_position_(0),
    selection_anchor_position_(0),
    is_loading_(false),
    gesture_recognizer_(ui::GestureRecognizer::Create()) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  resource_collection_->SetClient(this);
  gesture_recognizer_->AddGestureEventHelper(this);
  host_->SetView(this);
}

void RenderWidgetHostView::WasShown() {
  if (host_->is_hidden() && !frame_is_evicted_) {
    RendererFrameEvictor::GetInstance()->LockFrame(this);
  }
  host()->WasShown();
}

void RenderWidgetHostView::WasHidden() {
  if (!host_->is_hidden() && !frame_is_evicted_) {
    RendererFrameEvictor::GetInstance()->UnlockFrame(this);
  }
  host()->WasHidden();
  RunAckCallbacks();
}

bool RenderWidgetHostView::HasFocus() const {
  if (!web_view_) {
    return false;
  }

  return web_view_->HasFocus();
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

RenderWidgetHostView::~RenderWidgetHostView() {
  DCHECK(ack_callbacks_.empty());
  resource_collection_->SetClient(NULL);
  SetWebView(NULL);
}

void RenderWidgetHostView::CompositorDidCommit() {
  RunAckCallbacks();
}

void RenderWidgetHostView::SetWebView(WebView* view) {
  if (view == web_view_) {
    return;
  }

  DetachLayer();
  web_view_ = view;
  AttachLayer();

  if (web_view_) {
    host_->SendScreenRects();
    host_->WasResized();
  }
}

void RenderWidgetHostView::Blur() {
  host_->SetInputMethodActive(false);

  host_->SetActive(false);
  host_->Blur();
}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  last_size_ = size;
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
}

void RenderWidgetHostView::Focus() {
  host_->Focus();
  host_->SetActive(true);

  // XXX: Should we have a run-time check to see if this is required?
  host_->SetInputMethodActive(true);
}

} // namespace oxide
