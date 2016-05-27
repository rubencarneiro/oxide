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

#include "oxide_render_widget_host_view.h"

#include <utility>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/scoped_vector.h"
#include "cc/layers/surface_layer.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/delegated_frame_data.h"
#include "cc/output/viewport_selection_bound.h"
#include "cc/quads/render_pass.h"
#include "cc/surfaces/surface.h"
#include "cc/surfaces/surface_id_allocator.h"
#include "cc/surfaces/surface_manager.h"
#include "content/browser/renderer_host/render_widget_host_delegate.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/common/text_input_state.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebGestureDevice.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/touch/selection_bound.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/browser/input/oxide_input_method_context.h"

#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
#include "oxide_event_utils.h"
#include "oxide_render_widget_host_view_container.h"

namespace oxide {

namespace {

const float kMobileViewportWidthEpsilon = 0.15f;

bool ShouldSendPinchGesture() {
  static bool pinch_allowed =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableViewport) ||
      base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnablePinch);
  return pinch_allowed;
}

bool HasFixedPageScale(const cc::CompositorFrameMetadata& frame_metadata) {
  return frame_metadata.min_page_scale_factor ==
         frame_metadata.max_page_scale_factor;
}

bool HasMobileViewport(const cc::CompositorFrameMetadata& frame_metadata) {
  float window_width_dip =
      frame_metadata.page_scale_factor *
      frame_metadata.scrollable_viewport_size.width();
  float content_width_css = frame_metadata.root_layer_size.width();
  return content_width_css <= window_width_dip + kMobileViewportWidthEpsilon;
}

ui::SelectionBound::Type ConvertSelectionBoundType(
    cc::SelectionBoundType type) {
  switch (type) {
    case cc::SELECTION_BOUND_LEFT:
      return ui::SelectionBound::LEFT;
    case cc::SELECTION_BOUND_RIGHT:
      return ui::SelectionBound::RIGHT;
    case cc::SELECTION_BOUND_CENTER:
      return ui::SelectionBound::CENTER;
    case cc::SELECTION_BOUND_EMPTY:
      return ui::SelectionBound::EMPTY;
  }
  NOTREACHED() << "Unknown selection bound type";
  return ui::SelectionBound::EMPTY;
}

// Copied from content/browser/renderer_host/input/ui_touch_selection_helper.cc,
// because that helper is not part of content’s public API.
ui::SelectionBound ConvertSelectionBound(
    const cc::ViewportSelectionBound& bound) {
  ui::SelectionBound ui_bound;
  ui_bound.set_type(ConvertSelectionBoundType(bound.type));
  ui_bound.set_visible(bound.visible);
  if (ui_bound.type() != ui::SelectionBound::EMPTY)
    ui_bound.SetEdge(bound.edge_top, bound.edge_bottom);
  return ui_bound;
}

void SatisfyCallback(cc::SurfaceManager* manager,
                     cc::SurfaceSequence sequence) {
  std::vector<uint32_t> sequences;
  sequences.push_back(sequence.sequence);
  manager->DidSatisfySequences(sequence.id_namespace, &sequences);
}

void RequireCallback(cc::SurfaceManager* manager,
                     cc::SurfaceId id,
                     cc::SurfaceSequence sequence) {
  cc::Surface* surface = manager->GetSurfaceForId(id);
  if (!surface) {
    LOG(ERROR) << "Attempting to require callback on nonexistent surface";
    return;
  }
  surface->AddDestructionDependency(sequence);
}

bool HasLocationBarOffsetChanged(const cc::CompositorFrameMetadata& old,
                                 const cc::CompositorFrameMetadata& current) {
  if (old.location_bar_offset.y() != current.location_bar_offset.y()) {
    return true;
  }
  if (old.location_bar_content_translation.y() !=
      current.location_bar_content_translation.y()) {
    return true;
  }
  return false;
}

} // namespace

void RenderWidgetHostView::OnSelectionBoundsChanged(
    const gfx::Rect& anchor_rect,
    const gfx::Rect& focus_rect,
    bool is_anchor_first) {
  gfx::Rect caret_rect = gfx::UnionRects(anchor_rect, focus_rect);

  size_t selection_cursor_position = 0;
  size_t selection_anchor_position = 0;

  if (selection_range_.IsValid()) {
    if (is_anchor_first) {
      selection_cursor_position =
          selection_range_.GetMax() - selection_text_offset_;
      selection_anchor_position =
          selection_range_.GetMin() - selection_text_offset_;
    } else {
      selection_cursor_position =
          selection_range_.GetMin() - selection_text_offset_;
      selection_anchor_position =
          selection_range_.GetMax() - selection_text_offset_;
    }
  }

  ime_bridge_.SelectionBoundsChanged(caret_rect,
                                     selection_cursor_position,
                                     selection_anchor_position);

  if (container_) {
    container_->EditingCapabilitiesChanged(this);
  }
}

void RenderWidgetHostView::SelectionChanged(const base::string16& text,
                                            size_t offset,
                                            const gfx::Range& range) {
  if ((range.GetMin() - offset) > text.length()) {
    // Got an invalid selection (see https://launchpad.net/bugs/1375900).
    // The issue lies in content::RenderFrameImpl::SyncSelectionIfRequired(…)
    // where the selection text and the corresponding range are computed
    // separately. If the word that just got committed is at the beginning of a
    // new line, the selection range includes the trailing newline character(s)
    // whereas the selection text truncates them.
    // This looks very similar to https://crbug.com/101435.
    return;
  }

  content::RenderWidgetHostViewBase::SelectionChanged(text, offset, range);

  if (ime_bridge_.context()) {
    ime_bridge_.context()->SelectionChanged();
  }
}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  if (!container_) {
    return gfx::Size();
  }

  return container_->GetViewSizeInPixels();
}

bool RenderWidgetHostView::DoTopControlsShrinkBlinkSize() const {
  return top_controls_shrink_blink_size_;
}

float RenderWidgetHostView::GetTopControlsHeight() const {
  if (!container_) {
    return 0.0f;
  }

  if (container_->IsFullscreen()) {
    return 0.0f;
  }

  return container_->GetLocationBarHeight();
}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {
  ime_bridge_.FocusedNodeChanged(is_editable_node);
}

void RenderWidgetHostView::OnSwapCompositorFrame(
    uint32_t output_surface_id,
    std::unique_ptr<cc::CompositorFrame> frame) {
  if (!frame->delegated_frame_data) {
    DLOG(ERROR) << "Non delegated renderer path is not supported";
    host_->GetProcess()->ShutdownForBadMessage();
    return;
  }

  cc::DelegatedFrameData* frame_data = frame->delegated_frame_data.get();

  if (frame_data->render_pass_list.empty()) {
    DLOG(ERROR) << "Invalid delegated frame";
    host_->GetProcess()->ShutdownForBadMessage();
    return;
  }

  if (output_surface_id != last_output_surface_id_) {
    DestroyDelegatedContent();
    surface_factory_.reset();
    if (!surface_returned_resources_.empty()) {
      SendReturnedDelegatedResources();
    }
    last_output_surface_id_ = output_surface_id;
  }

  base::Closure ack_callback =
      base::Bind(&RenderWidgetHostView::SendDelegatedFrameAck,
                 weak_ptr_factory_.GetWeakPtr(),
                 output_surface_id);
  ack_callbacks_.push(ack_callback);

  float device_scale_factor = frame->metadata.device_scale_factor;
  cc::RenderPass* root_pass = frame_data->render_pass_list.back().get();

  gfx::Size frame_size = root_pass->output_rect.size();
  gfx::Size frame_size_dip = gfx::Size(
      std::lround(frame_size.width() / device_scale_factor),
      std::lround(frame_size.height() / device_scale_factor));

  gfx::Rect damage_rect_dip = gfx::ToEnclosingRect(
      gfx::ScaleRect(gfx::RectF(root_pass->damage_rect),
                     1.0f / device_scale_factor));

  cc::CompositorFrameMetadata metadata = frame->metadata;

  if (frame_size.IsEmpty()) {
    DestroyDelegatedContent();
  } else {
    cc::SurfaceManager* manager =
        CompositorUtils::GetInstance()->GetSurfaceManager();
    if (!surface_factory_) {
      DCHECK(manager);
      surface_factory_ =
          base::WrapUnique(new cc::SurfaceFactory(manager, this));
    }

    if (surface_id_.is_null() ||
        frame_size_dip != last_frame_size_dip_) {
      DestroyDelegatedContent();

      surface_id_ = id_allocator_->GenerateId();
      DCHECK(!surface_id_.is_null());
      surface_factory_->Create(surface_id_);

      layer_ =
          cc::SurfaceLayer::Create(base::Bind(&SatisfyCallback,
                                              base::Unretained(manager)),
                                   base::Bind(&RequireCallback,
                                              base::Unretained(manager)));
      DCHECK(layer_);
      layer_->SetSurfaceId(surface_id_, device_scale_factor, frame_size);
      layer_->SetBounds(frame_size_dip);
      layer_->SetIsDrawable(true);
      layer_->SetContentsOpaque(true);
      layer_->SetHideLayerAndSubtree(!is_showing_);

      AttachLayer();
    }

    cc::SurfaceFactory::DrawCallback ack_callback =
        base::Bind(&RenderWidgetHostView::RunAckCallbacks,
                   weak_ptr_factory_.GetWeakPtr());
    surface_factory_->SubmitCompositorFrame(surface_id_,
                                            std::move(frame),
                                            ack_callback);
  }

  last_submitted_frame_metadata_ = metadata;
  last_frame_size_dip_ = frame_size_dip;

  if (layer_.get()) {
    layer_->SetNeedsDisplayRect(damage_rect_dip);
  }

  bool shrink =
      metadata.location_bar_offset.y() == 0.0f &&
      metadata.location_bar_content_translation.y() > 0.0f;
  if (shrink != top_controls_shrink_blink_size_) {
    top_controls_shrink_blink_size_ = shrink;
    host_->WasResized();
  }

  bool has_mobile_viewport = HasMobileViewport(metadata);
  bool has_fixed_page_scale = HasFixedPageScale(metadata);
  gesture_provider_->SetDoubleTapSupportForPageEnabled(
      !has_fixed_page_scale && !has_mobile_viewport);

  if (host_->is_hidden()) {
    RunAckCallbacks(cc::SurfaceDrawStatus::DRAW_SKIPPED);
  }

  const cc::ViewportSelection& selection = metadata.selection;
  selection_controller_->OnSelectionEditable(selection.is_editable);
  selection_controller_->OnSelectionEmpty(selection.is_empty_text_form_control);
  selection_controller_->OnSelectionBoundsChanged(
      ConvertSelectionBound(selection.start),
      ConvertSelectionBound(selection.end));
}

void RenderWidgetHostView::ClearCompositorFrame() {
  DestroyDelegatedContent();
}

void RenderWidgetHostView::ProcessAckedTouchEvent(
    const content::TouchEventWithLatencyInfo& touch,
    content::InputEventAckState ack_result) {
  gesture_provider_->OnTouchEventAck(
      touch.event.uniqueTouchEventId,
      ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED);
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

void RenderWidgetHostView::UpdateCursor(const content::WebCursor& cursor) {
  if (cursor.IsEqual(web_cursor_)) {
    return;
  }

  web_cursor_ = cursor;

  if (is_loading_) {
    return;
  }

  UpdateCurrentCursor();
}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {
  if (is_loading == is_loading_) {
    return;
  }

  is_loading_ = is_loading;
  UpdateCurrentCursor();
}

void RenderWidgetHostView::TextInputStateChanged(
    const content::TextInputState& params) {
  ime_bridge_.TextInputStateChanged(params.type, params.show_ime_if_needed);
}

void RenderWidgetHostView::ImeCancelComposition() {
  if (!ime_bridge_.context()) {
    return;
  }

  ime_bridge_.context()->CancelComposition();
}

void RenderWidgetHostView::RenderProcessGone(base::TerminationStatus status,
                                             int error_code) {
  Destroy();
}

void RenderWidgetHostView::Destroy() {
  DestroyDelegatedContent();
  surface_factory_.reset();
  host_ = nullptr;
  delete this;
}

void RenderWidgetHostView::SetTooltipText(const base::string16& tooltip_text) {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const content::ReadbackRequestCallback& callback,
    const SkColorType color_type) {
  callback.Run(SkBitmap(), content::READBACK_FAILED);
}

void RenderWidgetHostView::CopyFromCompositingSurfaceToVideoFrame(
    const gfx::Rect& src_subrect,
    const scoped_refptr<media::VideoFrame>& target,
    const base::Callback<void(const gfx::Rect&, bool)>& callback) {
  NOTIMPLEMENTED();
  callback.Run(gfx::Rect(), false);
}

bool RenderWidgetHostView::CanCopyToVideoFrame() const {
  return false;
}

bool RenderWidgetHostView::HasAcceleratedSurface(
    const gfx::Size& desired_size) {
  return false;
}

void RenderWidgetHostView::GetScreenInfo(blink::WebScreenInfo* result) {
  if (!container_) {
    RenderWidgetHostViewOxide::GetDefaultScreenInfo(result);
    return;
  }

  *result = container_->GetScreenInfo();
}

bool RenderWidgetHostView::GetScreenColorProfile(
    std::vector<char>* color_profile) {
  DCHECK(color_profile->empty());
  NOTREACHED();
  return false;
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow() {
  return GetViewBounds();
}

void RenderWidgetHostView::ShowDisambiguationPopup(
    const gfx::Rect& rect_pixels,
    const SkBitmap& zoomed_bitmap) {}

void RenderWidgetHostView::LockCompositingSurface() {
  NOTIMPLEMENTED();
}

void RenderWidgetHostView::UnlockCompositingSurface() {
  NOTIMPLEMENTED();
}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {}

gfx::Vector2dF RenderWidgetHostView::GetLastScrollOffset() const {
  NOTREACHED();
  return gfx::Vector2dF();
}

gfx::NativeView RenderWidgetHostView::GetNativeView() const {
  return nullptr;
}

gfx::NativeViewAccessible RenderWidgetHostView::GetNativeViewAccessible() {
  return nullptr;
}

bool RenderWidgetHostView::HasFocus() const {
  if (!container_) {
    return false;
  }

  return container_->HasFocus();
}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

bool RenderWidgetHostView::IsShowing() {
  return is_showing_ && container_;
}

gfx::Rect RenderWidgetHostView::GetViewBounds() const {
  gfx::Rect bounds;

  if (!container_) {
    bounds = gfx::Rect(last_size_);
  } else {
    bounds = container_->GetViewBounds();
  }

  if (DoTopControlsShrinkBlinkSize()) {
    bounds.Inset(0, GetTopControlsHeight(), 0, 0);
  }

  return bounds;
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::CompositorDidCommit() {
  committed_frame_metadata_ = last_submitted_frame_metadata_;
  RunAckCallbacks(cc::SurfaceDrawStatus::DRAWN);
}

void RenderWidgetHostView::CompositorWillRequestSwapFrame() {
  cc::CompositorFrameMetadata old = displayed_frame_metadata_;
  displayed_frame_metadata_ = committed_frame_metadata_;

  if (!container_) {
    return;
  }

  // If the location bar offset changes while a touch selection is active,
  // the bounding rect and the position of the handles need to be updated.
  if ((selection_controller_->active_status() !=
          ui::TouchSelectionController::INACTIVE) &&
      HasLocationBarOffsetChanged(old, displayed_frame_metadata_)) {
    NotifyTouchSelectionChanged();
    // XXX: hack to ensure the position of the handles is updated.
    selection_controller_->SetTemporarilyHidden(true);
    selection_controller_->SetTemporarilyHidden(false);
  }
}

void RenderWidgetHostView::CompositorEvictResources() {
  DestroyDelegatedContent();
}

void RenderWidgetHostView::OnGestureEvent(
    const blink::WebGestureEvent& event) {
  if (!host_) {
    return;
  }

  if ((event.type == blink::WebInputEvent::GesturePinchBegin ||
       event.type == blink::WebInputEvent::GesturePinchUpdate ||
       event.type == blink::WebInputEvent::GesturePinchEnd) &&
      !ShouldSendPinchGesture()) {
    return;
  }

  if (HandleGestureForTouchSelection(event)) {
    return;
  }

  if (event.type == blink::WebInputEvent::GestureTapDown) {
    // Webkit does not stop a fling-scroll on tap-down. So explicitly send an
    // event to stop any in-progress flings.
    blink::WebGestureEvent fling_cancel = event;
    fling_cancel.type = blink::WebInputEvent::GestureFlingCancel;
    fling_cancel.sourceDevice = blink::WebGestureDeviceTouchpad;
    host_->ForwardGestureEvent(fling_cancel);
  }

  if (event.type == blink::WebInputEvent::Undefined) {
    return;
  }

  host_->ForwardGestureEvent(event);
}

bool RenderWidgetHostView::HandleContextMenu(
    const content::ContextMenuParams& params) {
  if ((params.source_type == ui::MENU_SOURCE_LONG_PRESS) &&
      params.is_editable &&
      params.selection_text.empty()) {
    return true;
  }

  selection_controller_->HideAndDisallowShowingAutomatically();
  return false;
}

bool RenderWidgetHostView::HandleGestureForTouchSelection(
    const blink::WebGestureEvent& event) const {
  switch (event.type) {
    case blink::WebInputEvent::GestureLongPress: {
      base::TimeTicks event_time = base::TimeTicks() +
          base::TimeDelta::FromSecondsD(event.timeStampSeconds);
      gfx::PointF location(event.x, event.y);
      if (selection_controller_->WillHandleLongPressEvent(
              event_time, location)) {
        return true;
      }
      break;
    }
    case blink::WebInputEvent::GestureTap: {
      gfx::PointF location(event.x, event.y);
      if (selection_controller_->WillHandleTapEvent(
              location, event.data.tap.tapCount)) {
        return true;
      }
      break;
    }
    case blink::WebInputEvent::GestureScrollBegin:
      // XXX: currently commented out because when doing a pinch-to-zoom
      // gesture, we don’t always get the corresponding GestureScrollEnd event,
      // so selection handles would remain hidden.
      //selection_controller()->SetTemporarilyHidden(true);
      break;
    case blink::WebInputEvent::GestureScrollEnd:
      // XXX: see above
      //selection_controller()->SetTemporarilyHidden(false);
      break;
    default:
      break;
  }
  return false;
}

void RenderWidgetHostView::NotifyTouchSelectionChanged() {
  if (!container_) {
    return;
  }

  container_->TouchSelectionChanged(this, handle_drag_in_progress_);
}

void RenderWidgetHostView::ReturnResources(
    const cc::ReturnedResourceArray& resources) {
  if (resources.empty()) {
    return;
  }
  std::copy(resources.begin(), resources.end(),
            std::back_inserter(surface_returned_resources_));
  if (ack_callbacks_.empty()) {
    SendReturnedDelegatedResources();
  }
}

void RenderWidgetHostView::SetBeginFrameSource(
    cc::BeginFrameSource* begin_frame_source) {}

bool RenderWidgetHostView::SupportsAnimation() const {
  return false;
}

void RenderWidgetHostView::SetNeedsAnimate() {
  NOTREACHED();
}

void RenderWidgetHostView::MoveCaret(const gfx::PointF& position) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(host_);
  rwhi->MoveCaret(gfx::ToRoundedPoint(position));
}

void RenderWidgetHostView::MoveRangeSelectionExtent(const gfx::PointF& extent) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(host_);
  content::RenderWidgetHostDelegate* host_delegate = rwhi->delegate();
  if (host_delegate) {
    host_delegate->MoveRangeSelectionExtent(gfx::ToRoundedPoint(extent));
  }
}

void RenderWidgetHostView::SelectBetweenCoordinates(const gfx::PointF& base,
                                                    const gfx::PointF& extent) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(host_);
  content::RenderWidgetHostDelegate* host_delegate = rwhi->delegate();
  if (host_delegate) {
    host_delegate->SelectRange(gfx::ToRoundedPoint(base),
                               gfx::ToRoundedPoint(extent));
  }
}

void RenderWidgetHostView::OnSelectionEvent(ui::SelectionEventType event) {
  switch (event) {
    case ui::SELECTION_HANDLE_DRAG_STARTED:
    case ui::INSERTION_HANDLE_DRAG_STARTED:
      handle_drag_in_progress_ = true;
      break;
    case ui::SELECTION_HANDLE_DRAG_STOPPED:
    case ui::INSERTION_HANDLE_DRAG_STOPPED:
      handle_drag_in_progress_ = false;
      break;
    case ui::INSERTION_HANDLE_TAPPED:
      break;
    default:
      break;
  }
  NotifyTouchSelectionChanged();
}

std::unique_ptr<ui::TouchHandleDrawable>
RenderWidgetHostView::CreateDrawable() {
  if (!container_) {
    return nullptr;
  }

  return base::WrapUnique(container_->CreateTouchHandleDrawable());
}

void RenderWidgetHostView::UpdateCurrentCursor() {
  if (is_loading_) {
    content::WebCursor::CursorInfo busy_cursor_info(
        blink::WebCursorInfo::TypeWait);
    current_cursor_.InitFromCursorInfo(busy_cursor_info);
  } else {
    current_cursor_ = web_cursor_;
  }

  if (!container_) {
    return;
  }

  container_->CursorChanged(this);
}

void RenderWidgetHostView::DestroyDelegatedContent() {
  DetachLayer();
  if (!surface_id_.is_null()) {
    DCHECK(surface_factory_.get());
    surface_factory_->Destroy(surface_id_);
    surface_id_ = cc::SurfaceId();
  }
  layer_ = nullptr;
}

void RenderWidgetHostView::SendDelegatedFrameAck(uint32_t surface_id) {
  cc::CompositorFrameAck ack;
  ack.resources.swap(surface_returned_resources_);

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      host_->GetRoutingID(),
      surface_id,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::SendReturnedDelegatedResources() {
  DCHECK(host_);

  cc::CompositorFrameAck ack;
  DCHECK(!surface_returned_resources_.empty());
  ack.resources.swap(surface_returned_resources_);

  content::RenderWidgetHostImpl::SendReclaimCompositorResources(
      host_->GetRoutingID(),
      last_output_surface_id_,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::RunAckCallbacks(cc::SurfaceDrawStatus status) {
  while (!ack_callbacks_.empty()) {
    ack_callbacks_.front().Run();
    ack_callbacks_.pop();
  }
}

void RenderWidgetHostView::AttachLayer() {
  if (!container_) {
    return;
  }
  if (!layer_.get()) {
    return;
  }

  container_->AttachLayer(layer_);
}

void RenderWidgetHostView::DetachLayer() {
  if (!container_) {
    return;
  }
  if (!layer_.get()) {
    return;
  }

  container_->DetachLayer(layer_);
}

RenderWidgetHostView::RenderWidgetHostView(
    content::RenderWidgetHostImpl* host)
    : host_(host),
      container_(nullptr),
      id_allocator_(
          CompositorUtils::GetInstance()->CreateSurfaceIdAllocator()),
      last_output_surface_id_(0),
      ime_bridge_(this),
      is_loading_(false),
      is_showing_(!host->is_hidden()),
      top_controls_shrink_blink_size_(false),
      gesture_provider_(GestureProvider::Create(this)),
      handle_drag_in_progress_(false),
      weak_ptr_factory_(this) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  host_->SetView(this);

  gesture_provider_->SetDoubleTapSupportForPageEnabled(false);

  ui::TouchSelectionController::Config tsc_config;
  // default values from ui/events/gesture_detection/gesture_configuration.cc
  tsc_config.max_tap_duration = base::TimeDelta::FromMilliseconds(150);
  tsc_config.tap_slop = 15;
  tsc_config.enable_adaptive_handle_orientation = true;
  tsc_config.enable_longpress_drag_selection = false;
  selection_controller_.reset(
      new ui::TouchSelectionController(this, tsc_config));
}

RenderWidgetHostView::~RenderWidgetHostView() {
  DCHECK(!layer_);
  DCHECK(!surface_factory_);
  DCHECK(surface_id_.is_null());
}

void RenderWidgetHostView::SetContainer(
    RenderWidgetHostViewContainer* container) {
  if (container == container_) {
    return;
  }

  DetachLayer();
  container_ = container;
  AttachLayer();

  CompositorObserver::Observe(
      container_ ? container_->GetCompositor() : nullptr);

  if (!host_) {
    return;
  }

  host_->SendScreenRects();
  host_->WasResized();
}

void RenderWidgetHostView::Blur() {
  host_->SetActive(false);
  host_->Blur();

  selection_controller_->HideAndDisallowShowingAutomatically();
}

void RenderWidgetHostView::HandleTouchEvent(const ui::MotionEvent& event) {
  if (selection_controller_->WillHandleTouchEvent(event)) {
    return;
  }

  auto rv = gesture_provider_->OnTouchEvent(event);
  if (!rv.succeeded) {
    return;
  }

  if (!host_) {
    gesture_provider_->OnTouchEventAck(event.GetUniqueEventId(), false);
    return;
  }

  host_->ForwardTouchEventWithLatencyInfo(
      MakeWebTouchEvent(event, rv.moved_beyond_slop_region),
      ui::LatencyInfo());
}

void RenderWidgetHostView::ResetGestureDetection() {
  const ui::MotionEvent* current_down_event =
      gesture_provider_->GetCurrentDownEvent();
  if (current_down_event) {
    std::unique_ptr<ui::MotionEvent> cancel_event = current_down_event->Cancel();
    HandleTouchEvent(*cancel_event);
  }

  gesture_provider_->ResetDetection();
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
}

void RenderWidgetHostView::Show() {
  if (is_showing_) {
    return;
  }
  is_showing_ = true;

  if (layer_.get()) {
    layer_->SetHideLayerAndSubtree(false);
  }

  if (!host_ || !host_->is_hidden()) {
    return;
  }

  host_->WasShown(ui::LatencyInfo());
}

void RenderWidgetHostView::Hide() {
  if (!is_showing_) {
    return;
  }
  is_showing_ = false;

  if (layer_.get()) {
    layer_->SetHideLayerAndSubtree(true);
  }

  RunAckCallbacks(cc::SurfaceDrawStatus::DRAW_SKIPPED);

  if (!host_ || host_->is_hidden()) {
    return;
  }

  host_->WasHidden();
}

} // namespace oxide
