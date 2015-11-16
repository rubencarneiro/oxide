// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_web_view.h"

#include <queue>
#include <utility>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/supports_user_data.h"
#include "cc/layers/layer.h"
#include "cc/layers/solid_color_layer.h"
#include "cc/trees/layer_tree_settings.h"
#include "components/sessions/content/content_serialized_navigation_builder.h"
#include "content/browser/renderer_host/event_with_latency_info.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/interstitial_page.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/resource_request_details.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/menu_item.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"
#include "ipc/ipc_message_macros.h"
#include "net/base/net_errors.h"
#include "net/ssl/ssl_info.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/window_open_disposition.h"
#include "ui/events/event.h"
#include "ui/gl/gl_implementation.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "url/gurl.h"
#include "url/url_constants.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_data.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/input/oxide_ime_bridge.h"
#include "shared/browser/input/oxide_input_method_context.h"
#include "shared/browser/media/oxide_media_capture_devices_dispatcher.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher.h"
#include "shared/browser/permissions/oxide_temporary_saved_permission_context.h"
#include "shared/browser/ssl/oxide_certificate_error_dispatcher.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_unowned_user_data.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_event_utils.h"
#include "oxide_favicon_helper.h"
#include "oxide_file_picker.h"
#include "oxide_find_controller.h"
#include "oxide_javascript_dialog_manager.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_script_message_contents_helper.h"
#include "oxide_web_contents_unloader.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_context_menu.h"
#include "oxide_web_frame_tree.h"
#include "oxide_web_frame.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view_client.h"
#include "oxide_web_view_contents_helper.h"

#if defined(ENABLE_MEDIAHUB)
#include "shared/browser/media/oxide_media_web_contents_observer.h"
#endif

#define DCHECK_VALID_SOURCE_CONTENTS DCHECK_EQ(source, web_contents());

namespace oxide {

namespace {

int kUserDataKey;

void FillLoadURLParamsFromOpenURLParams(
    content::NavigationController::LoadURLParams* load_params,
    const content::OpenURLParams& params) {
  load_params->transition_type = params.transition;
  load_params->frame_tree_node_id = params.frame_tree_node_id;
  load_params->referrer = params.referrer;
  load_params->redirect_chain = params.redirect_chain;
  load_params->extra_headers = params.extra_headers;
  load_params->is_renderer_initiated = params.is_renderer_initiated;
  load_params->transferred_global_request_id =
      params.transferred_global_request_id;
  load_params->should_replace_current_entry =
      params.should_replace_current_entry;

  if (params.uses_post && !params.is_renderer_initiated) {
    load_params->load_type =
        content::NavigationController::LOAD_TYPE_BROWSER_INITIATED_HTTP_POST;
    load_params->browser_initiated_post_data =
        params.browser_initiated_post_data;
  }
}

void CreateHelpers(content::WebContents* contents,
                   content::WebContents* opener = nullptr) {
  new WebViewContentsHelper(contents, opener);
  CertificateErrorDispatcher::CreateForWebContents(contents);
  PermissionRequestDispatcher::CreateForWebContents(contents);
  ScriptMessageContentsHelper::CreateForWebContents(contents);
#if defined(ENABLE_MEDIAHUB)
  new MediaWebContentsObserver(contents);
#endif
  FindController::CreateForWebContents(contents);
  WebFrameTree::CreateForWebContents(contents);
  FaviconHelper::CreateForWebContents(contents);
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(ui::PageTransition)
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(ContentType)

base::LazyInstance<std::vector<WebView*> > g_all_web_views;

}

void WebView::WebContentsDeleter::operator()(content::WebContents* contents) {
  WebContentsUnloader::GetInstance()->Unload(make_scoped_ptr(contents));
};

WebViewIterator::WebViewIterator(const std::vector<WebView*>& views) {
  for (std::vector<WebView*>::const_iterator it = views.begin();
       it != views.end(); ++it) {
    views_.push_back((*it)->AsWeakPtr());
  }
  current_ = views_.begin();
}

WebViewIterator::~WebViewIterator() {}

bool WebViewIterator::HasMore() const {
  return current_ != views_.end();
}

WebView* WebViewIterator::GetNext() {
  while (current_ != views_.end()) {
    base::WeakPtr<WebView>& view = *current_;
    current_++;
    if (view.get()) {
      return view.get();
    }
  }

  return nullptr;
}

WebView::Params::Params()
    : client(nullptr),
      context(nullptr),
      incognito(false),
      restore_type(content::NavigationController::RESTORE_CURRENT_SESSION),
      restore_index(0) {}

WebView::Params::~Params() {}

// static
WebViewIterator WebView::GetAllWebViews() {
  return WebViewIterator(g_all_web_views.Get());
}

WebView::WebView(WebViewClient* client)
    : client_(client),
      web_contents_helper_(nullptr),
      compositor_(Compositor::Create(this)),
      root_layer_(cc::SolidColorLayer::Create(cc::LayerSettings())),
      fullscreen_granted_(false),
      fullscreen_requested_(false),
      blocked_content_(CONTENT_TYPE_NONE),
      location_bar_height_pix_(0),
      location_bar_constraints_(blink::WebTopControlsBoth),
      location_bar_animated_(true),
      weak_factory_(this) {
  CHECK(client) << "Didn't specify a client";

  root_layer_->SetIsDrawable(true);
  root_layer_->SetBackgroundColor(SK_ColorWHITE);
  root_layer_->SetBounds(GetViewSizeDip());

  compositor_->SetRootLayer(root_layer_);
  compositor_->SetViewportSize(GetViewSizePix());
  compositor_->SetVisibility(IsVisible());
  compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);

  CompositorObserver::Observe(compositor_.get());
  InputMethodContextObserver::Observe(client_->GetInputMethodContext());
}

void WebView::CommonInit(scoped_ptr<content::WebContents> contents) {
  web_contents_.reset(contents.release());

  // Attach ourself to the WebContents
  web_contents_->SetDelegate(this);
  web_contents_->SetUserData(&kUserDataKey,
                             new UnownedUserData<WebView>(this));

  content::WebContentsObserver::Observe(web_contents_.get());

  // Set the initial WebPreferences. This has to happen after attaching
  // ourself to the WebContents, as the pref update needs to call back in
  // to us (via CanCreateWindows)
  web_contents_helper_ =
      WebViewContentsHelper::FromWebContents(web_contents_.get());
  web_contents_helper_->WebContentsAdopted();

  registrar_.Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                 content::NotificationService::AllBrowserContextsAndSources());
  registrar_.Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                 content::NotificationService::AllBrowserContextsAndSources());

  WebContentsView::FromWebContents(web_contents_.get())->SetContainer(this);

  DCHECK(std::find(g_all_web_views.Get().begin(),
                   g_all_web_views.Get().end(),
                   this) ==
         g_all_web_views.Get().end());
  g_all_web_views.Get().push_back(this);
}

RenderWidgetHostView* WebView::GetRenderWidgetHostView() const {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetFullscreenRenderWidgetHostView();
  if (!rwhv) {
    rwhv = web_contents_->GetRenderWidgetHostView();
  }

  return static_cast<RenderWidgetHostView *>(rwhv);
}

content::RenderViewHost* WebView::GetRenderViewHost() const {
  return web_contents_->GetRenderViewHost();
}

content::RenderWidgetHost* WebView::GetRenderWidgetHost() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return nullptr;
  }

  return rwhv->GetRenderWidgetHost();
}

void WebView::DispatchLoadFailed(const GURL& validated_url,
                                 int error_code,
                                 const base::string16& error_description,
                                 bool is_provisional_load) {
  if (error_code == net::ERR_ABORTED) {
    client_->LoadStopped(validated_url);
  } else {
    content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
    client_->LoadFailed(validated_url,
                        error_code,
                        base::UTF16ToUTF8(error_description),
                        is_provisional_load ?
                            0 : entry->GetHttpStatusCode());
  }
}

void WebView::OnDidBlockDisplayingInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_DISPLAY;

  client_->ContentBlocked();
}

void WebView::OnDidBlockRunningInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_SCRIPT) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_SCRIPT;

  client_->ContentBlocked();
}

bool WebView::ShouldScrollFocusedEditableNodeIntoView() {
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

void WebView::MaybeScrollFocusedEditableNodeIntoView() {
  if (!ShouldScrollFocusedEditableNodeIntoView()) {
    return;
  }

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  content::RenderWidgetHostImpl::From(host)
      ->ScrollFocusedEditableNodeIntoRect(GetViewBoundsDip());
}

float WebView::GetFrameMetadataScaleToPix() {
  return compositor_frame_metadata().device_scale_factor *
         compositor_frame_metadata().page_scale_factor;
}

void WebView::InitializeTopControlsForHost(content::RenderViewHost* rvh,
                                           bool initial_host) {
  // Show the location bar if this is the initial RVH and the constraints
  // are set to blink::WebTopControlsBoth
  blink::WebTopControlsState current = location_bar_constraints_;
  if (initial_host &&
      location_bar_constraints_ == blink::WebTopControlsBoth) {
    current = blink::WebTopControlsShown;
  }

  rvh->Send(
      new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                          location_bar_constraints_,
                                          current,
                                          false));
}

void WebView::DispatchPrepareToCloseResponse(bool proceed) {
  client_->PrepareToCloseResponseReceived(proceed);
}

void WebView::MaybeCancelFullscreenMode() {
  if (IsFullscreen()) {
    // The application might have granted fullscreen by now
    return;
  }

  web_contents_->ExitFullscreen();
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return client_->GetScriptMessageHandlerCount();
}

const ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return client_->GetScriptMessageHandlerAt(index);
}

void WebView::InputPanelVisibilityChanged() {
  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::CompositorDidCommit() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  pending_compositor_frame_metadata_ = rwhv->compositor_frame_metadata();
}

void WebView::CompositorSwapFrame(CompositorFrameHandle* handle) {
  received_surface_ids_.push(handle->data()->surface_id);

  if (current_compositor_frame_.get()) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = handle;

  cc::CompositorFrameMetadata old = compositor_frame_metadata_;
  compositor_frame_metadata_ = pending_compositor_frame_metadata_;

  if (IsFullscreen()) {
    // Ensure that the location bar is always hidden in fullscreen. This
    // is required because fullscreen RenderWidgets don't have a mechanism
    // to control the location bar height
    compositor_frame_metadata_.location_bar_content_translation =
        gfx::Vector2dF();
    compositor_frame_metadata_.location_bar_offset =
        gfx::Vector2dF(0.0f, -GetLocationBarHeightDip());
  }

  // TODO(chrisccoulson): Merge these
  client_->FrameMetadataUpdated(old);
  client_->SwapCompositorFrame();
}

void WebView::WebPreferencesDestroyed() {
  client_->WebPreferencesDestroyed();
}

void WebView::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (content::Source<content::NavigationController>(source).ptr() !=
      &GetWebContents()->GetController()) {
    return;
  }
  if (type == content::NOTIFICATION_NAV_LIST_PRUNED) {
    content::PrunedDetails* pruned_details =
        content::Details<content::PrunedDetails>(details).ptr();
    client_->NavigationListPruned(pruned_details->from_front,
                                  pruned_details->count);
  } else if (type == content::NOTIFICATION_NAV_ENTRY_CHANGED) {
    int index =
        content::Details<content::EntryChangedDetails>(details).ptr()->index;
    client_->NavigationEntryChanged(index);
  }
}

Compositor* WebView::GetCompositor() const {
  return compositor_.get();
}

void WebView::AttachLayer(scoped_refptr<cc::Layer> layer) {
  DCHECK(layer.get());
  root_layer_->InsertChild(layer, 0);
  root_layer_->SetIsDrawable(false);
}

void WebView::DetachLayer(scoped_refptr<cc::Layer> layer) {
  DCHECK(layer.get());
  DCHECK_EQ(layer->parent(), root_layer_.get());
  layer->RemoveFromParent();
  if (root_layer_->children().size() == 0) {
    root_layer_->SetIsDrawable(true);
  }
}

void WebView::CursorChanged() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  client_->UpdateCursor(rwhv->current_cursor());
}

bool WebView::HasFocus(const RenderWidgetHostView* view) const {
  if (!HasFocus()) {
    return false;
  }

  return view == GetRenderWidgetHostView();
}

bool WebView::IsFullscreen() const {
  return fullscreen_granted_ && fullscreen_requested_;
}

void WebView::ShowContextMenu(content::RenderFrameHost* render_frame_host,
                              const content::ContextMenuParams& params) {
  WebContextMenu* menu = client_->CreateContextMenu(render_frame_host, params);
  if (!menu) {
    return;
  }

  menu->Show();
}

void WebView::ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                            const gfx::Rect& bounds,
                            int selected_item,
                            const std::vector<content::MenuItem>& items,
                            bool allow_multiple_selection) {
  DCHECK(!active_popup_menu_);

  WebPopupMenu* menu = client_->CreatePopupMenu(render_frame_host);
  if (!menu) {
    static_cast<content::RenderFrameHostImpl *>(
        render_frame_host)->DidCancelPopupMenu();
    return;
  }

  active_popup_menu_ = menu->GetWeakPtr();

  menu->Show(bounds, items, selected_item, allow_multiple_selection);
}

void WebView::HidePopupMenu() {
  if (!active_popup_menu_) {
    return;
  }

  active_popup_menu_->Close();
}

content::WebContents* WebView::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  // We get here from the following places:
  //  1) RenderFrameHostManager::OnCrossSiteResponse. In this case, disposition
  //     is always CURRENT_TAB and we always want to perform the navigation.
  //     Asking the embedder whether to proceed is done via
  //     NavigationInterceptResourceThrottle.
  //  2) Non CURRENT_TAB navigations as a result of keyboard modifiers. In
  //     this case, we want to ask the embedder whether to perform the
  //     navigation unless we change it to CURRENT_TAB (in which case, we ask
  //     the embedder via NavigationInterceptResourceThrottle)

  if (params.disposition != CURRENT_TAB &&
      params.disposition != NEW_FOREGROUND_TAB &&
      params.disposition != NEW_BACKGROUND_TAB &&
      params.disposition != NEW_POPUP &&
      params.disposition != NEW_WINDOW) {
    return nullptr;
  }

  // Block popups
  if ((params.disposition == NEW_FOREGROUND_TAB ||
       params.disposition == NEW_BACKGROUND_TAB ||
       params.disposition == NEW_WINDOW ||
       params.disposition == NEW_POPUP) &&
      !params.user_gesture && GetBrowserContext()->IsPopupBlockerEnabled()) {
    return nullptr;
  }

  WindowOpenDisposition disposition = params.disposition;

  // If we can't create new windows, this should be a CURRENT_TAB navigation
  // in the top-level frame
  if (!CanCreateWindows() && disposition != CURRENT_TAB) {
    disposition = CURRENT_TAB;
  }

  // Handle the CURRENT_TAB case now
  if (disposition == CURRENT_TAB) {
    // XXX: We have no way to propagate OpenURLParams::user_gesture here, so
    // ResourceRequestInfo::HasUserGesture will always return false in
    // NavigationInterceptResourceThrottle
    // See https://launchpad.net/bugs/1499434
    content::NavigationController::LoadURLParams load_params(params.url);
    FillLoadURLParamsFromOpenURLParams(&load_params, params);

    web_contents_->GetController().LoadURLWithParams(load_params);

    return web_contents_.get();
  }

  // At this point, we expect all navigations to be for the main frame and
  // to be renderer initiated
  DCHECK_EQ(params.frame_tree_node_id, -1);
  DCHECK(params.is_renderer_initiated);

  // Coerce all non CURRENT_TAB navigations that don't come from a user
  // gesture to NEW_POPUP
  if (!params.user_gesture) {
    disposition = NEW_POPUP;
  }

  // Give the application a chance to block the navigation if it is
  // renderer initiated
  if (!client_->ShouldHandleNavigation(params.url,
                                       disposition,
                                       params.user_gesture)) {
    return nullptr;
  }

  // XXX(chrisccoulson): Is there a way to tell when the opener shouldn't
  // be suppressed?
  bool opener_suppressed = true;

  content::WebContents::CreateParams contents_params(
      GetBrowserContext(),
      opener_suppressed ? nullptr : web_contents_->GetSiteInstance());
  contents_params.initial_size = GetViewSizeDip();
  contents_params.initially_hidden = disposition == NEW_BACKGROUND_TAB;
  contents_params.opener_render_process_id =
      web_contents_->GetRenderProcessHost()->GetID();
  // XXX(chrisccoulson): This is probably wrong, but we're going to revisit
  //  navigations anyway, and opener_suppressed is currently always true so
  //  this is ignored
  contents_params.opener_render_frame_id =
      web_contents_->GetMainFrame()->GetRoutingID();
  contents_params.opener_suppressed = opener_suppressed;

  scoped_ptr<content::WebContents> contents(
      content::WebContents::Create(contents_params));
  if (!contents) {
    LOG(ERROR) << "Failed to create new WebContents for navigation";
    return nullptr;
  }

  CreateHelpers(contents.get(), web_contents_.get());

  WebView* new_view =
      client_->CreateNewWebView(GetViewBoundsPix(),
                                disposition,
                                contents.Pass());
  if (!new_view) {
    return nullptr;
  }

  content::NavigationController::LoadURLParams load_params(params.url);
  FillLoadURLParamsFromOpenURLParams(&load_params, params);

  new_view->GetWebContents()->GetController().LoadURLWithParams(load_params);

  return new_view->GetWebContents();
}

void WebView::NavigationStateChanged(content::WebContents* source,
                                     content::InvalidateTypes changed_flags) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (changed_flags & content::INVALIDATE_TYPE_URL) {
    client_->URLChanged();
  }

  if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
    client_->TitleChanged();
  }

  if (changed_flags & (content::INVALIDATE_TYPE_URL |
                       content::INVALIDATE_TYPE_LOAD)) {
    client_->CommandsUpdated();
  }
}

void WebView::VisibleSSLStateChanged(const content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  content::NavigationEntry* entry =
      web_contents_->GetController().GetVisibleEntry();
  if (!entry) {
    return;
  }

  SecurityStatus old_status = security_status_;
  security_status_.Update(entry->GetSSL());

  client_->SecurityStatusChanged(old_status);
}

bool WebView::ShouldCreateWebContents(
    content::WebContents* source,
    int route_id,
    int main_frame_route_id,
    int main_frame_widget_route_id,
    WindowContainerType window_container_type,
    const std::string& frame_name,
    const GURL& target_url,
    const std::string& partition_id,
    content::SessionStorageNamespace* session_storage_namespace,
    WindowOpenDisposition disposition,
    bool user_gesture) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (disposition != NEW_FOREGROUND_TAB &&
      disposition != NEW_BACKGROUND_TAB &&
      disposition != NEW_POPUP &&
      disposition != NEW_WINDOW) {
    return false;
  }

  // Note that popup blocking was done on the IO thread

  if (!CanCreateWindows()) {
    return false;
  }

  return client_->ShouldHandleNavigation(
      target_url,
      user_gesture ? disposition : NEW_POPUP,
      user_gesture);
}

void WebView::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->UnhandledKeyboardEvent(event);
}

void WebView::WebContentsCreated(content::WebContents* source,
                                 int source_frame_id,
                                 const std::string& frame_name,
                                 const GURL& target_url,
                                 content::WebContents* new_contents) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(!WebView::FromWebContents(new_contents));

  CreateHelpers(new_contents, web_contents_.get());
}

void WebView::AddNewContents(content::WebContents* source,
                             content::WebContents* new_contents,
                             WindowOpenDisposition disposition,
                             const gfx::Rect& initial_pos,
                             bool user_gesture,
                             bool* was_blocked) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(disposition == NEW_FOREGROUND_TAB ||
         disposition == NEW_BACKGROUND_TAB ||
         disposition == NEW_POPUP ||
         disposition == NEW_WINDOW) << "Invalid disposition";
  DCHECK_EQ(GetBrowserContext(),
            BrowserContext::FromContent(new_contents->GetBrowserContext()));

  if (was_blocked) {
    *was_blocked = true;
  }

  scoped_ptr<content::WebContents> contents(new_contents);

  WebView* new_view =
      client_->CreateNewWebView(initial_pos,
                                user_gesture ? disposition : NEW_POPUP,
                                contents.Pass());
  if (!new_view) {
    return;
  }

  if (was_blocked) {
    *was_blocked = false;
  }
}

void WebView::LoadProgressChanged(content::WebContents* source,
                                  double progress) {
  DCHECK_VALID_SOURCE_CONTENTS

  // XXX: Is there a way to update this when we adopt a WebContents?
  client_->LoadProgressChanged(progress);
}

void WebView::CloseContents(content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->CloseRequested();
}

void WebView::UpdateTargetURL(content::WebContents* source, const GURL& url) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->UpdateTargetURL(url);
}

bool WebView::AddMessageToConsole(content::WebContents* source,
                                  int32 level,
                                  const base::string16& message,
                                  int32 line_no,
                                  const base::string16& source_id) {
  DCHECK_VALID_SOURCE_CONTENTS

  return client_->AddMessageToConsole(level, message, line_no, source_id);
}

void WebView::BeforeUnloadFired(content::WebContents* source,
                                bool proceed,
                                bool* proceed_to_fire_unload) {
  DCHECK_VALID_SOURCE_CONTENTS

  // We take care of the unload handler on deletion
  *proceed_to_fire_unload = false;

  client_->PrepareToCloseResponseReceived(proceed);
}

content::JavaScriptDialogManager* WebView::GetJavaScriptDialogManager(
    content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS
  return JavaScriptDialogManager::GetInstance();
}

void WebView::RunFileChooser(content::WebContents* source,
                             const content::FileChooserParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  FilePicker* file_picker = client_->CreateFilePicker(rvh);
  if (!file_picker) {
    std::vector<content::FileChooserFileInfo> empty;
    rvh->FilesSelectedInChooser(empty, params.mode);
    return;
  }

  file_picker->Run(params);
}

bool WebView::EmbedsFullscreenWidget() const {
  return true;
}

void WebView::EnterFullscreenModeForTab(content::WebContents* source,
                                        const GURL& origin) {
  DCHECK_VALID_SOURCE_CONTENTS

  fullscreen_requested_ = true;

  if (fullscreen_granted_) {
    // Nothing to do here. Note, RenderFrameHostImpl::OnToggleFullscreen will
    // send the resize message
    return;
  }

  client_->ToggleFullscreenMode(true);
}

void WebView::ExitFullscreenModeForTab(content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  fullscreen_requested_ = false;

  if (!fullscreen_granted_) {
    return;
  }

  client_->ToggleFullscreenMode(false);
}

bool WebView::IsFullscreenForTabOrPending(
    const content::WebContents* source) const {
  DCHECK_VALID_SOURCE_CONTENTS

  return IsFullscreen();
}

void WebView::FindReply(content::WebContents* source,
                        int request_id,
                        int number_of_matches,
                        const gfx::Rect& selection_rect,
                        int active_match_ordinal,
                        bool final_update) {
  DCHECK_VALID_SOURCE_CONTENTS

  FindController::FromWebContents(web_contents_.get())->HandleFindReply(
      request_id, number_of_matches, active_match_ordinal);
}

void WebView::RequestMediaAccessPermission(
    content::WebContents* source,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  DCHECK_VALID_SOURCE_CONTENTS

  MediaCaptureDevicesDispatcher::GetInstance()->RequestMediaAccessPermission(
      request,
      callback);
}

bool WebView::CheckMediaAccessPermission(content::WebContents* source,
                                         const GURL& security_origin,
                                         content::MediaStreamType type) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(type == content::MEDIA_DEVICE_VIDEO_CAPTURE ||
         type == content::MEDIA_DEVICE_AUDIO_CAPTURE);

  TemporarySavedPermissionType permission =
      type == content::MEDIA_DEVICE_VIDEO_CAPTURE ?
        TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_CAMERA :
        TEMPORARY_SAVED_PERMISSION_TYPE_MEDIA_DEVICE_MIC;

  TemporarySavedPermissionStatus status =
      GetBrowserContext()->GetTemporarySavedPermissionContext()
        ->GetPermissionStatus(permission,
                              security_origin,
                              web_contents_->GetLastCommittedURL().GetOrigin());
  return status == TEMPORARY_SAVED_PERMISSION_STATUS_ALLOWED;
}

void WebView::RenderFrameForInterstitialPageCreated(
    content::RenderFrameHost* render_frame_host) {
  if (render_frame_host->GetParent()) {
    return;
  }

  InitializeTopControlsForHost(render_frame_host->GetRenderViewHost(), false);
}

void WebView::RenderViewReady() {
  client_->CrashedStatusChanged();
}

void WebView::RenderProcessGone(base::TerminationStatus status) {
  client_->CrashedStatusChanged();
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
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

  InitializeTopControlsForHost(new_host, !old_host);
}

void WebView::DidStartLoading() {
  client_->LoadingChanged();
}

void WebView::DidStopLoading() {
  client_->LoadingChanged();
}

void WebView::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                            const GURL& validated_url) {
  if (render_frame_host->GetParent()) {
    return;
  }

  if (validated_url.spec() == content::kUnreachableWebDataURL) {
    return;
  }

  content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
  // Some transient about:blank navigations dont have navigation entries.
  client_->LoadSucceeded(
      validated_url,
      entry ? entry->GetHttpStatusCode() : 0);
}

void WebView::DidFailLoad(content::RenderFrameHost* render_frame_host,
                          const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description,
                          bool was_ignored_by_handler) {
  if (render_frame_host->GetParent()) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::DidStartProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    bool is_error_frame,
    bool is_iframe_srcdoc) {
  if (is_error_frame) {
    return;
  }

  if (render_frame_host->GetParent()) {
    return;
  }

  client_->LoadStarted(validated_url);
}

void WebView::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    ui::PageTransition transition_type) {
  if (render_frame_host->GetParent()) {
    return;
  }

  content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
  client_->LoadCommitted(url,
                         entry->GetPageType() == content::PAGE_TYPE_ERROR,
                         entry->GetHttpStatusCode());
}

void WebView::DidFailProvisionalLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description,
    bool was_ignored_by_handler) {
  if (render_frame_host->GetParent()) {
    return;
  }

  if (validated_url.spec() == content::kUnreachableWebDataURL) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description, true);
}

void WebView::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (details.is_navigation_to_different_page()) {
    blocked_content_ = CONTENT_TYPE_NONE;
    client_->ContentBlocked();

    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->ResetGestureDetection();
    }
  }
}

void WebView::DidGetRedirectForResourceRequest(
      content::RenderFrameHost* render_frame_host,
      const content::ResourceRedirectDetails& details) {
  if (details.resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    return;
  }

  client_->LoadRedirected(details.new_url,
                          details.original_url,
                          details.http_response_code);
}

void WebView::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  client_->NavigationEntryCommitted();
}

void WebView::DidShowFullscreenWidget(int routing_id) {
  content::RenderWidgetHost* rwh =
      content::RenderWidgetHost::FromID(
          web_contents_->GetRenderProcessHost()->GetID(),
          routing_id);
  DCHECK(rwh);

  static_cast<RenderWidgetHostView*>(rwh->GetView())->SetContainer(this);

  web_contents_->GetRenderWidgetHostView()->Hide();

  if (!IsFullscreen()) {
    // If the application didn't grant us fullscreen, schedule a task to cancel
    // the fullscreen. We do this as we'll have a fullscreen view that the
    // application can't get rid of.
    // We do this asynchronously to avoid a UAF in
    // WebContentsImpl::ShowCreatedWidget
    // See https://launchpad.net/bugs/1510973
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebView::MaybeCancelFullscreenMode, AsWeakPtr()));
  }
}

void WebView::DidDestroyFullscreenWidget(int routing_id) {
  DCHECK(!web_contents_->GetFullscreenRenderWidgetHostView());

  content::RenderWidgetHostView* orig_rwhv =
      web_contents_->GetRenderWidgetHostView();
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
    static_cast<RenderWidgetHostView*>(orig_rwhv)->Blur();
  }
}

void WebView::DidAttachInterstitialPage() {
  DCHECK(!interstitial_rwh_id_.IsValid());

  content::RenderWidgetHost* rwh =
      web_contents_->GetInterstitialPage()
        ->GetMainFrame()
        ->GetRenderViewHost()
        ->GetWidget();
  static_cast<RenderWidgetHostView*>(rwh->GetView())->SetContainer(this);

  interstitial_rwh_id_ = RenderWidgetHostID(rwh);
}

void WebView::DidDetachInterstitialPage() {
  if (!interstitial_rwh_id_.IsValid()) {
    return;
  }

  content::RenderWidgetHost* rwh = interstitial_rwh_id_.ToInstance();
  interstitial_rwh_id_ = RenderWidgetHostID();
  if (!rwh) {
    return;
  }

  static_cast<RenderWidgetHostView*>(rwh->GetView())->SetContainer(nullptr);
}

void WebView::TitleWasSet(content::NavigationEntry* entry, bool explicit_set) {
  const content::NavigationController& controller = web_contents_->GetController();
  int count = controller.GetEntryCount();
  for (int i = 0; i < count; ++i) {
    if (controller.GetEntryAtIndex(i) == entry) {
      client_->NavigationEntryChanged(i);
      return;
    }
  }
}

bool WebView::OnMessageReceived(const IPC::Message& msg,
                                content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WebView, msg)
    IPC_MESSAGE_HANDLER(OxideHostMsg_DidBlockDisplayingInsecureContent,
                        OnDidBlockDisplayingInsecureContent)
    IPC_MESSAGE_HANDLER(OxideHostMsg_DidBlockRunningInsecureContent,
                        OnDidBlockRunningInsecureContent)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

WebView::WebView(const Params& params)
    : WebView(params.client) {
  CHECK(params.context) << "Didn't specify a BrowserContext";

  scoped_refptr<BrowserContext> context = params.incognito ?
      params.context->GetOffTheRecordContext() :
      params.context->GetOriginalContext();

  content::WebContents::CreateParams content_params(context.get());
  content_params.initial_size = GetViewSizeDip();
  content_params.initially_hidden = !IsVisible();

  scoped_ptr<content::WebContents> contents(
      content::WebContents::Create(content_params));
  CHECK(contents.get()) << "Failed to create WebContents";

  CreateHelpers(contents.get());
  CommonInit(contents.Pass());

  if (params.restore_entries.size() > 0) {
    ScopedVector<content::NavigationEntry> entries =
        sessions::ContentSerializedNavigationBuilder::ToNavigationEntries(
            params.restore_entries, context.get());
    web_contents_->GetController().Restore(
        params.restore_index,
        params.restore_type,
        &entries);
    web_contents_->GetController().LoadIfNecessary();
  }
}

WebView::WebView(scoped_ptr<content::WebContents> contents,
                 WebViewClient* client)
    : WebView(client) {
  CHECK(contents);
  DCHECK(contents->GetBrowserContext()) <<
         "Specified WebContents doesn't have a BrowserContext";
  CHECK(WebViewContentsHelper::FromWebContents(contents.get())) <<
       "Specified WebContents should already have a WebViewContentsHelper";
  CHECK(!FromWebContents(contents.get())) <<
        "Specified WebContents already belongs to a WebView";

  CommonInit(contents.Pass());

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (rvh) {
    InitializeTopControlsForHost(rvh, true);
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetContainer(this);
    rwhv->ime_bridge()->SetContext(client_->GetInputMethodContext());

    rwhv->SetSize(GetViewSizeDip());
    content::RenderWidgetHostImpl::From(GetRenderWidgetHost())
        ->SendScreenRects();
    GetRenderWidgetHost()->WasResized();

    if (HasFocus()) {
      rwhv->Focus();
    } else {
      rwhv->Blur();
    }
  }

  if (IsVisible()) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
  }

  // Update SSL Status
  content::NavigationEntry* entry =
      web_contents_->GetController().GetVisibleEntry();
  if (entry) {
    security_status_.Update(entry->GetSSL());
  }
}

WebView::~WebView() {
  g_all_web_views.Get().erase(
      std::remove(g_all_web_views.Get().begin(),
                  g_all_web_views.Get().end(),
                  this),
      g_all_web_views.Get().end());

  WebContentsView::FromWebContents(web_contents_.get())->SetContainer(nullptr);

  RenderWidgetHostView* rwhv =
      static_cast<RenderWidgetHostView*>(
        web_contents_->GetRenderWidgetHostView());
  if (rwhv) {
    rwhv->SetContainer(nullptr);
    rwhv->ime_bridge()->SetContext(nullptr);
  }

  RenderWidgetHostView* fullscreen_rwhv =
      static_cast<RenderWidgetHostView*>(
        web_contents_->GetFullscreenRenderWidgetHostView());
  if (fullscreen_rwhv) {
    fullscreen_rwhv->SetContainer(nullptr);
  }

  if (interstitial_rwh_id_.IsValid()) {
    content::RenderWidgetHost* rwh = interstitial_rwh_id_.ToInstance();
    if (rwh) {
      static_cast<RenderWidgetHostView*>(
          rwh->GetView())->SetContainer(nullptr);
    }
    interstitial_rwh_id_ = RenderWidgetHostID();
  }

  // Stop WebContents from calling back in to us
  content::WebContentsObserver::Observe(nullptr);

  // It's time we split the WebContentsDelegate implementation from WebView,
  // given that a lot of functionality that is interested in it lives outside
  // now
  web_contents_->SetDelegate(nullptr);

  web_contents_->RemoveUserData(&kUserDataKey);
}

// static
WebView* WebView::FromWebContents(const content::WebContents* web_contents) {
  UnownedUserData<WebView>* data =
      static_cast<UnownedUserData<WebView>*>(
        web_contents->GetUserData(&kUserDataKey));
  if (!data) {
    return nullptr;
  }

  return data->get();
}

// static
WebView* WebView::FromRenderViewHost(content::RenderViewHost* rvh) {
  content::WebContents* contents =
      content::WebContents::FromRenderViewHost(rvh);
  if (!contents) {
    return nullptr;
  }

  return FromWebContents(contents);
}

// static
WebView* WebView::FromRenderFrameHost(content::RenderFrameHost* rfh) {
  content::WebContents* contents =
      content::WebContents::FromRenderFrameHost(rfh);
  if (!contents) {
    return nullptr;
  }

  return FromWebContents(contents);
}

const GURL& WebView::GetURL() const {
  return web_contents_->GetVisibleURL();
}

void WebView::SetURL(const GURL& url) {
  if (!url.is_valid()) {
    return;
  }

  content::NavigationController::LoadURLParams params(url);
  params.transition_type =
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_API;

  web_contents_->GetController().LoadURLWithParams(params);
}

std::vector<sessions::SerializedNavigationEntry> WebView::GetState() const {
  std::vector<sessions::SerializedNavigationEntry> entries;
  const content::NavigationController& controller =
      web_contents_->GetController();
  const int pending_index = controller.GetPendingEntryIndex();
  int entry_count = controller.GetEntryCount();
  if (entry_count == 0 && pending_index == 0) {
    entry_count++;
  }
  entries.resize(entry_count);
  for (int i = 0; i < entry_count; ++i) {
    content::NavigationEntry* entry = (i == pending_index) ?
        controller.GetPendingEntry() : controller.GetEntryAtIndex(i);
    entries[i] =
        sessions::ContentSerializedNavigationBuilder::FromNavigationEntry(
          i, *entry);
  }
  return entries;
}

void WebView::LoadData(const std::string& encoded_data,
                       const std::string& mime_type,
                       const GURL& base_url) {
  std::string url("data:");
  url.append(mime_type);
  url.append(",");
  url.append(encoded_data);

  content::NavigationController::LoadURLParams params((GURL(url)));
  params.transition_type =
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_API;
  params.load_type = content::NavigationController::LOAD_TYPE_DATA;
  params.base_url_for_data_url = base_url;
  params.virtual_url_for_data_url =
      base_url.is_empty() ? GURL(url::kAboutBlankURL) : base_url;
  params.can_load_local_resources = true;

  web_contents_->GetController().LoadURLWithParams(params);
}

std::string WebView::GetTitle() const {
  return base::UTF16ToUTF8(web_contents_->GetTitle());
}

const GURL& WebView::GetFaviconURL() const {
  return FaviconHelper::FromWebContents(web_contents_.get())->GetFaviconURL();
}

bool WebView::CanGoBack() const {
  return web_contents_->GetController().CanGoBack();
}

bool WebView::CanGoForward() const {
  return web_contents_->GetController().CanGoForward();
}

void WebView::GoBack() {
  if (!CanGoBack()) {
    return;
  }

  web_contents_->GetController().GoBack();
}

void WebView::GoForward() {
  if (!CanGoForward()) {
    return;
  }

  web_contents_->GetController().GoForward();
}

void WebView::Stop() {
  web_contents_->Stop();
}

void WebView::Reload() {
  web_contents_->GetController().Reload(true);
}

bool WebView::IsIncognito() const {
  return GetBrowserContext()->IsOffTheRecord();
}

bool WebView::IsLoading() const {
  return web_contents_->IsLoading();
}

bool WebView::FullscreenGranted() const {
  return fullscreen_granted_;
}

void WebView::SetFullscreenGranted(bool fullscreen) {
  if (fullscreen == fullscreen_granted_) {
    return;
  }

  bool was_fullscreen = IsFullscreen();
  // It's important to do this before calling WebContents::ExitFullscreen,
  // as this calls back in to ExitFullscreenModeForTab. If the application
  // calls us synchronously, then we'll run out of stack
  fullscreen_granted_ = fullscreen;
  bool is_fullscreen = IsFullscreen();

  if (is_fullscreen == was_fullscreen) {
    return;
  }

  if (is_fullscreen) {
    content::RenderWidgetHost* host = GetRenderWidgetHost();
    if (host) {
      host->WasResized();
    }
  } else {
    web_contents_->ExitFullscreen();
  }
}

void WebView::WasResized() {
  compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);
  compositor_->SetViewportSize(GetViewSizePix());
  root_layer_->SetBounds(GetViewSizeDip());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(GetViewSizeDip());
    content::RenderWidgetHostImpl::From(GetRenderWidgetHost())
        ->SendScreenRects();
    GetRenderWidgetHost()->WasResized();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::ScreenUpdated() {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  content::RenderWidgetHostImpl::From(host)->NotifyScreenInfoChanged();
}

void WebView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

  if (visible) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
    // TODO: Have an eviction algorithm for LayerTreeHosts in Compositor, and
    //  trigger eviction of the frontbuffer from a CompositorClient callback.
    // XXX: Also this isn't really necessary for eviction - after all, the LTH
    //  owned by Compositor owns the frontbuffer (via its cc::OutputSurface).
    //  This callback is really to notify the toolkit layer that the
    //  frontbuffer is being dropped
    current_compositor_frame_ = nullptr;
    client_->EvictCurrentFrame();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::FocusChanged() {
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

void WebView::UpdateWebPreferences() {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->OnWebkitPreferencesChanged();
}

BrowserContext* WebView::GetBrowserContext() const {
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
}

content::WebContents* WebView::GetWebContents() const {
  return web_contents_.get();
}

int WebView::GetNavigationEntryCount() const {
  return web_contents_->GetController().GetEntryCount();
}

int WebView::GetNavigationCurrentEntryIndex() const {
  return web_contents_->GetController().GetCurrentEntryIndex();
}

void WebView::SetNavigationCurrentEntryIndex(int index) {
  web_contents_->GetController().GoToIndex(index);
}

int WebView::GetNavigationEntryUniqueID(int index) const {
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetUniqueID();
}

const GURL& WebView::GetNavigationEntryUrl(int index) const {
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetURL();
}

std::string WebView::GetNavigationEntryTitle(int index) const {
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return base::UTF16ToUTF8(entry->GetTitle());
}

base::Time WebView::GetNavigationEntryTimestamp(int index) const {
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetTimestamp();
}

WebFrame* WebView::GetRootFrame() const {
  return WebFrameTree::FromWebContents(web_contents_.get())->root_frame();
}

WebPreferences* WebView::GetWebPreferences() {
  return web_contents_helper_->GetWebPreferences();
}

void WebView::SetWebPreferences(WebPreferences* prefs) {
  WebPreferencesObserver::Observe(prefs);
  web_contents_helper_->SetWebPreferences(prefs);
}

gfx::Size WebView::GetViewSizePix() const {
  return GetViewBoundsPix().size();
}

gfx::Rect WebView::GetViewBoundsDip() const {
  float scale = 1.0f / GetScreenInfo().deviceScaleFactor;
  gfx::Rect bounds(GetViewBoundsPix());

  int x = std::lround(bounds.x() * scale);
  int y = std::lround(bounds.y() * scale);
  int width = std::lround(bounds.width() * scale);
  int height = std::lround(bounds.height() * scale);

  return gfx::Rect(x, y, width, height);
}

gfx::Size WebView::GetViewSizeDip() const {
  float scale = 1.0f / GetScreenInfo().deviceScaleFactor;
  gfx::Size size(GetViewSizePix());

  int width = std::lround(size.width() * scale);
  int height = std::lround(size.height() * scale);

  return gfx::Size(width, height);
}

gfx::Point WebView::GetCompositorFrameScrollOffsetPix() {
  // See https://launchpad.net/bugs/1336730
  const gfx::SizeF& viewport_size =
      compositor_frame_metadata().scrollable_viewport_size;
  float x_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.width() / std::round(viewport_size.width());
  float y_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.height() / std::round(viewport_size.height());

  gfx::Vector2dF offset =
      gfx::ScaleVector2d(compositor_frame_metadata().root_scroll_offset,
                         x_scale, y_scale);

  return gfx::Point(std::round(offset.x()), std::round(offset.y()));
}

gfx::Size WebView::GetCompositorFrameContentSizePix() {
  // See https://launchpad.net/bugs/1336730
  const gfx::SizeF& viewport_size =
      compositor_frame_metadata().scrollable_viewport_size;
  float x_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.width() / std::round(viewport_size.width());
  float y_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.height() / std::round(viewport_size.height());

  gfx::SizeF size =
      gfx::ScaleSize(compositor_frame_metadata().root_layer_size,
                     x_scale, y_scale);

  return gfx::Size(std::round(size.width()), std::round(size.height()));
}

gfx::Size WebView::GetCompositorFrameViewportSizePix() {
  gfx::SizeF size =
      gfx::ScaleSize(compositor_frame_metadata().scrollable_viewport_size,
                     GetFrameMetadataScaleToPix());

  return gfx::Size(std::round(size.width()), std::round(size.height()));
}

int WebView::GetLocationBarOffsetPix() {
  return compositor_frame_metadata().location_bar_offset.y() *
         compositor_frame_metadata().device_scale_factor;
}

int WebView::GetLocationBarContentOffsetPix() {
  return compositor_frame_metadata().location_bar_content_translation.y() *
         compositor_frame_metadata().device_scale_factor;
}

float WebView::GetLocationBarContentOffsetDip() {
  return compositor_frame_metadata().location_bar_content_translation.y();
}

float WebView::GetLocationBarHeightDip() const {
  return GetLocationBarHeightPix() / GetScreenInfo().deviceScaleFactor;
}

int WebView::GetLocationBarHeightPix() const {
  return location_bar_height_pix_;
}

void WebView::SetLocationBarHeightPix(int height) {
  DCHECK_GE(height, 0);

  if (height == location_bar_height_pix_) {
    return;
  }

  location_bar_height_pix_ = height;

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->WasResized();
}

void WebView::SetLocationBarConstraints(blink::WebTopControlsState constraints) {
  if (constraints == location_bar_constraints_) {
    return;
  }

  location_bar_constraints_ = constraints;

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsBoth,
                                                location_bar_animated_));
}

void WebView::ShowLocationBar(bool animate) {
  DCHECK_EQ(location_bar_constraints_, blink::WebTopControlsBoth);

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsShown,
                                                animate));
}

void WebView::HideLocationBar(bool animate) {
  DCHECK_EQ(location_bar_constraints_, blink::WebTopControlsBoth);

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsHidden,
                                                animate));
}

void WebView::SetCanTemporarilyDisplayInsecureContent(bool allow) {
  if (!(blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) && allow) {
    return;
  }

  if (allow) {
    blocked_content_ &= ~CONTENT_TYPE_MIXED_DISPLAY;
    client_->ContentBlocked();
  }

  web_contents_->SendToAllFrames(
      new OxideMsg_SetAllowDisplayingInsecureContent(MSG_ROUTING_NONE, allow));
  web_contents_->GetMainFrame()->Send(
      new OxideMsg_ReloadFrame(web_contents_->GetMainFrame()->GetRoutingID()));
}

void WebView::SetCanTemporarilyRunInsecureContent(bool allow) {
  if (!web_contents_) {
    return;
  }

  if (!(blocked_content_ & CONTENT_TYPE_MIXED_SCRIPT) && allow) {
    return;
  }

  if (allow) {
    blocked_content_ &= ~CONTENT_TYPE_MIXED_DISPLAY;
    blocked_content_ &= ~CONTENT_TYPE_MIXED_SCRIPT;
    client_->ContentBlocked();
  }

  web_contents_->SendToAllFrames(
      new OxideMsg_SetAllowRunningInsecureContent(MSG_ROUTING_NONE, allow));
  web_contents_->GetMainFrame()->Send(
      new OxideMsg_ReloadFrame(web_contents_->GetMainFrame()->GetRoutingID()));
}

void WebView::PrepareToClose() {
  base::Closure no_before_unload_handler_response_task =
      base::Bind(&WebView::DispatchPrepareToCloseResponse,
                 AsWeakPtr(), true);

  if (!web_contents_->NeedToFireBeforeUnload()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebView::DispatchPrepareToCloseResponse,
                   AsWeakPtr(), true));
    return;
  }

  // This is ok to call multiple times - RFHI tracks whether a response
  // is pending and won't dispatch another event if it is
  web_contents_->DispatchBeforeUnload(false);
}

void WebView::HandleKeyEvent(const content::NativeWebKeyboardEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardKeyboardEvent(event);
}

void WebView::HandleMouseEvent(const blink::WebMouseEvent& event) {
  blink::WebMouseEvent e(event);

  if (e.type == blink::WebInputEvent::MouseEnter ||
      e.type == blink::WebInputEvent::MouseLeave) {
    global_mouse_position_.SetPoint(event.globalX, event.globalY);
    e.type = blink::WebInputEvent::MouseMove;
  }

  e.movementX = e.globalX - global_mouse_position_.x();
  e.movementY = e.globalY - global_mouse_position_.y();

  global_mouse_position_.SetPoint(e.globalX, e.globalY);

  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardMouseEvent(e);
}

void WebView::HandleTouchEvent(const ui::TouchEvent& event) {
  if (!touch_state_.Update(event)) {
    return;
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->HandleTouchEvent(touch_state_);
}

void WebView::HandleWheelEvent(const blink::WebMouseWheelEvent& event) {
  content::RenderWidgetHost* host = GetRenderWidgetHost();
  if (!host) {
    return;
  }

  host->ForwardWheelEvent(event);
}

void WebView::DownloadRequested(
    const GURL& url,
    const std::string& mime_type,
    const bool should_prompt,
    const base::string16& suggested_filename,
    const std::string& cookies,
    const std::string& referrer,
    const std::string& user_agent) {
  client_->DownloadRequested(url,
                             mime_type,
                             should_prompt,
                             suggested_filename,
                             cookies,
                             referrer,
                             user_agent);
}

void WebView::HttpAuthenticationRequested(
    ResourceDispatcherHostLoginDelegate* login_delegate) {
  client_->HttpAuthenticationRequested(login_delegate);
}

CompositorFrameHandle* WebView::GetCompositorFrameHandle() const {
  return current_compositor_frame_.get();
}

void WebView::DidCommitCompositorFrame() {
  DCHECK(!received_surface_ids_.empty());

  while (!received_surface_ids_.empty()) {
    uint32 surface_id = received_surface_ids_.front();
    received_surface_ids_.pop();

    compositor_->DidSwapCompositorFrame(
        surface_id,
        std::move(previous_compositor_frames_));
  }
}

blink::WebScreenInfo WebView::GetScreenInfo() const {
  return client_->GetScreenInfo();
}

gfx::Rect WebView::GetViewBoundsPix() const {
  if (IsFullscreen()) {
    // If we're in fullscreen mode, return the screen size rather than the
    // view bounds. This works around an issue where buggy Flash content
    // expects the view to resize synchronously when it goes fullscreen, but it
    // happens asynchronously instead.
    // See https://launchpad.net/bugs/1510508
    // XXX: Obviously, this means we assume that we do occupy the full screen
    //  when the browser grants us fullscreen. If that's not the case, then
    //  this is going to break
    return GetScreenInfo().rect;
  }
  return client_->GetViewBoundsPix();
}

bool WebView::IsVisible() const {
  return client_->IsVisible();
}

bool WebView::HasFocus() const {
  return client_->HasFocus();
}

JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type) {
  return client_->CreateJavaScriptDialog(javascript_message_type);
}

JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  return client_->CreateBeforeUnloadDialog();
}

bool WebView::ShouldHandleNavigation(const GURL& url, bool has_user_gesture) {
  if (web_contents_->GetController().IsInitialNavigation()) {
    return true;
  }

  return client_->ShouldHandleNavigation(url,
                                         CURRENT_TAB,
                                         has_user_gesture);
}

bool WebView::CanCreateWindows() const {
  return client_->CanCreateWindows();
}

} // namespace oxide

