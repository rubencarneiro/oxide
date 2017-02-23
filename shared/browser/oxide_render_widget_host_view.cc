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
#include "cc/surfaces/surface_hittest.h"
#include "cc/surfaces/surface_id.h"
#include "cc/surfaces/surface_id_allocator.h"
#include "cc/surfaces/surface_info.h"
#include "cc/surfaces/surface_manager.h"
#include "content/browser/renderer_host/render_widget_host_delegate.h" // nogncheck
#include "content/browser/renderer_host/render_widget_host_impl.h" // nogncheck
#include "content/browser/renderer_host/render_widget_host_input_event_router.h" // nogncheck
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
#include "ui/gfx/geometry/dip_util.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gfx/selection_bound.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/clipboard/oxide_clipboard.h"
#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/common/oxide_enum_flags.h"

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

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

} // namespace

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
  if (!container_) {
    return;
  }

  container_->FocusedNodeChanged(this, is_editable_node);
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

  const cc::CompositorFrameMetadata& metadata = frame.metadata;

  float device_scale_factor = metadata.device_scale_factor;
  cc::RenderPass* root_pass = frame.render_pass_list.back().get();

  gfx::Size frame_size = root_pass->output_rect.size();
  gfx::Size frame_size_dip = gfx::Size(
      std::lround(frame_size.width() / device_scale_factor),
      std::lround(frame_size.height() / device_scale_factor));

  bool shrink =
      metadata.top_controls_height > 0.f &&
      metadata.top_controls_shown_ratio == 1.f;
  bool has_mobile_viewport = HasMobileViewport(metadata);
  bool has_fixed_page_scale = HasFixedPageScale(metadata);
  cc::Selection<gfx::SelectionBound> selection = metadata.selection;

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

    if (!local_surface_id_.is_valid() ||
        frame_size_dip != last_frame_size_dip_) {
      DestroyDelegatedContent();

      local_surface_id_ = id_allocator_->GenerateId();
      DCHECK(local_surface_id_.is_valid());

      layer_ = cc::SurfaceLayer::Create(manager->reference_factory());
      DCHECK(layer_);

      layer_->SetSurfaceInfo(
          cc::SurfaceInfo(cc::SurfaceId(frame_sink_id_, local_surface_id_),
                          device_scale_factor,
                          frame_size));
      layer_->SetBounds(frame_size_dip);
      layer_->SetIsDrawable(true);
      layer_->SetContentsOpaque(true);
      layer_->SetHideLayerAndSubtree(!is_showing_);

      AttachLayer();
    }

    cc::SurfaceFactory::DrawCallback ack_callback =
        base::Bind(&RenderWidgetHostView::SurfaceDrawn,
                   weak_ptr_factory_.GetWeakPtr(),
                   local_surface_id_,
                   base::Passed(metadata.Clone()));
    surface_factory_->SubmitCompositorFrame(local_surface_id_,
                                            std::move(frame),
                                            ack_callback);
  }

  last_frame_size_dip_ = frame_size_dip;

  if (shrink != browser_controls_shrink_blink_size_) {
    browser_controls_shrink_blink_size_ = shrink;
    host_->WasResized();
  }

  gesture_provider_->SetDoubleTapSupportForPageEnabled(
      !has_fixed_page_scale && !has_mobile_viewport);

  selection_controller_->OnSelectionBoundsChanged(selection.start,
                                                  selection.end);

  if (host_->is_hidden()) {
    RunAckCallbacks();
  }
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

cc::FrameSinkId RenderWidgetHostView::FrameSinkIdAtPoint(
    cc::SurfaceHittestDelegate* delegate,
    const gfx::Point& point,
    gfx::Point* transformed_point) {
  if (!container_) {
    return cc::FrameSinkId();
  }

  gfx::Point point_in_pixels =
      gfx::ConvertPointToPixel(
          container_->GetCompositor()->device_scale_factor(), point);

  cc::SurfaceId surface_id(frame_sink_id_, local_surface_id_);
  if (!surface_id.is_valid()) {
    return cc::FrameSinkId();
  }

  gfx::Transform transform;
  cc::SurfaceHittest hittest(
      delegate,
      CompositorUtils::GetInstance()->GetSurfaceManager());
  cc::SurfaceId id =
      hittest.GetTargetSurfaceAtPoint(surface_id, point_in_pixels,
                                      &transform);

  *transformed_point = point_in_pixels;
  if (id.is_valid()) {
    transform.TransformPoint(transformed_point);
  }

  *transformed_point =
      gfx::ConvertPointToDIP(
          container_->GetCompositor()->device_scale_factor(),
          *transformed_point);

  if (!id.is_valid()) {
    return GetFrameSinkId();
  }

  return id.frame_sink_id();
}

void RenderWidgetHostView::ProcessMouseEvent(const blink::WebMouseEvent& event,
                                             const ui::LatencyInfo& latency) {
  GetRenderWidgetHost()->ForwardMouseEvent(event);
}

void RenderWidgetHostView::ProcessMouseWheelEvent(
    const blink::WebMouseWheelEvent& event,
    const ui::LatencyInfo& latency) {
  GetRenderWidgetHost()->ForwardWheelEvent(event);
}

void RenderWidgetHostView::ProcessTouchEvent(const blink::WebTouchEvent& event,
                                             const ui::LatencyInfo& latency) {
  host_->ForwardTouchEventWithLatencyInfo(event, latency);
}

void RenderWidgetHostView::ProcessGestureEvent(
    const blink::WebGestureEvent& event,
    const ui::LatencyInfo& latency) {
  host_->ForwardGestureEventWithLatencyInfo(event, latency);
}

bool RenderWidgetHostView::TransformPointToLocalCoordSpace(
    const gfx::Point& point,
    const cc::SurfaceId& original_surface,
    gfx::Point* transformed_point) {
  if (!container_) {
    return false;
  }

  gfx::Point point_in_pixels =
      gfx::ConvertPointToPixel(
          container_->GetCompositor()->device_scale_factor(), point);

  cc::SurfaceId surface_id(frame_sink_id_, local_surface_id_);
  if (!surface_id.is_valid()) {
    return false;
  }

  *transformed_point = point_in_pixels;
  if (original_surface != surface_id) {
    cc::SurfaceHittest hittest(
        nullptr,
        CompositorUtils::GetInstance()->GetSurfaceManager());
    if (!hittest.TransformPointToTargetSurface(original_surface, surface_id,
                                               transformed_point)) {
      return false;
    }
  }

  *transformed_point =
      gfx::ConvertPointToDIP(
          container_->GetCompositor()->device_scale_factor(),
          *transformed_point);
  return true;
}

bool RenderWidgetHostView::TransformPointToCoordSpaceForView(
    const gfx::Point& point,
    content::RenderWidgetHostViewBase* target_view,
    gfx::Point* transformed_point) {
  if (target_view == this) {
    *transformed_point = point;
    return true;
  }

  if (!local_surface_id_.is_valid()) {
    return false;
  }

  return target_view->TransformPointToLocalCoordSpace(
      point,
      cc::SurfaceId(frame_sink_id_, local_surface_id_),
      transformed_point);
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

void RenderWidgetHostView::CompositorEvictResources() {
  DestroyDelegatedContent();
}

void RenderWidgetHostView::OnGestureEvent(blink::WebGestureEvent event) {
  if (!host_) {
    return;
  }

  HandleGestureForTouchSelection(event);

  content::RenderWidgetHostInputEventRouter* router =
      host_->delegate()->GetInputEventRouter();

  if (event.type() == blink::WebInputEvent::GestureTapDown) {
    // Webkit does not stop a fling-scroll on tap-down. So explicitly send an
    // event to stop any in-progress flings.
    blink::WebGestureEvent fling_cancel = event;
    fling_cancel.setType(blink::WebInputEvent::GestureFlingCancel);
    fling_cancel.sourceDevice = blink::WebGestureDeviceTouchpad;
    router->RouteGestureEvent(this, &fling_cancel, ui::LatencyInfo());
  }

  if (event.type() == blink::WebInputEvent::Undefined) {
    return;
  }

  router->RouteGestureEvent(this, &event, ui::LatencyInfo());
}

void RenderWidgetHostView::OnUserInput() const {
  if (selection_controller_->active_status() ==
          ui::TouchSelectionController::INSERTION_ACTIVE) {
    selection_controller_->HideAndDisallowShowingAutomatically();
  }
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

void RenderWidgetHostView::NotifyTouchSelectionChanged() {
  if (!container_) {
    return;
  }

  container_->TouchEditingStatusChanged(this);
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
  if (!container_) {
    return;
  }

  const content::TextInputState* state =
      GetTextInputManager()->GetTextInputState();
  container_->TextInputStateChanged(this, state);
}

void RenderWidgetHostView::OnImeCancelComposition(
    content::TextInputManager* text_input_manager,
    content::RenderWidgetHostViewBase* updated_view) {
  if (!container_) {
    return;
  }

  container_->ImeCancelComposition(this);
}

void RenderWidgetHostView::OnSelectionBoundsChanged(
    content::TextInputManager* text_input_manager,
    content::RenderWidgetHostViewBase* updated_view) {
  if (!container_) {
    return;
  }

  if (updated_view != GetFocusedViewForTextSelection()) {
    return;
  }

  const content::TextInputManager::SelectionRegion* region =
      GetTextInputManager()->GetSelectionRegion(updated_view);
  container_->SelectionBoundsChanged(this, region);
}

void RenderWidgetHostView::OnTextSelectionChanged(
    content::TextInputManager* text_input_manager,
    content::RenderWidgetHostViewBase* updated_view) {
  if (!container_) {
    return;
  }

  if (updated_view != GetFocusedViewForTextSelection()) {
    return;
  }

  const content::TextInputManager::TextSelection* selection =
      GetTextInputManager()->GetTextSelection(updated_view);
  container_->TextSelectionChanged(this, updated_view, selection);
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
  switch (event) {
    case ui::SELECTION_HANDLE_DRAG_STARTED:
    case ui::INSERTION_HANDLE_DRAG_STARTED:
      selection_handle_drag_in_progress_ = true;
      break;
    case ui::SELECTION_HANDLE_DRAG_STOPPED:
    case ui::INSERTION_HANDLE_DRAG_STOPPED:
      selection_handle_drag_in_progress_ = false;
      break;
    case ui::INSERTION_HANDLE_TAPPED:
      if (container_) {
        container_->TouchInsertionHandleTapped(this);
      }
      return;
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

  return container_->CreateTouchHandleDrawable();
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
  if (local_surface_id_.is_valid()) {
    DCHECK(surface_factory_.get());
    local_surface_id_ = cc::LocalSurfaceId();
    surface_factory_->EvictSurface();
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

void RenderWidgetHostView::SurfaceDrawn(const cc::LocalSurfaceId& id,
                                        cc::CompositorFrameMetadata metadata) {
  if (id == local_surface_id_) {
    last_drawn_frame_metadata_ = std::move(metadata);
  }

  RunAckCallbacks();
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
      is_loading_(false),
      is_showing_(!host->is_hidden()),
      browser_controls_shrink_blink_size_(false),
      gesture_provider_(GestureProvider::Create(this)),
      selection_handle_drag_in_progress_(false),
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
  DCHECK(!local_surface_id_.is_valid());

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

blink::WebContextMenuData::EditFlags RenderWidgetHostView::GetEditFlags() {
  blink::WebContextMenuData::EditFlags flags =
      blink::WebContextMenuData::CanDoNone;

  content::RenderWidgetHostViewBase* view = GetFocusedViewForTextSelection();
  if (!view) {
    return flags;
  }

  const content::TextInputState* state =
      GetTextInputManager()->GetTextInputState();
  ui::TextInputType text_input_type =
      state ? state->type : ui::TEXT_INPUT_TYPE_NONE;

  bool editable = (text_input_type != ui::TEXT_INPUT_TYPE_NONE);
  bool readable = (text_input_type != ui::TEXT_INPUT_TYPE_PASSWORD);

  const content::TextInputManager::TextSelection* selection =
      GetTextInputManager()->GetTextSelection(state ? nullptr : view);
  bool has_selection = selection && !selection->range.is_empty();

  // XXX: if editable, can we determine whether undo/redo is available?
  if (editable && readable && has_selection) {
    flags |= blink::WebContextMenuData::CanCut;
  }
  if (readable && has_selection) {
    flags |= blink::WebContextMenuData::CanCopy;
  }
  if (editable &&
      Clipboard::GetForCurrentThread()->HasData(ui::CLIPBOARD_TYPE_COPY_PASTE)) {
    flags |= blink::WebContextMenuData::CanPaste;
  }
  if (editable && has_selection) {
    flags |= blink::WebContextMenuData::CanDelete;
  }
  flags |= blink::WebContextMenuData::CanSelectAll;

  return flags;
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

  content::RenderWidgetHostInputEventRouter* router =
      host_->delegate()->GetInputEventRouter();
  blink::WebTouchEvent web_touch_event =
      MakeWebTouchEvent(event, rv.moved_beyond_slop_region);
  router->RouteTouchEvent(this, &web_touch_event, ui::LatencyInfo());
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

void RenderWidgetHostView::Blur() {
  host_->SetActive(false);
  host_->Blur();

  selection_controller_->HideAndDisallowShowingAutomatically();
}

content::RenderWidgetHostViewBase*
RenderWidgetHostView::GetFocusedViewForTextSelection() const {
  content::RenderWidgetHostImpl* host = GetFocusedWidget();
  if (!host) {
    return nullptr;
  }

  return host->GetView();
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
  host_->GotFocus();
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
