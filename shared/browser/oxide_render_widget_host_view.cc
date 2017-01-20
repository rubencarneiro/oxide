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
#include "cc/quads/render_pass.h"
#include "cc/surfaces/surface.h"
#include "cc/surfaces/surface_factory.h"
#include "cc/surfaces/surface_id.h"
#include "cc/surfaces/surface_id_allocator.h"
#include "cc/surfaces/surface_info.h"
#include "cc/surfaces/surface_manager.h"
#include "content/browser/renderer_host/render_widget_host_delegate.h" // nogncheck
#include "content/browser/renderer_host/render_widget_host_impl.h" // nogncheck
#include "content/common/text_input_state.h" // nogncheck
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebGestureDevice.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gfx/selection_bound.h"
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

bool HasLocationBarOffsetChanged(const cc::CompositorFrameMetadata& old,
                                 const cc::CompositorFrameMetadata& current) {
  if (old.top_controls_height != current.top_controls_height) {
    return true;
  }
  if (old.top_controls_shown_ratio != current.top_controls_shown_ratio) {
    return true;
  }
  return false;
}

} // namespace

void RenderWidgetHostView::OnSelectionBoundsChanged(
    const gfx::Rect& anchor_rect,
    const gfx::Rect& focus_rect,
    bool is_anchor_first) {
  gfx::Rect caret_rect;
  if (anchor_rect == focus_rect) {
    caret_rect = anchor_rect;
  }

  size_t selection_cursor_position = 0;
  size_t selection_anchor_position = 0;

  const content::TextInputManager::TextSelection* selection =
      GetTextInputManager()->GetTextSelection();
  if (selection && selection->range.IsValid()) {
    if (is_anchor_first) {
      selection_cursor_position =
          selection->range.GetMax() - selection->offset;
      selection_anchor_position =
          selection->range.GetMin() - selection->offset;
    } else {
      selection_cursor_position =
          selection->range.GetMin() - selection->offset;
      selection_anchor_position =
          selection->range.GetMax() - selection->offset;
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

bool RenderWidgetHostView::DoBrowserControlsShrinkBlinkSize() const {
  return browser_controls_shrink_blink_size_;
}

float RenderWidgetHostView::GetTopControlsHeight() const {
  if (!container_) {
    return 0.0f;
  }

  if (container_->IsFullscreen()) {
    return 0.0f;
  }

  return container_->GetTopControlsHeight();
}

void RenderWidgetHostView::FocusedNodeChanged(
    bool is_editable_node,
    const gfx::Rect& node_bounds_in_screen) {
  ime_bridge_.FocusedNodeChanged(is_editable_node);
}

void RenderWidgetHostView::OnSwapCompositorFrame(uint32_t output_surface_id,
                                                 cc::CompositorFrame frame) {
  if (frame.render_pass_list.empty()) {
    DLOG(ERROR) << "Invalid delegated frame";
    host_->GetProcess()->ShutdownForBadMessage(
        content::RenderProcessHost::CrashReportMode::GENERATE_CRASH_DUMP);
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

  cc::CompositorFrameMetadata metadata = frame.metadata.Clone();

  float device_scale_factor = metadata.device_scale_factor;
  cc::RenderPass* root_pass = frame.render_pass_list.back().get();

  gfx::Size frame_size = root_pass->output_rect.size();
  gfx::Size frame_size_dip = gfx::Size(
      std::lround(frame_size.width() / device_scale_factor),
      std::lround(frame_size.height() / device_scale_factor));

  gfx::Rect damage_rect_dip = gfx::ToEnclosingRect(
      gfx::ScaleRect(gfx::RectF(root_pass->damage_rect),
                     1.0f / device_scale_factor));

  if (frame_size.IsEmpty()) {
    DestroyDelegatedContent();
  } else {
    cc::SurfaceManager* manager =
        CompositorUtils::GetInstance()->GetSurfaceManager();
    if (!surface_factory_) {
      DCHECK(manager);
      surface_factory_ =
          base::MakeUnique<cc::SurfaceFactory>(frame_sink_id_, manager, this);
    }

    if (!local_frame_id_.is_valid() ||
        frame_size_dip != last_frame_size_dip_) {
      DestroyDelegatedContent();

      local_frame_id_ = id_allocator_->GenerateId();
      DCHECK(local_frame_id_.is_valid());

      layer_ = cc::SurfaceLayer::Create(manager->reference_factory());
      DCHECK(layer_);

      layer_->SetSurfaceInfo(
          cc::SurfaceInfo(cc::SurfaceId(frame_sink_id_, local_frame_id_),
                          device_scale_factor,
                          frame_size));
      layer_->SetBounds(frame_size_dip);
      layer_->SetIsDrawable(true);
      layer_->SetContentsOpaque(true);
      layer_->SetHideLayerAndSubtree(!is_showing_);

      AttachLayer();
    }

    cc::SurfaceFactory::DrawCallback ack_callback =
        base::Bind(&RenderWidgetHostView::RunAckCallbacks,
                   weak_ptr_factory_.GetWeakPtr());
    surface_factory_->SubmitCompositorFrame(local_frame_id_,
                                            std::move(frame),
                                            ack_callback);
  }

  if (layer_.get()) {
    layer_->SetNeedsDisplayRect(damage_rect_dip);
  }

  bool shrink =
      metadata.top_controls_height > 0.f &&
      metadata.top_controls_shown_ratio == 1.f;
  if (shrink != browser_controls_shrink_blink_size_) {
    browser_controls_shrink_blink_size_ = shrink;
    host_->WasResized();
  }

  bool has_mobile_viewport = HasMobileViewport(metadata);
  bool has_fixed_page_scale = HasFixedPageScale(metadata);
  gesture_provider_->SetDoubleTapSupportForPageEnabled(
      !has_fixed_page_scale && !has_mobile_viewport);

  if (host_->is_hidden()) {
    RunAckCallbacks();
  }

  const cc::Selection<gfx::SelectionBound>& selection = metadata.selection;
  selection_controller_->OnSelectionBoundsChanged(selection.start,
                                                  selection.end);

  last_submitted_frame_metadata_ = std::move(metadata);
  last_frame_size_dip_ = frame_size_dip;
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

cc::FrameSinkId RenderWidgetHostView::GetFrameSinkId() {
  return frame_sink_id_;
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

  if (DoBrowserControlsShrinkBlinkSize()) {
    bounds.Inset(0, GetTopControlsHeight(), 0, 0);
  }

  return bounds;
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::SetNeedsBeginFrames(bool needs_begin_frames) {}

void RenderWidgetHostView::CompositorDidCommit() {
  committed_frame_metadata_ = std::move(last_submitted_frame_metadata_);
  RunAckCallbacks();
}

void RenderWidgetHostView::CompositorWillRequestSwapFrame() {
  cc::CompositorFrameMetadata old = std::move(displayed_frame_metadata_);
  displayed_frame_metadata_ = std::move(committed_frame_metadata_);

  if (!container_) {
    return;
  }

  // If the location bar offset changes while a touch selection is active,
  // the bounding rect and the position of the handles need to be updated.
  if ((selection_controller_->active_status() !=
          ui::TouchSelectionController::INACTIVE) &&
      HasLocationBarOffsetChanged(old, displayed_frame_metadata_)) {
    NotifyTouchSelectionChanged(false);
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

  HandleGestureForTouchSelection(event);

  if (event.type() == blink::WebInputEvent::GestureTapDown) {
    // Webkit does not stop a fling-scroll on tap-down. So explicitly send an
    // event to stop any in-progress flings.
    blink::WebGestureEvent fling_cancel = event;
    fling_cancel.setType(blink::WebInputEvent::GestureFlingCancel);
    fling_cancel.sourceDevice = blink::WebGestureDeviceTouchpad;
    host_->ForwardGestureEvent(fling_cancel);
  }

  if (event.type() == blink::WebInputEvent::Undefined) {
    return;
  }

  host_->ForwardGestureEvent(event);
}

void RenderWidgetHostView::OnUserInput() const {
  if (selection_controller_->active_status() ==
          ui::TouchSelectionController::INSERTION_ACTIVE) {
    selection_controller_->HideAndDisallowShowingAutomatically();
  }
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

void RenderWidgetHostView::HandleGestureForTouchSelection(
    const blink::WebGestureEvent& event) const {
  switch (event.type()) {
    case blink::WebInputEvent::GestureLongPress: {
      base::TimeTicks event_time = base::TimeTicks() +
          base::TimeDelta::FromSecondsD(event.timeStampSeconds());
      gfx::PointF location(event.x, event.y);
      selection_controller_->HandleLongPressEvent(event_time, location);
      break;
    }
    case blink::WebInputEvent::GestureTap: {
      gfx::PointF location(event.x, event.y);
      selection_controller_->HandleTapEvent(location, event.data.tap.tapCount);
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
}

void RenderWidgetHostView::NotifyTouchSelectionChanged(
    bool insertion_handle_tapped) {
  if (!container_) {
    return;
  }

  container_->TouchSelectionChanged(this,
                                    handle_drag_in_progress_,
                                    insertion_handle_tapped);
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

void RenderWidgetHostView::OnUpdateTextInputStateCalled(
    content::TextInputManager* text_input_manager,
    content::RenderWidgetHostViewBase* updated_view,
    bool did_update_state) {
  if (updated_view != this) {
    return;
  }

  const content::TextInputState* state =
      GetTextInputManager()->GetTextInputState();

  ime_bridge_.TextInputStateChanged(
      state ? state->type : ui::TEXT_INPUT_TYPE_NONE,
      state ? state->show_ime_if_needed : false);
}

void RenderWidgetHostView::OnImeCancelComposition(
    content::TextInputManager* text_input_manager,
    content::RenderWidgetHostViewBase* updated_view) {
  if (ime_bridge_.context() || updated_view != this) {
    return;
  }

  ime_bridge_.context()->CancelComposition();
}

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
  bool insertion_handle_tapped = false;
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
      insertion_handle_tapped = true;
      break;
    default:
      break;
  }
  NotifyTouchSelectionChanged(insertion_handle_tapped);
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
  if (local_frame_id_.is_valid()) {
    DCHECK(surface_factory_.get());
    surface_factory_->EvictSurface();
    local_frame_id_ = cc::LocalFrameId();
  }
  layer_ = nullptr;
}

void RenderWidgetHostView::SendDelegatedFrameAck(uint32_t surface_id) {
  content::RenderWidgetHostImpl::SendReclaimCompositorResources(
      host_->GetRoutingID(),
      surface_id,
      host_->GetProcess()->GetID(),
      true, // is_swap_ack
      surface_returned_resources_);
  surface_returned_resources_.clear();
}

void RenderWidgetHostView::SendReturnedDelegatedResources() {
  DCHECK(host_);

  content::RenderWidgetHostImpl::SendReclaimCompositorResources(
      host_->GetRoutingID(),
      last_output_surface_id_,
      host_->GetProcess()->GetID(),
      false, // is_swap_ack
      surface_returned_resources_);
  surface_returned_resources_.clear();
}

void RenderWidgetHostView::RunAckCallbacks() {
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
      frame_sink_id_(CompositorUtils::GetInstance()->AllocateFrameSinkId()),
      id_allocator_(new cc::SurfaceIdAllocator()),
      last_output_surface_id_(0),
      ime_bridge_(this),
      is_loading_(false),
      is_showing_(!host->is_hidden()),
      browser_controls_shrink_blink_size_(false),
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

  CompositorUtils::GetInstance()
      ->GetSurfaceManager()
      ->RegisterFrameSinkId(frame_sink_id_);

  if (GetTextInputManager()) {
    GetTextInputManager()->AddObserver(this);
  }
}

RenderWidgetHostView::~RenderWidgetHostView() {
  DCHECK(!layer_);
  DCHECK(!surface_factory_);
  DCHECK(!local_frame_id_.is_valid());

  if (text_input_manager_) {
    text_input_manager_->RemoveObserver(this);
  }

  CompositorUtils::GetInstance()
      ->GetSurfaceManager()
      ->InvalidateFrameSinkId(frame_sink_id_);
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

base::string16 RenderWidgetHostView::GetSelectionText() const {
  auto* self = const_cast<RenderWidgetHostView*>(this);
  if (!self->GetTextInputManager() ||
      !self->GetTextInputManager()->GetTextSelection(self)) {
    return base::string16();
  }
  return self->GetTextInputManager()->GetTextSelection(self)->text;
}

gfx::Range RenderWidgetHostView::GetSelectionRange() const {
  auto* self = const_cast<RenderWidgetHostView*>(this);
  if (!self->GetTextInputManager() ||
      !self->GetTextInputManager()->GetTextSelection(self)) {
    return gfx::Range();
  }
  return self->GetTextInputManager()->GetTextSelection(self)->range;
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

  RunAckCallbacks();

  if (!host_ || host_->is_hidden()) {
    return;
  }

  host_->WasHidden();
}

} // namespace oxide
