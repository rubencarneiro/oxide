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
#include "cc/layers/solid_color_layer.h"
#include "content/browser/renderer_host/render_widget_host_impl.h" // nogncheck
#include "content/browser/web_contents/web_contents_impl.h" // nogncheck
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/WebKit/public/platform/WebDragOperation.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
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
#include "shared/browser/context_menu/web_context_menu_impl.h"
#include "shared/browser/input/oxide_input_method_context.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_unowned_user_data.h"

#include "chrome_controller.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_drag_source.h"
#include "oxide_fullscreen_helper.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view_client.h"
#include "oxide_web_popup_menu_impl.h"
#include "screen.h"

namespace oxide {

namespace {
int kUserDataKey;

ChromeController* kUninitializedChromeController =
    reinterpret_cast<ChromeController*>(1);
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

WebContentsView::WebContentsView(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      client_(nullptr),
      compositor_(Compositor::Create(this)),
      root_layer_(cc::SolidColorLayer::Create()),
      current_drag_allowed_ops_(blink::WebDragOperationNone),
      current_drag_op_(blink::WebDragOperationNone),
      chrome_controller_(kUninitializedChromeController) {
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

  if (ViewSizeShouldBeScreenSize()) {
    return gfx::RectF(GetDisplay().bounds());
  }

  return client_->GetBounds();
}

bool WebContentsView::ViewSizeShouldBeScreenSize() const {
  // If we're in fullscreen mode, we force the view bounds to be based on the
  // screen size. This works around an issue where buggy Flash content
  // expects the view to resize synchronously when it goes fullscreen, but it
  // happens asynchronously instead.
  // See https://launchpad.net/bugs/1510508
  // XXX: Obviously, this means we assume that we do occupy the full screen
  //  when the browser grants us fullscreen. If that's not the case, then
  //  this is going to break
  FullscreenHelper* fullscreen =
      FullscreenHelper::FromWebContents(web_contents());
  if (!fullscreen) {
    return false;
  }

  if (!fullscreen->IsFullscreen()) {
    // We're not in fullscreen
    return false;
  }

  if (!web_contents()->GetFullscreenRenderWidgetHostView()) {
    // Only do this for fullscreen widgets. We don't do this for the HTML5
    // fullscreen API
    return false;
  }

  return true;
}

void WebContentsView::ResizeCompositorViewport() {
  compositor_->SetDeviceScaleFactor(GetDisplay().device_scale_factor());
  compositor_->SetViewportSize(GetSizeInPixels());
}

void WebContentsView::UpdateContentsSize() {
  gfx::Size size = GetSize();

  root_layer_->SetBounds(size);

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(size);
  }
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

void WebContentsView::GetScreenInfo(content::ScreenInfo* screen_info) const {
  content::WebContentsViewOxide::GetScreenInfoForDisplay(GetDisplay(),
                                                         screen_info);
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
  RenderWidgetHostView* view =
      new RenderWidgetHostView(
          content::RenderWidgetHostImpl::From(render_widget_host));
  if (web_contents()->GetRenderViewHost() &&
      web_contents()->GetRenderViewHost()->GetWidget() == render_widget_host) {
    view->SetContainer(this);
    if (client_) {
      view->ime_bridge()->SetContext(client_->GetInputMethodContext());
    }
  }

  return view;
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
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv && rwhv->HandleContextMenu(params)) {
    if (client_) {
      client_->ContextMenuIntercepted();
    }
    return;
  }

  WebContextMenuImpl* menu = new WebContextMenuImpl(render_frame_host, params);
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
                                       GetDisplay().device_scale_factor()));

  drag_source_->StartDragging(web_contents(),
                              drop_data,
                              allowed_ops,
                              new_image,
                              image_offset);
}

void WebContentsView::UpdateDragCursor(blink::WebDragOperation operation) {
  current_drag_op_ = operation;
}

void WebContentsView::ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                                    const gfx::Rect& bounds,
                                    int item_height,
                                    double item_font_size,
                                    int selected_item,
                                    const std::vector<content::MenuItem>& items,
                                    bool right_aligned,
                                    bool allow_multiple_selection) {
  // XXX: We should do better than this
  DCHECK(!active_popup_menu_);

  WebPopupMenuImpl* menu = new WebPopupMenuImpl(render_frame_host,
                                                items,
                                                selected_item,
                                                allow_multiple_selection);
  active_popup_menu_ = menu->GetWeakPtr();

  menu->Show(bounds);
}

void WebContentsView::HidePopupMenu() {
  if (!active_popup_menu_) {
    return;
  }

  active_popup_menu_->Hide();
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

  EditingCapabilitiesChanged(GetRenderWidgetHostView());
  TouchSelectionChanged(GetRenderWidgetHostView(), false, false);
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

void WebContentsView::DidShowFullscreenWidget() {
  DCHECK(!web_contents()->ShowingInterstitialPage());
  DCHECK(web_contents()->GetFullscreenRenderWidgetHostView());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();

  rwhv->SetContainer(this);

  // The visibility of RWH is set on creation, but it's possible that it could
  // have changed by now
  if (IsVisible()) {
    rwhv->Show();
  } else {
    rwhv->Hide();
  }

  // The WasResized / SendScreenRects / Focus dance happens in content, so
  // we don't need to do anything else with |rwhv|

  // Hide the original RWHV
  web_contents()->GetRenderWidgetHostView()->Hide();

  EditingCapabilitiesChanged(rwhv);
  TouchSelectionChanged(rwhv, false, false);
}

void WebContentsView::DidDestroyFullscreenWidget() {
  RenderWidgetHostView* orig_rwhv =
      static_cast<RenderWidgetHostView*>(
        web_contents()->GetRenderWidgetHostView());
  if (orig_rwhv) {
    // Update the size, as it might have changed
    orig_rwhv->SetSize(GetSize());

    if (IsVisible()) {
      // Unhide the original RWHV again. This also sends a resize message
      orig_rwhv->Show();
    }

    // Update the focus, as it might have changed
    if (HasFocus()) {
      orig_rwhv->Focus();
    } else {
      orig_rwhv->Blur();
    }
  }

  // FIXME: This actually doesn't work, because GetRenderWidgetHostView still
  // returns the fullscreen view
  EditingCapabilitiesChanged(orig_rwhv);
  TouchSelectionChanged(orig_rwhv, false, false);
}

void WebContentsView::DidAttachInterstitialPage() {
  DCHECK(!interstitial_rwh_id_.IsValid());
  DCHECK(web_contents()->GetInterstitialPage());

  if (web_contents()->GetFullscreenRenderWidgetHostView()) {
    // Cancel fullscreen if there is a fullscreen view, to avoid getting in
    // a weird state when we swap between views
    web_contents()->ExitFullscreen(false);
    DCHECK(!web_contents()->GetFullscreenRenderWidgetHostView());
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  rwhv->SetContainer(this);

  // The size could have changed between the points at which the interstitial
  // RWHV was created and now
  rwhv->SetSize(GetSize());

  // Save the ID, as it's detached by the time DidDetachInterstitialPage is
  // called
  interstitial_rwh_id_ = rwhv->GetRenderWidgetHost();

  // Content takes care of adjusting visibility and sending focus / resize
  // messages, so there's nothing else to do with |rwhv|

  EditingCapabilitiesChanged(rwhv),
  TouchSelectionChanged(rwhv, false, false);
}

void WebContentsView::DidDetachInterstitialPage() {
  DCHECK(interstitial_rwh_id_.IsValid());

  content::RenderWidgetHost* interstitial_rwh =
      interstitial_rwh_id_.ToInstance();
  interstitial_rwh_id_ = RenderWidgetHostID();
  if (interstitial_rwh) {
    static_cast<RenderWidgetHostView*>(
        interstitial_rwh->GetView())->SetContainer(nullptr);
  }

  RenderWidgetHostView* orig_rwhv = GetRenderWidgetHostView();
  if (orig_rwhv) {
    DCHECK(!interstitial_rwh || interstitial_rwh->GetView() != orig_rwhv);   
    orig_rwhv->SetSize(GetSize());
  }

  EditingCapabilitiesChanged(orig_rwhv);
  TouchSelectionChanged(orig_rwhv, false, false);
}

void WebContentsView::CompositorSwapFrame(CompositorFrameHandle* handle,
                                          const SwapAckCallback& callback) {
  compositor_ack_callbacks_.push(callback);

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
        GetRenderWidgetHostView()->last_submitted_frame_metadata().Clone()
        : cc::CompositorFrameMetadata();
}

void WebContentsView::CompositorEvictResources() {
  current_compositor_frame_ = nullptr;
  previous_compositor_frames_.clear();
 
  if (!client_) {
    return;
  }

  client_->EvictCurrentFrame();
}

void WebContentsView::EndDrag(blink::WebDragOperation operation) {
  DCHECK(drag_source_);

  gfx::Point screen_point =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreen()
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

void WebContentsView::CursorChanged(RenderWidgetHostView* view) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  client_->UpdateCursor(view ? view->current_cursor() : content::WebCursor());
}

gfx::Size WebContentsView::GetViewSizeInPixels() const {
  return GetSizeInPixels();
}

bool WebContentsView::IsFullscreen() const {
  FullscreenHelper* helper = FullscreenHelper::FromWebContents(web_contents());
  if (!helper) {
    return false;
  }

  return helper->IsFullscreen();
}

float WebContentsView::GetTopControlsHeight() {
  ChromeController* chrome_controller = GetChromeController();
  if (!chrome_controller) {
    return 0.f;
  }

  return chrome_controller->top_controls_height();
}

ui::TouchHandleDrawable* WebContentsView::CreateTouchHandleDrawable() const {
  if (!client_) {
    return nullptr;
  }

  return client_->CreateTouchHandleDrawable();
}

void WebContentsView::TouchSelectionChanged(RenderWidgetHostView* view,
                                            bool handle_drag_in_progress,
                                            bool insertion_handle_tapped) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  ui::TouchSelectionController* controller =
      view ? view->selection_controller() : nullptr;
  ui::TouchSelectionController::ActiveStatus status =
      controller ? controller->active_status()
                 : ui::TouchSelectionController::INACTIVE;

  gfx::RectF bounds;
  if (controller) {
    bounds = controller->GetRectBetweenBounds();
  }

  float offset = 0.f;
  ChromeController* chrome_controller = GetChromeController();
  if (chrome_controller) {
    offset = chrome_controller->GetTopContentOffset();
  }
  bounds.Offset(0, offset);

  client_->TouchSelectionChanged(status,
                                 bounds,
                                 handle_drag_in_progress,
                                 insertion_handle_tapped);
}

void WebContentsView::EditingCapabilitiesChanged(RenderWidgetHostView* view) {
  if (view != GetRenderWidgetHostView()) {
    return;
  }

  if (!editing_capabilities_changed_callback_.is_null()) {
    editing_capabilities_changed_callback_.Run();
  }
}

void WebContentsView::OnDisplayPropertiesChanged(
    const display::Display& display) {
  if (display.id() != GetDisplay().id()) {
    return;
  }

  ScreenChanged();
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

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->ime_bridge()->SetContext(
        client_ ? client_->GetInputMethodContext() : nullptr);
  }

  // Update view from client
  WasResized();
  VisibilityChanged();
  FocusChanged();
  ScreenChanged();

  // Update client from view
  CursorChanged(GetRenderWidgetHostView());
  TouchSelectionChanged(GetRenderWidgetHostView(), false, false);
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

gfx::Size WebContentsView::GetSize() const {
  return GetBounds().size();
}

gfx::Size WebContentsView::GetSizeInPixels() const {
  return gfx::ToRoundedSize(
      gfx::ScaleSize(GetBoundsF().size(), GetDisplay().device_scale_factor()));
}

gfx::Rect WebContentsView::GetBounds() const {
  return gfx::ToEnclosingRect(GetBoundsF());
}

display::Display WebContentsView::GetDisplay() const {
  if (client_) {
    display::Display display = client_->GetDisplay();
    if (display.is_valid()) {
      return display;
    }
  }

  return Screen::GetInstance()->GetPrimaryDisplay();
}

void WebContentsView::HandleKeyEvent(
    const content::NativeWebKeyboardEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  GetRenderWidgetHostView()->OnUserInput();

  host->ForwardKeyboardEvent(event);
}

void WebContentsView::HandleMouseEvent(const blink::WebMouseEvent& event) {
  blink::WebMouseEvent e(event);

  mouse_state_.UpdateEvent(&e);

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  GetRenderWidgetHostView()->OnUserInput();

  host->ForwardMouseEvent(e);
}

void WebContentsView::HandleMotionEvent(const ui::MotionEvent& event) {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->HandleTouchEvent(event);
}

void WebContentsView::HandleWheelEvent(
    const blink::WebMouseWheelEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  GetRenderWidgetHostView()->OnUserInput();

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
          ->GetScreen()
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
        ->GetScreen()
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
        ->GetScreen()
        ->GetCursorScreenPoint();
  rvh->DragTargetDrop(*current_drop_data_,
                      location,
                      screen_location,
                      key_modifiers);

  return current_drag_op_;
}

Compositor* WebContentsView::GetCompositor() const {
  return compositor_.get();
}

CompositorFrameHandle* WebContentsView::GetCompositorFrameHandle() const {
  return current_compositor_frame_.get();
}

void WebContentsView::DidCommitCompositorFrame() {
  DCHECK(!compositor_ack_callbacks_.empty());

  while (!compositor_ack_callbacks_.empty()) {
    compositor_ack_callbacks_.front().Run(
        std::move(previous_compositor_frames_));
    compositor_ack_callbacks_.pop();
  }
}

void WebContentsView::WasResized() {
  ResizeCompositorViewport();
  UpdateContentsSize();

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (rwh) {
    content::RenderWidgetHostImpl::From(rwh)->SendScreenRects();
    rwh->WasResized();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

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

void WebContentsView::ScreenChanged() {
  // If the device scale has changed, then the compositor viewport size
  // and scale needs updating
  // See https://launchpad.net/bugs/1575216
  ResizeCompositorViewport();
 
  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
 
  if (ViewSizeShouldBeScreenSize()) {
    // See https://launchpad.net/bugs/1558792
    UpdateContentsSize();
    if (rwh) {
      content::RenderWidgetHostImpl::From(rwh)->SendScreenRects();
    }
  }

  if (!rwh) {
    return;
  }

  content::RenderWidgetHostImpl::From(rwh)->NotifyScreenInfoChanged();
}

void WebContentsView::HideTouchSelectionController() {
  ui::TouchSelectionController* selection_controller =
      GetTouchSelectionController();
  if (selection_controller) {
    selection_controller->HideAndDisallowShowingAutomatically();
  }
}

ChromeController* WebContentsView::GetChromeController() {
  if (chrome_controller_ == kUninitializedChromeController) {
    chrome_controller_ = ChromeController::FromWebContents(web_contents());
  }

  return chrome_controller_;
}

} // namespace oxide
