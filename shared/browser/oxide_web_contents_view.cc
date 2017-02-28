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
#include "base/optional.h"
#include "cc/layers/solid_color_layer.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/browser/renderer_host/render_widget_host_impl.h" // nogncheck
#include "content/browser/renderer_host/render_widget_host_input_event_router.h" // nogncheck
#include "content/browser/web_contents/web_contents_impl.h" // nogncheck
#include "content/common/text_input_state.h" // nogncheck
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/WebKit/public/platform/WebDragOperation.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "third_party/WebKit/public/platform/WebMouseEvent.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/keycodes/dom/dom_key.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/gfx/geometry/vector2d_conversions.h"
#include "ui/gfx/geometry/vector2d_f.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/gfx/selection_bound.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/clipboard/oxide_clipboard.h"
#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_data.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/context_menu/web_context_menu_host.h"
#include "shared/browser/input/input_method_context.h"
#include "shared/browser/touch_selection/touch_editing_menu_controller_impl.h"
#include "shared/browser/touch_selection/touch_handle_drawable_host.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_unowned_user_data.h"

#include "chrome_controller.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_drag_source.h"
#include "oxide_fullscreen_helper.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view_client.h"
#include "screen.h"
#include "web_contents_client.h"
#include "web_popup_menu_host.h"

namespace oxide {

namespace {

int kUserDataKey;

// Qt input methods don’t generate key events, but a lot of web pages out there
// rely on keydown and keyup events to e.g. perform search-as-you-type or
// enable/disable a submit button based on the contents of a text input field,
// so we send a fake pair of keydown/keyup events.
// This mimicks what is done in GtkIMContextWrapper::HandlePreeditChanged(…)
// and GtkIMContextWrapper::HandleCommit(…)
// (see content/browser/renderer_host/gtk_im_context_wrapper.cc).
void SendFakeCompositionKeyEvent(content::RenderWidgetHostImpl* host,
                                 blink::WebInputEvent::Type type) {
  content::NativeWebKeyboardEvent fake_event(type, 0, base::TimeTicks::Now());
  fake_event.windowsKeyCode = ui::VKEY_PROCESSKEY;
  fake_event.skip_in_browser = true;
  fake_event.domKey = ui::DomKey::Key::PROCESS;
  host->ForwardKeyboardEvent(fake_event);
}

gfx::SelectionBound ComputeOffsetSelectionBound(
    const gfx::SelectionBound& in,
    const gfx::Vector2dF& offset) {
  gfx::SelectionBound bound(in);
  bound.SetEdge(bound.edge_top() + offset, bound.edge_bottom() + offset);

  return std::move(bound);
}

}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

WebContentsView::WebContentsView(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      client_(nullptr),
      compositor_(Compositor::Create(this)),
      root_layer_(cc::SolidColorLayer::Create()),
      current_drag_allowed_ops_(blink::WebDragOperationNone),
      current_drag_op_(blink::WebDragOperationNone),
      chrome_controller_(ChromeController::CreateForWebContents(web_contents)) {
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

gfx::RectF WebContentsView::GetBoundsF() const {
  if (!client_) {
    return gfx::RectF();
  }

  if (ViewSizeShouldBeScreenSize()) {
    return gfx::RectF(GetDisplay().bounds());
  }

  return client_->GetBounds();
}

gfx::Rect WebContentsView::GetTopLevelWindowBounds() const {
  if (!client_) {
    return gfx::Rect();
  }

  return client_->GetTopLevelWindowBounds();
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

void WebContentsView::DidCloseContextMenu() {
  DCHECK(active_context_menu_);

  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(
      FROM_HERE,
      active_context_menu_.release());
}

void WebContentsView::DidHidePopupMenu() {
  DCHECK(active_popup_menu_);

  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE,
                                                  active_popup_menu_.release());
}

void WebContentsView::CursorChangedInternal(RenderWidgetHostView* view) {
  client_->UpdateCursor(view ? view->current_cursor() : content::WebCursor());
}

void WebContentsView::TextInputStateChangedInternal(
    const content::TextInputState* state) {
  InputMethodContext* context = client_->GetInputMethodContext();
  if (context) {
    context->TextInputStateChanged(
        state ? state->type : ui::TEXT_INPUT_TYPE_NONE,
        state ? state->selection_start : -1,
        state ? state->selection_end : -1,
        state ? state->show_ime_if_needed : false);
  }

  if (!editing_capabilities_changed_callback_.is_null()) {
    editing_capabilities_changed_callback_.Run();
  }
}

void WebContentsView::SelectionBoundsChangedInternal(
    const content::TextInputManager::SelectionRegion& region) {
  InputMethodContext* context = client_->GetInputMethodContext();
  if (!context) {
    return;
  }

  gfx::Vector2dF offset(0, chrome_controller_->GetTopContentOffset());

  context->SelectionBoundsChanged(
      ComputeOffsetSelectionBound(region.anchor, offset),
      ComputeOffsetSelectionBound(region.focus, offset),
      region.caret_rect + gfx::ToRoundedVector2d(offset));
}

void WebContentsView::TextSelectionChangedInternal(
    const content::TextInputManager::TextSelection& selection) {
  InputMethodContext* context = client_->GetInputMethodContext();
  if (context) {
    context->TextSelectionChanged(selection.offset(), selection.range());
  }

  if (!editing_capabilities_changed_callback_.is_null()) {
    editing_capabilities_changed_callback_.Run();
  }
}

void WebContentsView::SyncClientWithNewView(RenderWidgetHostView* view) {
  if (!client_) {
    return;
  }

  if (!view) {
    view = GetRenderWidgetHostView();
  }

  CursorChangedInternal(view);

  if (touch_editing_menu_controller_) {
    touch_editing_menu_controller_->TouchSelectionControllerSwapped();
  }

  content::TextInputManager* text_input_manager =
      view ? view->GetTextInputManager() : nullptr;

  TextInputStateChangedInternal(
      view ? text_input_manager->GetTextInputState() : nullptr);

  content::RenderWidgetHostViewBase* focused_view =
      view ? view->GetFocusedViewForTextSelection() : nullptr;

  const content::TextInputManager::SelectionRegion* region =
      view ? text_input_manager->GetSelectionRegion(focused_view) : nullptr;
  SelectionBoundsChangedInternal(
      region ? *region : content::TextInputManager::SelectionRegion());

  const content::TextInputManager::TextSelection* selection =
      view ? text_input_manager->GetTextSelection(focused_view) : nullptr;
  TextSelectionChangedInternal(
      selection ? *selection : content::TextInputManager::TextSelection());
}

const content::TextInputState* WebContentsView::GetTextInputState() const {
  RenderWidgetHostView* view = GetRenderWidgetHostView();
  if (!view) {
    return nullptr;
  }

  return view->GetTextInputManager()->GetTextInputState();
}

const content::TextInputManager::TextSelection*
WebContentsView::GetTextSelection() const {
  RenderWidgetHostView* view = GetRenderWidgetHostView();
  if (!view) {
    return nullptr;
  }

  content::TextInputManager* text_input_manager = view->GetTextInputManager();
  content::RenderWidgetHostViewBase* focused_view =
      view->GetFocusedViewForTextSelection();

  return text_input_manager->GetTextSelection(focused_view);
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
  if (touch_editing_menu_controller_ &&
      touch_editing_menu_controller_->HandleContextMenu(params)) {
    return;
  }

  ui::TouchSelectionController* tsc = GetTouchSelectionController();
  if (tsc) {
    tsc->HideAndDisallowShowingAutomatically();
  }

  active_context_menu_ =
      base::MakeUnique<WebContextMenuHost>(
          render_frame_host,
          params,
          chrome_controller_->GetTopContentOffset(),
          base::Bind(&WebContentsView::DidCloseContextMenu,
                     base::Unretained(this)));
  active_context_menu_->Show();
}

void WebContentsView::StartDragging(
    const content::DropData& drop_data,
    blink::WebDragOperationsMask allowed_ops,
    const gfx::ImageSkia& image,
    const gfx::Vector2d& image_offset,
    const content::DragEventSourceInfo& event_info,
    content::RenderWidgetHostImpl* source_rwh) {
  if (drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag when one is already in progress";
    web_contents()->SystemDragEnded(source_rwh);
    return;
  }

  DCHECK(!drag_source_rwh_);

  drag_source_ =
      BrowserPlatformIntegration::GetInstance()->CreateDragSource(this);
  if (!drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag - not supported";
    web_contents()->SystemDragEnded(source_rwh);
    return;
  }
  drag_source_rwh_ = source_rwh;

  ui::TouchSelectionController* selection_controller =
      GetTouchSelectionController();
  if (selection_controller) {
    selection_controller->HideAndDisallowShowingAutomatically();
  }

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

void WebContentsView::GotFocus() {
  web_contents_impl()->NotifyWebContentsFocused();
}

void WebContentsView::ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                                    const gfx::Rect& bounds,
                                    int item_height,
                                    double item_font_size,
                                    int selected_item,
                                    const std::vector<content::MenuItem>& items,
                                    bool right_aligned,
                                    bool allow_multiple_selection) {
  gfx::Vector2d offset(0, chrome_controller_->GetTopContentOffset());

  active_popup_menu_ =
      base::MakeUnique<WebPopupMenuHost>(
          render_frame_host,
          items,
          selected_item,
          allow_multiple_selection,
          bounds + offset,
          base::Bind(&WebContentsView::DidHidePopupMenu,
                     base::Unretained(this)));
  active_popup_menu_->Show();
}

void WebContentsView::HidePopupMenu() {
  active_popup_menu_.reset();
}

void WebContentsView::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  if (active_popup_menu_ &&
      active_popup_menu_->GetRenderFrameHost() == render_frame_host) {
    active_popup_menu_.reset();
  }

  if (active_context_menu_ &&
      active_context_menu_->GetRenderFrameHost() == render_frame_host) {
    active_context_menu_.reset();
  }
}

void WebContentsView::RenderViewHostChanged(
    content::RenderViewHost* old_host,
    content::RenderViewHost* new_host) {
  if (old_host && old_host->GetWidget()->GetView()) {
    RenderWidgetHostView* rwhv =
        static_cast<RenderWidgetHostView*>(old_host->GetWidget()->GetView());
    rwhv->SetContainer(nullptr);
  }

  if (!new_host) {
    return;
  }

  if (new_host->GetWidget()->GetView()) {
    RenderWidgetHostView* rwhv =
        static_cast<RenderWidgetHostView*>(new_host->GetWidget()->GetView());
    rwhv->SetContainer(this);

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

  SyncClientWithNewView();
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

  SyncClientWithNewView();
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

  // We pass the view explicitly here because GetRenderWidgetHostView() still
  // returns the fullscreen view
  SyncClientWithNewView(orig_rwhv);
}

void WebContentsView::DidAttachInterstitialPage() {
  DCHECK(!interstitial_rwh_);
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

  // Save the host, as it's detached by the time DidDetachInterstitialPage is
  // called
  interstitial_rwh_ = rwhv->GetRenderWidgetHost();

  // Content takes care of adjusting visibility and sending focus / resize
  // messages, so there's nothing else to do with |rwhv|

  SyncClientWithNewView();
}

void WebContentsView::DidDetachInterstitialPage() {
  DCHECK(interstitial_rwh_);

  content::RenderWidgetHost* interstitial_rwh = interstitial_rwh_.get();
  interstitial_rwh_ = nullptr;
  if (interstitial_rwh) {
    static_cast<RenderWidgetHostView*>(
        interstitial_rwh->GetView())->SetContainer(nullptr);
  }

  RenderWidgetHostView* orig_rwhv = GetRenderWidgetHostView();
  if (orig_rwhv) {
    DCHECK(!interstitial_rwh || interstitial_rwh->GetView() != orig_rwhv);   
    orig_rwhv->SetSize(GetSize());
  }

  SyncClientWithNewView();
}

void WebContentsView::CompositorSwapFrame(CompositorFrameHandle* handle,
                                          const SwapAckCallback& callback) {
  compositor_ack_callbacks_.push(callback);

  if (current_compositor_frame_.get()) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = handle;

  RenderWidgetHostView* rwhv =
      rwh_at_last_commit_ ?
          static_cast<RenderWidgetHostView*>(rwh_at_last_commit_->GetView())
          : nullptr;
  float last_top_content_offset = chrome_controller_->GetTopContentOffset();
  static cc::CompositorFrameMetadata null_metadata;
  const cc::CompositorFrameMetadata& metadata =
      rwhv ? rwhv->last_drawn_frame_metadata() : null_metadata;
  chrome_controller_->FrameMetadataUpdated(
      rwhv ? base::make_optional(metadata.Clone()) : base::nullopt);
  if (last_top_content_offset != chrome_controller_->GetTopContentOffset()) {
    if (rwhv && last_focused_widget_for_text_selection_ && client_) {
      content::TextInputManager* text_input_manager =
          rwhv->GetTextInputManager();
      const content::TextInputManager::SelectionRegion* region =
          text_input_manager->GetSelectionRegion(
              static_cast<content::RenderWidgetHostViewBase*>(
                  last_focused_widget_for_text_selection_->GetView()));
      SelectionBoundsChangedInternal(*region);
    }
  }

  gfx::Vector2d offset(0,
                       chrome_controller_->GetTopContentOffset() *
                           metadata.device_scale_factor);
  handle->data()->rect_in_pixels += offset;

  swap_compositor_frame_callbacks_.Notify(handle->data(), metadata);

  if (client_) {
    client_->SwapCompositorFrame();
  } else {
    DidCommitCompositorFrame();
  }
}

void WebContentsView::CompositorDidCommit() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  rwh_at_last_commit_ = rwhv ? rwhv->GetRenderWidgetHost() : nullptr;
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

  if (!drag_source_rwh_) {
    return;
  }

  gfx::Point screen_point =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreen()
        ->GetCursorScreenPoint();
  gfx::Point view_point =
      screen_point - gfx::Vector2d(GetBounds().origin().x(),
                                   GetBounds().origin().y());
  web_contents_impl()->DragSourceEndedAt(
      view_point.x(), view_point.y(),
      screen_point.x(), screen_point.y(),
      operation,
      content::RenderWidgetHostImpl::From(drag_source_rwh_.get()));

  web_contents()->SystemDragEnded(drag_source_rwh_.get());

  drag_source_.reset();
  drag_source_rwh_ = nullptr;
}

void WebContentsView::InputPanelVisibilityChanged() {
  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::SetComposingText(
    const base::string16& text,
    const std::vector<blink::WebCompositionUnderline>& underlines,
    const gfx::Range& selection_range) {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->OnUserInput();

  content::TextInputManager* text_input_manager =
      rwhv->GetTextInputManager();
  content::RenderWidgetHostImpl* widget = text_input_manager->GetActiveWidget();
  if (!widget) {
    return;
  }

  SendFakeCompositionKeyEvent(widget, blink::WebInputEvent::RawKeyDown);
  widget->ImeSetComposition(text,
                            underlines,
                            gfx::Range::InvalidRange(),
                            selection_range.start(),
                            selection_range.end());
  SendFakeCompositionKeyEvent(widget, blink::WebInputEvent::KeyUp);
}

void WebContentsView::CommitText(const base::string16& text,
                                 const gfx::Range& replacement_range) {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->OnUserInput();

  content::TextInputManager* text_input_manager =
      rwhv->GetTextInputManager();
  content::RenderWidgetHostImpl* widget = text_input_manager->GetActiveWidget();
  if (!widget) {
    return;
  }

  SendFakeCompositionKeyEvent(widget, blink::WebInputEvent::RawKeyDown);
  widget->ImeCommitText(text,
                        std::vector<blink::WebCompositionUnderline>(),
                        replacement_range,
                        false);
  SendFakeCompositionKeyEvent(widget, blink::WebInputEvent::KeyUp);
}

base::string16 WebContentsView::GetSelectionText() const {
  const content::TextInputManager::TextSelection* selection =
      GetTextSelection();
  if (!selection) {
    return base::string16();
  }

  return selection->text();
}

bool WebContentsView::GetSelectedText(base::string16* text) const {
  const content::TextInputManager::TextSelection* selection =
      GetTextSelection();
  if (!selection) {
    text->clear();
    return false;
  }

  *text = selection->selected_text();
  return true;
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

  CursorChangedInternal(view);
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
  return chrome_controller_->top_controls_height();
}

std::unique_ptr<ui::TouchHandleDrawable>
WebContentsView::CreateTouchHandleDrawable() {
  if (!client_) {
    return nullptr;
  }

  std::unique_ptr<TouchHandleDrawableHost> host =
      base::MakeUnique<TouchHandleDrawableHost>(chrome_controller_.get());

  std::unique_ptr<ui::TouchHandleDrawable> drawable =
      client_->CreateTouchHandleDrawable();
  if (!drawable) {
    return nullptr;
  }

  host->Init(std::move(drawable));

  return std::move(host);
}

void WebContentsView::OnTouchSelectionEvent(RenderWidgetHostView* view,
                                            ui::SelectionEventType event) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  if (!touch_editing_menu_controller_) {
    return;
  }

  touch_editing_menu_controller_->OnSelectionEvent(event);
}

void WebContentsView::TextInputStateChanged(
    RenderWidgetHostView* view,
    const content::TextInputState* state) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  TextInputStateChangedInternal(state);
}

void WebContentsView::ImeCancelComposition(RenderWidgetHostView* view) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  InputMethodContext* context = client_->GetInputMethodContext();
  if (!context) {
    return;
  }

  context->CancelComposition();
}

void WebContentsView::SelectionBoundsChanged(
    RenderWidgetHostView* view,
    const content::TextInputManager::SelectionRegion* region) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  SelectionBoundsChangedInternal(*region);
}

void WebContentsView::TextSelectionChanged(
    RenderWidgetHostView* view,
    content::RenderWidgetHostViewBase* focused_view,
    const content::TextInputManager::TextSelection* selection) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  last_focused_widget_for_text_selection_ = focused_view->GetRenderWidgetHost();
  TextSelectionChangedInternal(*selection);
}

void WebContentsView::FocusedNodeChanged(RenderWidgetHostView* view,
                                         bool is_editable_node) {
  if (!client_) {
    return;
  }

  if (view != GetRenderWidgetHostView()) {
    return;
  }

  InputMethodContext* context = client_->GetInputMethodContext();
  if (!context) {
    return;
  }

  context->FocusedNodeChanged(is_editable_node);
}

void WebContentsView::OnDisplayPropertiesChanged(
    const display::Display& display) {
  if (display.id() != GetDisplay().id()) {
    return;
  }

  ScreenChanged();
}

ChromeController* WebContentsView::GetChromeController() const {
  return chrome_controller_.get();
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
    client_->view_ = nullptr;
  }

  InputMethodContextClient::DetachFromContext();

  client_ = client;

  if (client_) {
    DCHECK(!client_->view_);
    client_->view_ = this;

    InputMethodContextClient::AttachToContext(client_->GetInputMethodContext());
  }

  // Update view from client
  WasResized();
  VisibilityChanged();
  FocusChanged();
  ScreenChanged();
  TopLevelWindowBoundsChanged();

  // Update client from view
  SyncClientWithNewView();
}

blink::WebContextMenuData::EditFlags
WebContentsView::GetEditingCapabilities() const {
  blink::WebContextMenuData::EditFlags flags =
      blink::WebContextMenuData::CanDoNone;

  const content::TextInputState* state = GetTextInputState();
  ui::TextInputType text_input_type =
      state ? state->type : ui::TEXT_INPUT_TYPE_NONE;

  bool editable = (text_input_type != ui::TEXT_INPUT_TYPE_NONE);
  bool readable = (text_input_type != ui::TEXT_INPUT_TYPE_PASSWORD);

  const content::TextInputManager::TextSelection* selection =
      GetTextSelection();
  bool has_selection = selection && !selection->range().is_empty();

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

void WebContentsView::HandleMouseEvent(blink::WebMouseEvent event) {
  event.y = std::floor(event.y - chrome_controller_->GetTopContentOffset());
  mouse_state_.UpdateEvent(&event);

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->OnUserInput();

  content::RenderWidgetHostInputEventRouter* router =
      content::RenderWidgetHostImpl::From(rwhv->GetRenderWidgetHost())
          ->delegate()->GetInputEventRouter();
  router->RouteMouseEvent(rwhv, &event, ui::LatencyInfo());
}

void WebContentsView::HandleMotionEvent(const ui::MotionEvent& event) {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->HandleTouchEvent(event);
}

void WebContentsView::HandleWheelEvent(blink::WebMouseWheelEvent event) {
  event.y = std::floor(event.y - chrome_controller_->GetTopContentOffset());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->OnUserInput();

  content::RenderWidgetHostInputEventRouter* router =
      content::RenderWidgetHostImpl::From(rwhv->GetRenderWidgetHost())
          ->delegate()->GetInputEventRouter();
  router->RouteMouseWheelEvent(rwhv, &event, ui::LatencyInfo());
}

void WebContentsView::HandleDragEnter(
    const content::DropData& drop_data,
    const gfx::Point& location,
    blink::WebDragOperationsMask allowed_ops,
    int key_modifiers) {
  current_drop_data_.reset(new content::DropData(drop_data));
  current_drag_allowed_ops_ = allowed_ops;

  gfx::Vector2d offset(0, chrome_controller_->GetTopContentOffset());

  current_drag_location_ = location - offset;
  current_drag_screen_location_ =
      BrowserPlatformIntegration::GetInstance()
          ->GetScreen()
          ->GetCursorScreenPoint();

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  current_drag_target_ = rwh;

  rwh->DragTargetDragEnter(*current_drop_data_,
                           current_drag_location_,
                           current_drag_screen_location_,
                           current_drag_allowed_ops_,
                           key_modifiers);
}

blink::WebDragOperation WebContentsView::HandleDragMove(
    const gfx::Point& location,
    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  gfx::Vector2d offset(0, chrome_controller_->GetTopContentOffset());
  current_drag_location_ = location - offset;

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (rwh != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    current_drag_location_,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  current_drag_screen_location_ =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreen()
        ->GetCursorScreenPoint();
  rwh->DragTargetDragOver(current_drag_location_,
                          current_drag_screen_location_,
                          current_drag_allowed_ops_,
                          key_modifiers);

  return current_drag_op_;
}

void WebContentsView::HandleDragLeave() {
  if (!current_drop_data_) {
    return;
  }

  current_drop_data_.reset();

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (rwh != current_drag_target_) {
    return;
  }

  rwh->DragTargetDragLeave(current_drag_location_,
                           current_drag_screen_location_);
}

blink::WebDragOperation WebContentsView::HandleDrop(const gfx::Point& location,
                                                    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  gfx::Vector2d offset(0, chrome_controller_->GetTopContentOffset());

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (rwh != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    location - offset,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreen()
        ->GetCursorScreenPoint();
  rwh->DragTargetDrop(*current_drop_data_,
                      location - offset,
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

std::unique_ptr<WebContentsView::SwapCompositorFrameSubscription>
WebContentsView::AddSwapCompositorFrameCallback(
    const base::Callback<void(const CompositorFrameData*,
                              const cc::CompositorFrameMetadata&)>& callback) {
  return swap_compositor_frame_callbacks_.Add(callback);
}

void WebContentsView::WasResized() {
  ResizeCompositorViewport();
  UpdateContentsSize();

  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (rwh) {
    rwh->WasResized();
  }

  if (touch_editing_menu_controller_) {
    touch_editing_menu_controller_->SetViewportBounds(GetBoundsF());
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebContentsView::ScreenRectsChanged() {
  content::RenderWidgetHost* rwh = GetRenderWidgetHost();
  if (!rwh) {
    return;
  }

  content::RenderWidgetHostImpl::From(rwh)->SendScreenRects();
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
  InputMethodContext* im_context =
      client_ ? client_->GetInputMethodContext() : nullptr;

  if (HasFocus()) {
    if (rwhv) {
      rwhv->Focus();
    }
    if (im_context) {
      im_context->FocusIn();
    }
  } else {
    if (rwhv) {
      rwhv->Blur();
    }
    if (im_context) {
      im_context->FocusOut();
    }
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

void WebContentsView::TopLevelWindowBoundsChanged() {
  if (!touch_editing_menu_controller_) {
    return;
  }

  touch_editing_menu_controller_->SetTopLevelWindowBounds(
      GetTopLevelWindowBounds());
}

void WebContentsView::InitializeTouchEditingController() {
  WebContentsClient* contents_client =
      WebContentsClient::FromWebContents(web_contents());

  if (contents_client) {
    touch_editing_menu_controller_ =
        contents_client->CreateOverrideTouchEditingMenuController(this);
    if (!touch_editing_menu_controller_) {
      touch_editing_menu_controller_ =
          base::MakeUnique<TouchEditingMenuControllerImpl>(this);
    }
    touch_editing_menu_controller_->SetViewportBounds(GetBoundsF());
    touch_editing_menu_controller_->SetTopLevelWindowBounds(
        GetTopLevelWindowBounds());
  } else {
    touch_editing_menu_controller_.reset();
  }
}

} // namespace oxide
