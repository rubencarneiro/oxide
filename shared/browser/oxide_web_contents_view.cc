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

#include "oxide_web_contents_view.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "cc/layers/layer_settings.h"
#include "cc/layers/solid_color_layer.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/web/WebDragOperation.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_data.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/input/oxide_input_method_context.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/common/oxide_unowned_user_data.h"

#include "oxide_browser_platform_integration.h"
#include "oxide_drag_source.h"
#include "oxide_fullscreen_helper.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_screen_client.h"
#include "oxide_web_contents_view_client.h"
#include "oxide_web_context_menu.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
int kUserDataKey;
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

WebContentsView::WebContentsView(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      client_(nullptr),
      compositor_(Compositor::Create(this)),
      root_layer_(cc::SolidColorLayer::Create(cc::LayerSettings())),
      current_drag_allowed_ops_(blink::WebDragOperationNone),
      current_drag_op_(blink::WebDragOperationNone) {
  web_contents->SetUserData(&kUserDataKey,
                            new UnownedUserData<WebContentsView>(this));

  root_layer_->SetIsDrawable(true);
  root_layer_->SetBackgroundColor(SK_ColorWHITE);

  compositor_->SetVisibility(false);
  compositor_->SetRootLayer(root_layer_);

  CompositorObserver::Observe(compositor_.get());
}

content::WebContentsImpl* WebContentsView::web_contents_impl() const {
  return static_cast<content::WebContentsImpl*>(web_contents());
}

ui::TouchSelectionController*
WebContentsView::GetTouchSelectionController() const {
  content::RenderWidgetHostView* view = GetRenderWidgetHostView();
  if (!view) {
    return nullptr;
  }

  return static_cast<RenderWidgetHostView*>(view)->selection_controller();
}

RenderWidgetHostView* WebContentsView::GetRenderWidgetHostView() const {
  content::RenderWidgetHostView* rwhv =
      web_contents()->GetFullscreenRenderWidgetHostView();
  if (!rwhv) {
    rwhv = web_contents()->GetRenderWidgetHostView();
  }

  return static_cast<RenderWidgetHostView *>(rwhv);
}

content::RenderWidgetHost* WebContentsView::GetRenderWidgetHost() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return nullptr;
  }

  return rwhv->GetRenderWidgetHost();
}

bool WebContentsView::ShouldScrollFocusedEditableNodeIntoView() {
  if (!HasFocus()) {
    return false;
  }

  if (!IsVisible()) {
    return false;
  }

  if (!client_->GetInputMethodContext() ||
      !client_->GetInputMethodContext()->IsInputPanelVisible()) {
    return false;
  }

  if (!GetRenderWidgetHostView() ||
      !GetRenderWidgetHostView()->ime_bridge()->focused_node_is_editable()) {
    return false;
  }

  return true;
}

void WebContentsView::MaybeScrollFocusedEditableNodeIntoView() {
  if (!ShouldScrollFocusedEditableNodeIntoView()) {
    return;
  }

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  content::RenderWidgetHostImpl::From(host)
      ->ScrollFocusedEditableNodeIntoRect(GetBounds());
}

gfx::RectF WebContentsView::GetBoundsF() const {
  if (!client_) {
    return gfx::RectF();
  }

  // If we're in fullscreen mode, return the screen size rather than the
  // view bounds. This works around an issue where buggy Flash content
  // expects the view to resize synchronously when it goes fullscreen, but it
  // happens asynchronously instead.
  // See https://launchpad.net/bugs/1510508
  // XXX: Obviously, this means we assume that we do occupy the full screen
  //  when the browser grants us fullscreen. If that's not the case, then
  //  this is going to break
  FullscreenHelper* fullscreen =
      FullscreenHelper::FromWebContents(web_contents());
  if (fullscreen &&
      fullscreen->IsFullscreen() &&
      web_contents()->GetFullscreenRenderWidgetHostView()) {
    return gfx::RectF(GetScreenInfo().rect);
  }

  return client_->GetBounds();
}

gfx::NativeView WebContentsView::GetNativeView() const {
  return nullptr;
}

gfx::NativeView WebContentsView::GetContentNativeView() const {
  return nullptr;
}

gfx::NativeWindow WebContentsView::GetTopLevelNativeWindow() const {
  return nullptr;
}

void WebContentsView::GetContainerBounds(gfx::Rect* out) const {
  *out = GetBounds();
}

void WebContentsView::SizeContents(const gfx::Size& size) {
  content::RenderWidgetHostView* rwhv =
      web_contents()->GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(size);
  }
}

void WebContentsView::Focus() {
  content::RenderWidgetHostView* rwhv =
      web_contents()->GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->Focus();
}

void WebContentsView::SetInitialFocus() {
  NOTIMPLEMENTED();
}

void WebContentsView::StoreFocus() {}

void WebContentsView::RestoreFocus() {}

content::DropData* WebContentsView::GetDropData() const {
  return current_drop_data_.get();
}

gfx::Rect WebContentsView::GetViewBounds() const {
  return GetBounds();
}

void WebContentsView::CreateView(const gfx::Size& initial_size,
                                 gfx::NativeView context) {}

content::RenderWidgetHostViewBase* WebContentsView::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host,
    bool is_guest_view_hack) {
  return new RenderWidgetHostView(
      content::RenderWidgetHostImpl::From(render_widget_host));
}

content::RenderWidgetHostViewBase* WebContentsView::CreateViewForPopupWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostView(
      content::RenderWidgetHostImpl::From(render_widget_host));
}

void WebContentsView::SetPageTitle(const base::string16& title) {}

void WebContentsView::RenderViewCreated(content::RenderViewHost* host) {}

void WebContentsView::RenderViewSwappedIn(content::RenderViewHost* host) {}

void WebContentsView::SetOverscrollControllerEnabled(bool enabled) {}

void WebContentsView::ShowContextMenu(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params) {
  if (!client_) {
    return;
  }

  WebContextMenu* menu = client_->CreateContextMenu(render_frame_host, params);
  if (!menu) {
    return;
  }

  menu->Show();
}

void WebContentsView::StartDragging(
    const content::DropData& drop_data,
    blink::WebDragOperationsMask allowed_ops,
    const gfx::ImageSkia& image,
    const gfx::Vector2d& image_offset,
    const content::DragEventSourceInfo& event_info) {
  if (drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag when one is already in progress";
    web_contents()->SystemDragEnded();
    return;
  }

  drag_source_ =
      BrowserPlatformIntegration::GetInstance()->CreateDragSource(this);
  if (!drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag - not supported";
    web_contents()->SystemDragEnded();
    return;
  }

  HideTouchSelectionController();

  // As our implementation of gfx::Screen::GetDisplayNearestWindow always
  // returns an invalid display, the passed in image isn't quite correct.
  // Recreate it with the correct scale and dimenstions
  gfx::ImageSkia new_image =
      gfx::ImageSkia(gfx::ImageSkiaRep(image.GetRepresentation(1).sk_bitmap(),
                                       GetScreenInfo().deviceScaleFactor));

  drag_source_->StartDragging(web_contents(),
                              drop_data,
                              allowed_ops,
                              new_image,
                              image_offset);
}

void WebContentsView::UpdateDragCursor(blink::WebDragOperation operation) {
  current_drag_op_ = operation;
}

void WebContentsView::ShowPopupMenu(
    content::RenderFrameHost* render_frame_host,
    const gfx::Rect& bounds,
    int item_height,
    double item_font_size,
    int selected_item,
    const std::vector<content::MenuItem>& items,
    bool right_aligned,
    bool allow_multiple_selection) {
  // XXX: We should do better than this
  DCHECK(!active_popup_menu_);

  if (!client_) {
    static_cast<content::RenderFrameHostImpl *>(
        render_frame_host)->DidCancelPopupMenu();
    return;
  }

  WebPopupMenu* menu = client_->CreatePopupMenu(render_frame_host);
  if (!menu) {
    static_cast<content::RenderFrameHostImpl *>(
        render_frame_host)->DidCancelPopupMenu();
    return;
  }

  active_popup_menu_ = menu->GetWeakPtr();

  menu->Show(bounds, items, selected_item, allow_multiple_selection);
}

void WebContentsView::HidePopupMenu() {
  if (!active_popup_menu_) {
    return;
  }

  active_popup_menu_->Close();
}

void WebContentsView::RenderViewHostChanged(
    content::RenderViewHost* old_host,
    content::RenderViewHost* new_host) {
  if (old_host && old_host->GetWidget()->GetView()) {
    RenderWidgetHostView* rwhv =
        static_cast<RenderWidgetHostView*>(old_host->GetWidget()->GetView());
    rwhv->SetContainer(nullptr);
    rwhv->ime_bridge()->SetContext(nullptr);
  }

  if (!new_host) {
    return;
  }

  if (new_host->GetWidget()->GetView()) {
    RenderWidgetHostView* rwhv =
        static_cast<RenderWidgetHostView*>(new_host->GetWidget()->GetView());
    rwhv->SetContainer(this);
    rwhv->ime_bridge()->SetContext(client_->GetInputMethodContext());

    // For the initial view, we need to sync its visibility and focus state
    // with us. For subsequent views, RFHM does this for us
    if (!old_host) {
      if (IsVisible()) {
        rwhv->Show();
      } else {
        rwhv->Hide();
      }

      if (HasFocus()) {
        rwhv->Focus();
      } else {
        rwhv->Blur();
      }
    }
  }

  EditingCapabilitiesChanged();
  TouchSelectionChanged(false);
}

void WebContentsView::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (details.is_navigation_to_different_page()) {
    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->ResetGestureDetection();
    }

    mouse_state_.Reset();
  }
}

void WebContentsView::DidShowFullscreenWidget(int routing_id) {
  content::RenderWidgetHost* rwh =
      content::RenderWidgetHost::FromID(
          web_contents()->GetRenderProcessHost()->GetID(),
          routing_id);
  DCHECK(rwh);

  static_cast<RenderWidgetHostView*>(rwh->GetView())->SetContainer(this);

  web_contents()->GetRenderWidgetHostView()->Hide();

  TouchSelectionChanged(false);
}

void WebContentsView::DidDestroyFullscreenWidget(int routing_id) {
  DCHECK(!web_contents()->GetFullscreenRenderWidgetHostView());

  RenderWidgetHostView* orig_rwhv =
      static_cast<RenderWidgetHostView*>(
        web_contents()->GetRenderWidgetHostView());
  if (!orig_rwhv) {
    return;
  }

  content::RenderWidgetHost* orig_rwh = orig_rwhv->GetRenderWidgetHost();
  orig_rwh->WasResized();
  content::RenderWidgetHostImpl::From(orig_rwh)->SendScreenRects();

  if (IsVisible()) {
    orig_rwhv->Show();
  }

  if (HasFocus()) {
    orig_rwhv->Focus();
  } else {
    orig_rwhv->Blur();
  }

  TouchSelectionChanged(false);
}

void WebContentsView::DidAttachInterstitialPage() {
  DCHECK(!interstitial_rwh_id_.IsValid());
  DCHECK(web_contents()->GetInterstitialPage());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  rwhv->SetContainer(this);
  interstitial_rwh_id_ = rwhv->GetRenderWidgetHost();

  TouchSelectionChanged(false);
}

void WebContentsView::DidDetachInterstitialPage() {
  if (!interstitial_rwh_id_.IsValid()) {
    return;
  }

  content::RenderWidgetHost* rwh = interstitial_rwh_id_.ToInstance();
  interstitial_rwh_id_ = RenderWidgetHostID();
  if (!rwh) {
    return;
  }

  static_cast<RenderWidgetHostView*>(rwh->GetView())->SetContainer(nullptr);

  TouchSelectionChanged(false);
}

void WebContentsView::CompositorSwapFrame(CompositorFrameHandle* handle) {
  received_surface_ids_.push(handle->data()->surface_id);

  if (current_compositor_frame_.get()) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = handle;

  if (client_) {
    client_->SwapCompositorFrame();
  } else {
    DidCommitCompositorFrame();
  }
}

void WebContentsView::CompositorDidCommit() {
  // XXX: Not sure if the behaviour here when there's no view is correct
  committed_frame_metadata_ =
      GetRenderWidgetHostView() ?
        GetRenderWidgetHostView()->last_submitted_frame_metadata()
        : cc::CompositorFrameMetadata();
}

void WebContentsView::EndDrag(blink::WebDragOperation operation) {
  DCHECK(drag_source_);

  gfx::Point screen_point =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  gfx::Point view_point =
      screen_point - gfx::Vector2d(GetBounds().origin().x(),
                                   GetBounds().origin().y());
  web_contents_impl()->DragSourceEndedAt(view_point.x(), view_point.y(),
                                         screen_point.x(), screen_point.y(),
                                         operation);

  web_contents()->SystemDragEnded();

  drag_source_.reset();
}

void WebContentsView::InputPanelVisibilityChanged() {
  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::AttachLayer(scoped_refptr<cc::Layer> layer) {
  DCHECK(layer.get());
  root_layer_->InsertChild(layer, 0);
  root_layer_->SetIsDrawable(false);
}

void WebContentsView::DetachLayer(scoped_refptr<cc::Layer> layer) {
  DCHECK(layer.get());
  DCHECK_EQ(layer->parent(), root_layer_.get());
  layer->RemoveFromParent();
  if (root_layer_->children().size() == 0) {
    root_layer_->SetIsDrawable(true);
  }
}

void WebContentsView::CursorChanged() {
  if (!client_) {
    return;
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  client_->UpdateCursor(rwhv->current_cursor());
}

gfx::Size WebContentsView::GetViewSizeInPixels() const {
  return GetSizeInPixels();
}

bool WebContentsView::HasFocus(const RenderWidgetHostView* view) const {
  if (!HasFocus()) {
    return false;
  }

  return view == GetRenderWidgetHostView();
}

bool WebContentsView::IsFullscreen() const {
  return FullscreenHelper::FromWebContents(web_contents())->IsFullscreen();
}

float WebContentsView::GetLocationBarHeight() const {
  // TODO: Add LocationBarController class
  WebView* view = WebView::FromWebContents(web_contents());
  if (!view) {
    return 0.f;
  }

  return view->GetLocationBarHeight();
}

ui::TouchHandleDrawable* WebContentsView::CreateTouchHandleDrawable() const {
  if (!client_) {
    return nullptr;
  }

  return client_->CreateTouchHandleDrawable();
}

void WebContentsView::TouchSelectionChanged(
    bool handle_drag_in_progress) const {
  if (!client_) {
    return;
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  ui::TouchSelectionController* controller = rwhv->selection_controller();
  bool active =
      (controller->active_status() != ui::TouchSelectionController::INACTIVE);

  gfx::RectF bounds = controller->GetRectBetweenBounds();

  // TODO: Add LocationBarController class
  float offset = 0.f;
  WebView* view = WebView::FromWebContents(web_contents());
  if (view) {
    offset = view->GetLocationBarContentOffset();
  }
  bounds.Offset(0, offset);

  client_->TouchSelectionChanged(active, bounds, handle_drag_in_progress);
}

void WebContentsView::EditingCapabilitiesChanged() {
  if (!editing_capabilities_changed_callback_.is_null()) {
    editing_capabilities_changed_callback_.Run();
  }
}

WebContentsView::~WebContentsView() {
  if (client_) {
    DCHECK_EQ(client_->view_, this);
    client_->view_ = nullptr;
  }
  if (web_contents()) {
    web_contents()->RemoveUserData(&kUserDataKey);
  }
}

// static
content::WebContentsViewOxide* WebContentsView::Create(
    content::WebContents* web_contents) {
  return new WebContentsView(web_contents);
}

// static
WebContentsView* WebContentsView::FromWebContents(
    content::WebContents* contents) {
  UnownedUserData<WebContentsView>* data =
      static_cast<UnownedUserData<WebContentsView>*>(
        contents->GetUserData(&kUserDataKey));
  if (!data) {
    return nullptr;
  }

  return data->get();
}

content::WebContents* WebContentsView::GetWebContents() const {
  return web_contents();
}

void WebContentsView::SetClient(WebContentsViewClient* client) {
  if (client_) {
    DCHECK_EQ(client_->view_, this);
    InputMethodContextObserver::Observe(nullptr);
    client_->view_ = nullptr;
  }
  client_ = client;
  if (client_) {
    DCHECK(!client_->view_);
    client_->view_ = this;
    InputMethodContextObserver::Observe(client_->GetInputMethodContext());
  }

  // Update view from client
  WasResized();
  VisibilityChanged();
  FocusChanged();
  ScreenUpdated();

  // Update client from view
  CursorChanged();
  TouchSelectionChanged(false);
}

bool WebContentsView::IsVisible() const {
  if (!client_) {
    return false;
  }

  return client_->IsVisible();
}

bool WebContentsView::HasFocus() const {
  if (!client_) {
    return false;
  }

  return client_->HasFocus();
}

gfx::Size WebContentsView::GetSizeInPixels() const {
  return gfx::ToRoundedSize(
      gfx::ScaleSize(GetBoundsF().size(), GetScreenInfo().deviceScaleFactor));
}

gfx::Rect WebContentsView::GetBounds() const {
  return gfx::ToEnclosingRect(GetBoundsF());
}

blink::WebScreenInfo WebContentsView::GetScreenInfo() const {
  if (!client_) {
    blink::WebScreenInfo result;
    content::RenderWidgetHostViewBase::GetDefaultScreenInfo(&result);
    return result;
  }

  return client_->GetScreenInfo();
}

void WebContentsView::HandleKeyEvent(
    const content::NativeWebKeyboardEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardKeyboardEvent(event);
}

void WebContentsView::HandleMouseEvent(const blink::WebMouseEvent& event) {
  blink::WebMouseEvent e(event);

  mouse_state_.UpdateEvent(&e);

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardMouseEvent(e);
}

void WebContentsView::HandleTouchEvent(const ui::TouchEvent& event) {
  if (!touch_state_.Update(event)) {
    return;
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->HandleTouchEvent(touch_state_);
}

void WebContentsView::HandleWheelEvent(
    const blink::WebMouseWheelEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardWheelEvent(event);
}

void WebContentsView::HandleDragEnter(
    const content::DropData& drop_data,
    const gfx::Point& location,
    blink::WebDragOperationsMask allowed_ops,
    int key_modifiers) {
  current_drop_data_.reset(new content::DropData(drop_data));
  current_drag_allowed_ops_ = allowed_ops;

  content::RenderViewHost* rvh = web_contents()->GetRenderViewHost();
  current_drag_target_ = RenderWidgetHostID(rvh->GetWidget());

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDragEnter(*current_drop_data_,
                           location,
                           screen_location,
                           current_drag_allowed_ops_,
                           key_modifiers);
}

blink::WebDragOperation WebContentsView::HandleDragMove(
    const gfx::Point& location,
    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  content::RenderViewHost* rvh = web_contents()->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    location,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDragOver(location,
                          screen_location,
                          current_drag_allowed_ops_,
                          key_modifiers);

  return current_drag_op_;
}

void WebContentsView::HandleDragLeave() {
  if (!current_drop_data_) {
    return;
  }

  current_drop_data_.reset();

  content::RenderViewHost* rvh = web_contents()->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    return;
  }

  rvh->DragTargetDragLeave();
}

blink::WebDragOperation WebContentsView::HandleDrop(const gfx::Point& location,
                                                    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  content::RenderViewHost* rvh = web_contents()->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    location,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDrop(location, screen_location, key_modifiers);

  return current_drag_op_;
}

Compositor* WebContentsView::GetCompositor() const {
  return compositor_.get();
}

CompositorFrameHandle* WebContentsView::GetCompositorFrameHandle() const {
  return current_compositor_frame_.get();
}

void WebContentsView::DidCommitCompositorFrame() {
  DCHECK(!received_surface_ids_.empty());

  while (!received_surface_ids_.empty()) {
    uint32_t surface_id = received_surface_ids_.front();
    received_surface_ids_.pop();

    compositor_->DidSwapCompositorFrame(
        surface_id,
        std::move(previous_compositor_frames_));
  }
}

void WebContentsView::WasResized() {
  compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);
  compositor_->SetViewportSize(GetSizeInPixels());
  root_layer_->SetBounds(GetBounds().size());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(GetBounds().size());
    content::RenderWidgetHostImpl::From(GetRenderWidgetHost())
        ->SendScreenRects();
    GetRenderWidgetHost()->WasResized();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

  if (!visible) {
    // TODO: Have an eviction algorithm for LayerTreeHosts in Compositor, and
    //  trigger eviction of the frontbuffer from a CompositorClient callback.
    // XXX: Also this isn't really necessary for eviction - after all, the LTH
    //  owned by Compositor owns the frontbuffer (via its cc::OutputSurface).
    //  This callback is really to notify the toolkit layer that the
    //  frontbuffer is being dropped
    current_compositor_frame_ = nullptr;
    if (client_) {
      client_->EvictCurrentFrame();
    }
  }

  if (visible) {
    web_contents()->WasShown();
  } else {
    web_contents()->WasHidden();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::FocusChanged() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  if (HasFocus()) {
    rwhv->Focus();
  } else {
    rwhv->Blur();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::ScreenUpdated() {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  content::RenderWidgetHostImpl::From(host)->NotifyScreenInfoChanged();
}

void WebContentsView::HideTouchSelectionController() {
  ui::TouchSelectionController* selection_controller =
      GetTouchSelectionController();
  if (selection_controller) {
    selection_controller->HideAndDisallowShowingAutomatically();
  }
}

} // namespace oxide
