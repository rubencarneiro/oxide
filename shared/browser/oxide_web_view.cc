// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "base/auto_reset.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/supports_user_data.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/menu_item.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"
#include "ipc/ipc_message_macros.h"
#include "net/base/net_errors.h"
#include "net/ssl/ssl_info.h"
#include "third_party/WebKit/public/platform/WebGestureDevice.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/window_open_disposition.h"
#include "ui/events/event.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/gfx/range/range.h"
#include "ui/gl/gl_implementation.h"
#include "url/gurl.h"
#include "url/url_constants.h"

#include "shared/base/oxide_enum_flags.h"
#include "shared/base/oxide_event_utils.h"
#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_file_picker.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_frame.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view_contents_helper.h"

namespace oxide {

namespace {

// The time between the last resize, focus, view visibility or
// input panel visibility event before we scroll the currently focused
// editable node into view. We want to wait until any transitions
// are finished because we will only do the scroll once
const int64 kAutoScrollFocusedEditableNodeIntoViewDelayMs = 50;

const float kMobileViewportWidthEpsilon = 0.15f;
const char kWebViewKey[] = "oxide_web_view_data";

// SupportsUserData implementations own their data. This class exists
// because we don't want WebContents to own WebView (it's the other way
// around)
class WebViewUserData : public base::SupportsUserData::Data {
 public:
  WebViewUserData(WebView* view) : view_(view) {}

  WebView* view() const { return view_; }

 private:
  WebView* view_;
};

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

void InitCreatedWebView(WebView* view, ScopedNewContentsHolder contents) {
  WebView::Params params;
  params.contents = contents.Pass();

  view->Init(&params);
}

bool ShouldSendPinchGesture() {
  static bool pinch_allowed =
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnableViewport) ||
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kEnablePinch);
  return pinch_allowed;
}

bool ShouldUseSoftwareCompositing() {
  static bool initialized = false;
  static bool result = true;

  if (initialized) {
    return result;
  }

  initialized = true;

  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return true;
  }

  SharedGLContext* share_context =
      BrowserProcessMain::GetInstance()->GetSharedGLContext();
  if (!share_context) {
    return true;
  }

  if (share_context->GetImplementation() != gfx::GetGLImplementation()) {
    return true;
  }

  result = false;
  return false;
}

// Qt input methods don’t generate key events, but a lot of web pages out there
// rely on keydown and keyup events to e.g. perform search-as-you-type or
// enable/disable a submit button based on the contents of a text input field,
// so we send a fake pair of keydown/keyup events.
// This mimicks what is done in GtkIMContextWrapper::HandlePreeditChanged(…)
// and GtkIMContextWrapper::HandleCommit(…)
// (see content/browser/renderer_host/gtk_im_context_wrapper.cc).
void SendFakeCompositionKeyEvent(content::RenderWidgetHostImpl* host,
                                 blink::WebInputEvent::Type type) {
  content::NativeWebKeyboardEvent fake_event;
  fake_event.windowsKeyCode = ui::VKEY_PROCESSKEY;
  fake_event.skip_in_browser = true;
  fake_event.type = type;
  host->ForwardKeyboardEvent(fake_event);
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

CertError ToCertError(int error, net::X509Certificate* cert) {
  if (!net::IsCertificateError(error)) {
    return CERT_OK;
  }

  if (error == net::ERR_CERT_NO_REVOCATION_MECHANISM ||
      error == net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION) {
    // These aren't treated as hard errors
    return CERT_OK;
  }

  switch (error) {
    case net::ERR_CERT_COMMON_NAME_INVALID:
      return CERT_ERROR_BAD_IDENTITY;
    case net::ERR_CERT_DATE_INVALID: {
      if (cert && cert->HasExpired()) {
        return CERT_ERROR_EXPIRED;
      }
      return CERT_ERROR_DATE_INVALID;
    }
    case net::ERR_CERT_AUTHORITY_INVALID:
      return CERT_ERROR_AUTHORITY_INVALID;
    case net::ERR_CERT_CONTAINS_ERRORS:
    case net::ERR_CERT_INVALID:
      return CERT_ERROR_INVALID;
    case net::ERR_CERT_REVOKED:
      return CERT_ERROR_REVOKED;
    case net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
    case net::ERR_CERT_WEAK_KEY:
      return CERT_ERROR_INSECURE;
    //case net::ERR_CERT_NON_UNIQUE_NAME:
    //case net::ERR_CERT_NAME_CONSTRAINT_VIOLATION:
    default:
      return CERT_ERROR_GENERIC;
  }
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(ContentType)

typedef std::map<BrowserContext*, std::set<WebView*> > WebViewsPerContextMap;
base::LazyInstance<WebViewsPerContextMap> g_web_view_per_context;

}

// static
std::set<WebView*>
WebView::GetAllWebViewsFor(BrowserContext * browser_context) {
  std::set<WebView*> webviews;
  if (!browser_context) {
    return webviews;
  }
  WebViewsPerContextMap::iterator it;
  for (it = g_web_view_per_context.Get().begin();
       it != g_web_view_per_context.Get().end();
       ++it) {
    if (browser_context->IsSameContext(it->first)) {
      return it->second;
    }
  }
  return webviews;
}

RenderWidgetHostView* WebView::GetRenderWidgetHostView() const {
  if (!web_contents_) {
    return NULL;
  }

  return static_cast<RenderWidgetHostView *>(
      web_contents_->GetRenderWidgetHostView());
}

content::RenderViewHost* WebView::GetRenderViewHost() const {
  if (!web_contents_) {
    return NULL;
  }

  return web_contents_->GetRenderViewHost();
}

content::RenderWidgetHostImpl* WebView::GetRenderWidgetHostImpl() const {
  if (!web_contents_) {
    return NULL;
  }

  return content::RenderWidgetHostImpl::From(
      web_contents_->GetRenderViewHost());
}

void WebView::DispatchLoadFailed(const GURL& validated_url,
                                 int error_code,
                                 const base::string16& error_description) {
  if (error_code == net::ERR_ABORTED) {
    OnLoadStopped(validated_url);
  } else {
    OnLoadFailed(validated_url, error_code,
                 base::UTF16ToUTF8(error_description));
  }
}

void WebView::OnDidBlockDisplayingInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_DISPLAY;

  OnContentBlocked();
}

void WebView::OnDidBlockRunningInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_SCRIPT) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_SCRIPT;

  OnContentBlocked();
}

bool WebView::ShouldScrollFocusedEditableNodeIntoView() {
  if (did_scroll_focused_editable_node_into_view_) {
    return false;
  }

  if (!HasFocus()) {
    return false;
  }

  if (!IsVisible()) {
    return false;
  }

  if (!IsInputPanelVisible()) {
    return false;
  }

  if (!focused_node_is_editable_) {
    return false;
  }

  return true;
}

void WebView::MaybeResetAutoScrollTimer() {
  if (!ShouldScrollFocusedEditableNodeIntoView()) {
    auto_scroll_timer_.Stop();
    return;
  }

  if (auto_scroll_timer_.IsRunning()) {
    auto_scroll_timer_.Reset();
    return;
  }

  auto_scroll_timer_.Start(
      FROM_HERE,
      base::TimeDelta::FromMilliseconds(
        kAutoScrollFocusedEditableNodeIntoViewDelayMs),
      base::Bind(&WebView::ScrollFocusedEditableNodeIntoView,
                 base::Unretained(this)));
}

void WebView::ScrollFocusedEditableNodeIntoView() {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  did_scroll_focused_editable_node_into_view_ = true;

  host->ScrollFocusedEditableNodeIntoRect(GetViewBoundsDip());
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return 0;
}

ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(size_t index) const {
  return NULL;
}

void WebView::CompositorDidCommit() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->CompositorDidCommit();
}

void WebView::CompositorSwapFrame(uint32 surface_id,
                                  CompositorFrameHandle* frame) {
  received_surface_ids_.push(surface_id);

  if (current_compositor_frame_.get()) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = frame;

  OnSwapCompositorFrame();
}

void WebView::WebPreferencesDestroyed() {
  initial_preferences_ = NULL;
  OnWebPreferencesDestroyed();
}

void WebView::OnGestureEvent(const blink::WebGestureEvent& event) {
  if (in_swap_) {
    return;
  }

  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  if ((event.type == blink::WebInputEvent::GesturePinchBegin ||
       event.type == blink::WebInputEvent::GesturePinchUpdate ||
       event.type == blink::WebInputEvent::GesturePinchEnd) &&
      !ShouldSendPinchGesture()) {
    return;
  }

  if (event.type == blink::WebInputEvent::GestureTapDown) {
    // Webkit does not stop a fling-scroll on tap-down. So explicitly send an
    // event to stop any in-progress flings.
    blink::WebGestureEvent fling_cancel = event;
    fling_cancel.type = blink::WebInputEvent::GestureFlingCancel;
    fling_cancel.sourceDevice = blink::WebGestureDeviceTouchpad;
    host->ForwardGestureEvent(fling_cancel);
  }

  if (event.type == blink::WebInputEvent::Undefined) {
    return;
  }

  host->ForwardGestureEvent(event);
}

void WebView::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (content::Source<content::NavigationController>(source).ptr() !=
      &GetWebContents()->GetController()) {
    return;
  }
  if (type == content::NOTIFICATION_NAV_LIST_PRUNED) {
    content::PrunedDetails* pruned_details = content::Details<content::PrunedDetails>(details).ptr();
    OnNavigationListPruned(pruned_details->from_front, pruned_details->count);
  } else if (type == content::NOTIFICATION_NAV_ENTRY_CHANGED) {
    int index = content::Details<content::EntryChangedDetails>(details).ptr()->index;
    OnNavigationEntryChanged(index);
  }
}

content::WebContents* WebView::OpenURL(const content::OpenURLParams& params) {
  if (params.disposition != CURRENT_TAB &&
      params.disposition != NEW_FOREGROUND_TAB &&
      params.disposition != NEW_BACKGROUND_TAB &&
      params.disposition != NEW_POPUP &&
      params.disposition != NEW_WINDOW) {
    return NULL;
  }

  // Block popups
  if ((params.disposition == NEW_FOREGROUND_TAB ||
       params.disposition == NEW_BACKGROUND_TAB ||
       params.disposition == NEW_WINDOW ||
       params.disposition == NEW_POPUP) &&
      !params.user_gesture && GetBrowserContext()->IsPopupBlockerEnabled()) {
    return NULL;
  }

  // Without --site-per-process, frame_tree_node_id is always -1. That's ok,
  // because we only get called for top-level frames anyway. With
  // --site-per-process, we might get called for subframes that are the
  // toplevel within their renderer process, so we use the frame ID (which
  // won't be -1) to look up its corresponding WebFrame
  bool top_level = params.frame_tree_node_id == -1;
  if (!top_level) {
    WebFrame* frame = WebFrame::FromFrameTreeNodeID(params.frame_tree_node_id);
    DCHECK(frame);
    top_level = frame->parent() == NULL;
  }

  WindowOpenDisposition disposition = params.disposition;
  content::OpenURLParams local_params(params);

  // Coerce all non CURRENT_TAB navigations that don't come from a user
  // gesture to NEW_POPUP
  if (disposition != CURRENT_TAB && !local_params.user_gesture) {
    disposition = NEW_POPUP;
  }

  // If we can't create new windows, this should be a CURRENT_TAB navigation
  // in the top-level frame
  if (!CanCreateWindows() && disposition != CURRENT_TAB) {
    disposition = CURRENT_TAB;
    if (!top_level) {
      local_params.frame_tree_node_id = GetFrameTree()->root()->frame_tree_node_id();
      top_level = true;
    }
  }

  // Give the application a chance to block the navigation if it is
  // renderer initiated and it's a top-level navigation or requires a
  // new webview
  if (local_params.is_renderer_initiated &&
      (top_level || disposition != CURRENT_TAB) &&
      !ShouldHandleNavigation(local_params.url,
                              disposition,
                              local_params.user_gesture)) {
    return NULL;
  }

  if (disposition == CURRENT_TAB) {
    content::NavigationController::LoadURLParams load_params(local_params.url);
    FillLoadURLParamsFromOpenURLParams(&load_params, local_params);

    web_contents_->GetController().LoadURLWithParams(load_params);

    return web_contents_.get();
  }

  // XXX(chrisccoulson): Is there a way to tell when the opener shouldn't
  // be suppressed?
  bool opener_suppressed = true;

  content::WebContents::CreateParams contents_params(
      GetBrowserContext(),
      opener_suppressed ? NULL : web_contents_->GetSiteInstance());
  contents_params.initial_size = GetViewSizeDip();
  contents_params.initially_hidden = disposition == NEW_BACKGROUND_TAB;
  contents_params.opener = opener_suppressed ? NULL : web_contents_.get();

  ScopedNewContentsHolder contents(
      content::WebContents::Create(contents_params));
  if (!contents) {
    LOG(ERROR) << "Failed to create new WebContents for navigation";
    return NULL;
  }

  WebViewContentsHelper::Attach(contents.get(), web_contents_.get());

  WebView* new_view = CreateNewWebView(GetViewBoundsPix(), disposition);
  if (!new_view) {
    return NULL;
  }

  InitCreatedWebView(new_view, contents.Pass());

  content::NavigationController::LoadURLParams load_params(local_params.url);
  FillLoadURLParamsFromOpenURLParams(&load_params, local_params);

  new_view->GetWebContents()->GetController().LoadURLWithParams(load_params);

  return new_view->GetWebContents();
}

void WebView::NavigationStateChanged(content::InvalidateTypes changed_flags) {
  if (changed_flags & content::INVALIDATE_TYPE_URL) {
    OnURLChanged();
  }

  if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
    OnTitleChanged();
  }

  if (changed_flags & (content::INVALIDATE_TYPE_URL |
                       content::INVALIDATE_TYPE_LOAD)) {
    OnCommandsUpdated();
  }
}

void WebView::SSLStateChanged() {
  DCHECK(web_contents_);

  content::NavigationEntry* entry =
      web_contents_->GetController().GetVisibleEntry();
  if (!entry) {
    return;
  }

  SecurityStatus old_status = security_status_;
  security_status_.Update(entry->GetSSL());

  OnSecurityStatusChanged(old_status);
}

bool WebView::ShouldCreateWebContents(const GURL& target_url,
                                      WindowOpenDisposition disposition,
                                      bool user_gesture) {
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

  return ShouldHandleNavigation(target_url,
                                user_gesture ? disposition : NEW_POPUP,
                                user_gesture);
}

bool WebView::CreateNewViewAndAdoptWebContents(
    ScopedNewContentsHolder contents,
    WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos) {
  WebView* new_view = CreateNewWebView(initial_pos, disposition);
  if (!new_view) {
    return false;
  }

  InitCreatedWebView(new_view, contents.Pass());

  return true;
}

void WebView::LoadProgressChanged(double progress) {
  OnLoadProgressChanged(progress);
}

void WebView::AddMessageToConsole(int32 level,
                                  const base::string16& message,
                                  int32 line_no,
                                  const base::string16& source_id) {
  OnAddMessageToConsole(level, message, line_no, source_id);
}

bool WebView::RunFileChooser(const content::FileChooserParams& params) {
  DCHECK(!active_file_picker_);
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  FilePicker* filePicker = CreateFilePicker(rvh);
  if (!filePicker) {
    return false;
  }
  active_file_picker_ = filePicker->AsWeakPtr();
  active_file_picker_->Run(params);

  return true;
}

void WebView::ToggleFullscreenMode(bool enter) {
  OnToggleFullscreenMode(enter);
}

void WebView::NotifyWebPreferencesDestroyed() {
  OnWebPreferencesDestroyed();
}

void WebView::HandleUnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  OnUnhandledKeyboardEvent(event);
}

void WebView::RenderFrameCreated(content::RenderFrameHost* render_frame_host) {
  if (!root_frame_) {
    return;
  }

  if (WebFrame::FromRenderFrameHost(render_frame_host)) {
    // We get called whenever a new RFH is created for this node
    return;
  }

  WebFrame* parent =
      WebFrame::FromRenderFrameHost(render_frame_host->GetParent());
  DCHECK(parent);

  content::FrameTreeNode* node = static_cast<content::RenderFrameHostImpl *>(
      render_frame_host)->frame_tree_node();
  DCHECK(node);

  WebFrame* frame = CreateWebFrame(node);
  DCHECK(frame);
  frame->SetParent(parent);
}

void WebView::RenderProcessGone(base::TerminationStatus status) {
  permission_request_manager_.CancelAllPendingRequests();
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  DCHECK(!in_swap_);
  base::AutoReset<bool> in_swap(&in_swap_, true);

  while (root_frame_->ChildCount() > 0) {
    root_frame_->ChildAt(0)->Destroy();
  }

  // Fake a response for any pending touch ACK's
  gesture_provider_->OnTouchEventAck(false);
  gesture_provider_->SetDoubleTapSupportForPageEnabled(false);

  if (old_host && old_host->GetView()) {
    static_cast<RenderWidgetHostView *>(old_host->GetView())->SetWebView(NULL);
  }
  if (new_host && new_host->GetView()) {
    static_cast<RenderWidgetHostView *>(new_host->GetView())->SetWebView(this);
  }
}

void WebView::DidStartProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    bool is_error_frame,
    bool is_iframe_srcdoc) {
  if (render_frame_host->GetParent()) {
    return;
  }

  OnLoadStarted(validated_url, is_error_frame);
}

void WebView::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    content::PageTransition transition_type) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (frame) {
    frame->URLChanged();
  }

  OnLoadCommitted(url);
}

void WebView::DidFailProvisionalLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description) {
  if (render_frame_host->GetParent()) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (details.is_navigation_to_different_page()) {
    permission_request_manager_.CancelAllPendingRequests();

    blocked_content_ = CONTENT_TYPE_NONE;
    OnContentBlocked();
  }
}

void WebView::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                            const GURL& validated_url) {
  if (render_frame_host->GetParent()) {
    return;
  }

  OnLoadSucceeded(validated_url);
}

void WebView::DidFailLoad(content::RenderFrameHost* render_frame_host,
                          const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description) {
  if (render_frame_host->GetParent()) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  OnNavigationEntryCommitted();
}

void WebView::DidStartLoading(content::RenderViewHost* render_view_host) {
  OnLoadingChanged();
}

void WebView::DidStopLoading(content::RenderViewHost* render_view_host) {
  OnLoadingChanged();
}

void WebView::FrameDetached(content::RenderFrameHost* render_frame_host) {
  if (!root_frame_) {
    return;
  }

  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  DCHECK(frame);
  frame->Destroy();
}

void WebView::TitleWasSet(content::NavigationEntry* entry, bool explicit_set) {
  if (!web_contents_) {
    return;
  }
  const content::NavigationController& controller = web_contents_->GetController();
  int count = controller.GetEntryCount();
  for (int i = 0; i < count; ++i) {
    if (controller.GetEntryAtIndex(i) == entry) {
      OnNavigationEntryChanged(i);
      return;
    }
  }
}

void WebView::DidUpdateFaviconURL(
    const std::vector<content::FaviconURL>& candidates) {
  std::vector<content::FaviconURL>::const_iterator it;
  for (it = candidates.begin(); it != candidates.end(); ++it) {
    if (it->icon_type == content::FaviconURL::FAVICON) {
      OnIconChanged(it->icon_url);
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

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnIconChanged(const GURL& icon) {}
void WebView::OnCommandsUpdated() {}

void WebView::OnLoadingChanged() {}
void WebView::OnLoadProgressChanged(double progress) {}

void WebView::OnLoadStarted(const GURL& validated_url,
                            bool is_error_frame) {}
void WebView::OnLoadCommitted(const GURL& url) {}
void WebView::OnLoadStopped(const GURL& validated_url) {}
void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {}
void WebView::OnLoadSucceeded(const GURL& validated_url) {}

void WebView::OnNavigationEntryCommitted() {}
void WebView::OnNavigationListPruned(bool from_front, int count) {}
void WebView::OnNavigationEntryChanged(int index) {}

bool WebView::OnAddMessageToConsole(int32 level,
                                    const base::string16& message,
                                    int32 line_no,
                                    const base::string16& source_id) {
  return false;
}

void WebView::OnToggleFullscreenMode(bool enter) {}

void WebView::OnWebPreferencesDestroyed() {}

void WebView::OnRequestGeolocationPermission(
    const GURL& origin,
    const GURL& embedder,
    scoped_ptr<SimplePermissionRequest> request) {}

void WebView::OnUnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {}

void WebView::OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) {}

void WebView::OnDownloadRequested(const GURL& url,
				  const std::string& mimeType,
				  const bool shouldPrompt,
				  const base::string16& suggestedFilename,
				  const std::string& cookies,
				  const std::string& referrer) {}

bool WebView::ShouldHandleNavigation(const GURL& url,
                                     WindowOpenDisposition disposition,
                                     bool user_gesture) {
  return true;
}

WebPopupMenu* WebView::CreatePopupMenu(content::RenderFrameHost* rfh) {
  return NULL;
}

WebView* WebView::CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) {
  NOTREACHED() <<
      "Your CanCreateWindows() implementation should be returning false!";
  return NULL;
}

FilePicker* WebView::CreateFilePicker(content::RenderViewHost* rvh) {
  return NULL;
}

void WebView::OnEvictCurrentFrame() {}

void WebView::OnTextInputStateChanged() {}
void WebView::OnFocusedNodeChanged() {}
void WebView::OnSelectionBoundsChanged() {}

void WebView::OnSecurityStatusChanged(const SecurityStatus& old) {}
bool WebView::OnCertificateError(
    bool is_main_frame,
    CertError cert_error,
    const scoped_refptr<net::X509Certificate>& cert,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool strict_enforcement,
    scoped_ptr<SimplePermissionRequest> request) {
  permission_request_manager_.AbortPendingRequest(request.get());
  return false;
}
void WebView::OnContentBlocked() {}

WebView::WebView()
    : text_input_type_(ui::TEXT_INPUT_TYPE_NONE),
      show_ime_if_needed_(false),
      focused_node_is_editable_(false),
      selection_cursor_position_(0),
      selection_anchor_position_(0),
      web_contents_helper_(NULL),
      compositor_(Compositor::Create(this, ShouldUseSoftwareCompositing())),
      gesture_provider_(GestureProvider::Create(this)),
      in_swap_(false),
      initial_preferences_(NULL),
      root_frame_(NULL),
      is_fullscreen_(false),
      blocked_content_(CONTENT_TYPE_NONE),
      did_scroll_focused_editable_node_into_view_(false),
      auto_scroll_timer_(false, false) {
  gesture_provider_->SetDoubleTapSupportForPageEnabled(false);
}

base::string16 WebView::GetSelectedText() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return base::string16();
  }

  return rwhv->GetSelectedText();
}

const base::string16& WebView::GetSelectionText() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return base::EmptyString16();
  }

  return rwhv->selection_text();
}

WebView::~WebView() {
  permission_request_manager_.CancelAllPendingRequests();

  BrowserContext* context = GetBrowserContext();
  WebViewsPerContextMap::iterator it =
      g_web_view_per_context.Get().find(context);
  if (it != g_web_view_per_context.Get().end()) {
    std::set<WebView*>& wvl = it->second;
    if (wvl.find(this) != wvl.end()) {
      wvl.erase(this);
      g_web_view_per_context.Get()[context] = wvl;
    }
    if (g_web_view_per_context.Get()[context].empty()) {
      g_web_view_per_context.Get().erase(context);
    }
  }

  if (root_frame_) {
    root_frame_->Destroy();
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetWebView(NULL);
  }

  initial_preferences_ = NULL;
  web_contents_helper_ = NULL;
}

void WebView::Init(Params* params) {
  CHECK(!web_contents_) << "Cannot initialize webview more than once";

  CompositorLock lock(compositor_.get());

  if (params->contents) {
    CHECK(!params->context) <<
        "Shouldn't specify a BrowserContext and WebContents at initialization";
    CHECK(params->contents->GetBrowserContext()) <<
        "Specified WebContents doesn't have a BrowserContext";
    CHECK(WebViewContentsHelper::FromWebContents(params->contents.get())) <<
        "Specified WebContents should already have a WebViewContentsHelper";
    CHECK(!FromWebContents(params->contents.get())) <<
        "Specified WebContents already belongs to a WebView";

    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        params->contents.release()));

    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->SetWebView(this);
    }

    // Sync WebContents with the state of the WebView
    WasResized();
    VisibilityChanged();
    FocusChanged();
    InputPanelVisibilityChanged();
    UpdateWebPreferences();

    // Update SSL Status
    content::NavigationEntry* entry =
        web_contents_->GetController().GetVisibleEntry();
    if (entry) {
      security_status_.Update(entry->GetSSL());
    }
  } else {
    CHECK(params->context) << "Didn't specify a BrowserContext or WebContents";

    BrowserContext* context = params->incognito ?
        params->context->GetOffTheRecordContext() :
        params->context->GetOriginalContext();

    content::WebContents::CreateParams content_params(context);
    content_params.initial_size = GetViewSizeDip();
    content_params.initially_hidden = !IsVisible();
    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        content::WebContents::Create(content_params)));
    CHECK(web_contents_.get()) << "Failed to create WebContents";

    WebViewContentsHelper::Attach(web_contents_.get());

    compositor_->SetViewportSize(GetViewSizePix());
    compositor_->SetVisibility(IsVisible());
  }

  compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);

  web_contents_helper_ =
      WebViewContentsHelper::FromWebContents(web_contents_.get());
  web_contents_helper_->SetDelegate(this);

  web_contents_->SetUserData(kWebViewKey, new WebViewUserData(this));

  WebContentsObserver::Observe(web_contents_.get());

  registrar_.Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                 content::NotificationService::AllBrowserContextsAndSources());
  registrar_.Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                 content::NotificationService::AllBrowserContextsAndSources());

  root_frame_ = CreateWebFrame(web_contents_->GetFrameTree()->root());

  if (initial_preferences_) {
    SetWebPreferences(initial_preferences_);
    WebPreferencesObserver::Observe(NULL);
    initial_preferences_ = NULL;
  }
  if (params->context) {
    if (!initial_url_.is_empty()) {
      SetURL(initial_url_);
      initial_url_ = GURL();
    } else if (initial_data_) {
      web_contents_->GetController().LoadURLWithParams(*initial_data_);
      initial_data_.reset();
    }
  }

  SetIsFullscreen(is_fullscreen_);

  {
    BrowserContext* context = GetBrowserContext()->GetOriginalContext();
    WebViewsPerContextMap::iterator it =
        g_web_view_per_context.Get().find(context);
    if (it != g_web_view_per_context.Get().end()) {
      g_web_view_per_context.Get()[context].insert(this);
    } else {
      std::set<WebView*> wvl;
      wvl.insert(this);
      g_web_view_per_context.Get()[context] = wvl;
    }
  }
}

// static
WebView* WebView::FromWebContents(const content::WebContents* web_contents) {
  WebViewUserData* data = static_cast<WebViewUserData *>(
      web_contents->GetUserData(kWebViewKey));
  if (!data) {
    return NULL;
  }

  return data->view();
}

// static
WebView* WebView::FromRenderViewHost(content::RenderViewHost* rvh) {
  return FromWebContents(content::WebContents::FromRenderViewHost(rvh));
}

// static
WebView* WebView::FromRenderFrameHost(content::RenderFrameHost* rfh) {
  return FromWebContents(content::WebContents::FromRenderFrameHost(rfh));
}

const GURL& WebView::GetURL() const {
  if (!web_contents_) {
    return initial_url_;
  }
  return web_contents_->GetVisibleURL();
}

void WebView::SetURL(const GURL& url) {
  if (!url.is_valid()) {
    return;
  }

  if (!web_contents_) {
    initial_url_ = url;
    initial_data_.reset();
    return;
  }

  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PAGE_TRANSITION_TYPED;
  web_contents_->GetController().LoadURLWithParams(params);
}

void WebView::LoadData(const std::string& encodedData,
                       const std::string& mimeType,
                       const GURL& baseUrl) {
  std::string url("data:");
  url.append(mimeType);
  url.append(",");
  url.append(encodedData);

  content::NavigationController::LoadURLParams params((GURL(url)));
  params.load_type = content::NavigationController::LOAD_TYPE_DATA;
  params.base_url_for_data_url = baseUrl;
  params.virtual_url_for_data_url = baseUrl.is_empty() ? GURL(url::kAboutBlankURL) : baseUrl;
  params.can_load_local_resources = true;
  
  if (web_contents_) {
    web_contents_->GetController().LoadURLWithParams(params);
  } else {
    initial_data_.reset(new content::NavigationController::LoadURLParams(params));
    initial_url_ = GURL();
  }
}

std::string WebView::GetTitle() const {
  if (!web_contents_) {
    return std::string();
  }
  return base::UTF16ToUTF8(web_contents_->GetTitle());
}

bool WebView::CanGoBack() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->GetController().CanGoBack();
}

bool WebView::CanGoForward() const {
  if (!web_contents_) {
    return false;
  }
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
  if (!GetBrowserContext()) {
    return false;
  }

  return GetBrowserContext()->IsOffTheRecord();
}

bool WebView::IsLoading() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->IsLoading();
}

bool WebView::IsFullscreen() const {
  return is_fullscreen_;
}

void WebView::SetIsFullscreen(bool fullscreen) {
  if (fullscreen != is_fullscreen_) {
    is_fullscreen_ = fullscreen;
    if (web_contents_) {
      web_contents_->GetRenderViewHost()->WasResized();
    }
  }
}

void WebView::WasResized() {
  {
    CompositorLock lock(compositor_.get());
    compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);
    compositor_->SetViewportSize(GetViewSizePix());
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(GetViewSizeDip());
    GetRenderWidgetHostImpl()->SendScreenRects();
    rwhv->GetRenderWidgetHost()->WasResized();
  }

  MaybeResetAutoScrollTimer();
}

void WebView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

  if (visible) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
  }

  MaybeResetAutoScrollTimer();
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

  MaybeResetAutoScrollTimer();
}

void WebView::InputPanelVisibilityChanged() {
  if (!IsInputPanelVisible()) {
    did_scroll_focused_editable_node_into_view_ = false;
  }

  MaybeResetAutoScrollTimer();
}

BrowserContext* WebView::GetBrowserContext() const {
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
}

content::WebContents* WebView::GetWebContents() const {
  return web_contents_.get();
}

int WebView::GetNavigationEntryCount() const {
  if (!web_contents_) {
    return 0;
  }
  return web_contents_->GetController().GetEntryCount();
}

int WebView::GetNavigationCurrentEntryIndex() const {
  if (!web_contents_) {
    return -1;
  }
  return web_contents_->GetController().GetCurrentEntryIndex();
}

void WebView::SetNavigationCurrentEntryIndex(int index) {
  if (web_contents_) {
    web_contents_->GetController().GoToIndex(index);
  }
}

int WebView::GetNavigationEntryUniqueID(int index) const {
  if (!web_contents_) {
    return 0;
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetUniqueID();
}

const GURL& WebView::GetNavigationEntryUrl(int index) const {
  if (!web_contents_) {
    return GURL::EmptyGURL();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetURL();
}

std::string WebView::GetNavigationEntryTitle(int index) const {
  if (!web_contents_) {
    return std::string();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return base::UTF16ToUTF8(entry->GetTitle());
}

base::Time WebView::GetNavigationEntryTimestamp(int index) const {
  if (!web_contents_) {
    return base::Time();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetTimestamp();
}

WebFrame* WebView::GetRootFrame() const {
  return root_frame_;
}

content::FrameTree* WebView::GetFrameTree() {
  return web_contents_->GetFrameTree();
}

WebPreferences* WebView::GetWebPreferences() {
  if (!web_contents_helper_) {
    return initial_preferences_;
  }

  return web_contents_helper_->GetWebPreferences();
}

void WebView::SetWebPreferences(WebPreferences* prefs) {
  if (!web_contents_helper_) {
    CHECK(!prefs || prefs->IsOwnedByEmbedder());
    initial_preferences_ = prefs;
    WebPreferencesObserver::Observe(prefs);
  } else {
    web_contents_helper_->SetWebPreferences(prefs);
  }
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

void WebView::SetCanTemporarilyDisplayInsecureContent(bool allow) {
  if (!web_contents_) {
    return;
  }

  if (!(blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) && allow) {
    return;
  }

  if (allow) {
    blocked_content_ &= ~CONTENT_TYPE_MIXED_DISPLAY;
    OnContentBlocked();
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
    OnContentBlocked();
  }

  web_contents_->SendToAllFrames(
      new OxideMsg_SetAllowRunningInsecureContent(MSG_ROUTING_NONE, allow));
  web_contents_->GetMainFrame()->Send(
      new OxideMsg_ReloadFrame(web_contents_->GetMainFrame()->GetRoutingID()));
}

void WebView::ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                            const gfx::Rect& bounds,
                            int selected_item,
                            const std::vector<content::MenuItem>& items,
                            bool allow_multiple_selection) {
  DCHECK(!active_popup_menu_ || active_popup_menu_->WasHidden());

  WebPopupMenu* menu = CreatePopupMenu(render_frame_host);
  if (!menu) {
    static_cast<content::RenderFrameHostImpl *>(
        render_frame_host)->DidCancelPopupMenu();
    return;
  }

  active_popup_menu_ = menu->AsWeakPtr();

  menu->Show(bounds, items, selected_item, allow_multiple_selection);
}

void WebView::HidePopupMenu() {
  if (active_popup_menu_ && !active_popup_menu_->WasHidden()) {
    active_popup_menu_->Hide();
  }
}

void WebView::RequestGeolocationPermission(
    const GURL& origin,
    const base::Callback<void(bool)>& callback,
    base::Closure* cancel_callback) {
  scoped_ptr<SimplePermissionRequest> request(
      permission_request_manager_.CreateSimplePermissionRequest(
        PERMISSION_REQUEST_TYPE_GEOLOCATION,
        callback,
        cancel_callback));
  OnRequestGeolocationPermission(
      origin,
      web_contents_->GetLastCommittedURL().GetOrigin(),
      request.Pass());
}

void WebView::AllowCertificateError(
    content::RenderFrameHost* rfh,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(rfh);
  if (!frame) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  DCHECK_EQ(frame->view(), this);
  CHECK(!overridable || !strict_enforcement) <<
      "overridable and strict_enforcement are expected to be mutually exclusive";

  scoped_ptr<SimplePermissionRequest> request;
  // We can't safely allow the embedder to override errors for subresources or
  // subframes because they don't always result in the API indicating a
  // degraded security level. Mark these non-overridable for now and just
  // deny them outright
  // See https://launchpad.net/bugs/1368385
  if (overridable && resource_type == content::RESOURCE_TYPE_MAIN_FRAME) {
    request =
        permission_request_manager_.CreateSimplePermissionRequest(
          PERMISSION_REQUEST_TYPE_CERT_ERROR_OVERRIDE,
          callback, NULL);
  } else {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
  }

  if (!OnCertificateError(!frame->parent(),
                          ToCertError(cert_error, ssl_info.cert.get()),
                          ssl_info.cert,
                          request_url,
                          resource_type,
                          strict_enforcement,
                          request.Pass())) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
  }
}

void WebView::UpdateWebPreferences() {
  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->OnWebkitPreferencesChanged();
}

void WebView::HandleKeyEvent(const content::NativeWebKeyboardEvent& event) {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardKeyboardEvent(event);
}

void WebView::HandleMouseEvent(const blink::WebMouseEvent& event) {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardMouseEvent(event);
}

void WebView::HandleTouchEvent(const ui::TouchEvent& event) {
  if (!gesture_provider_->OnTouchEvent(event)) {
    return;
  }

  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host || !host->ShouldForwardTouchEvent()) {
    gesture_provider_->OnTouchEventAck(false);
    return;
  }

  scoped_ptr<ui::MotionEvent> motion_event =
      gesture_provider_->GetTouchState();
  host->ForwardTouchEventWithLatencyInfo(MakeWebTouchEvent(*motion_event),
                                         ui::LatencyInfo());
}

void WebView::HandleWheelEvent(const blink::WebMouseWheelEvent& event) {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardWheelEvent(event);
}

void WebView::ImeCommitText(const base::string16& text,
                            const gfx::Range& replacement_range) {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::RawKeyDown); 
  host->ImeConfirmComposition(text, replacement_range, false);
  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::KeyUp);
}

void WebView::ImeSetComposingText(
    const base::string16& text,
    const std::vector<blink::WebCompositionUnderline>& underlines,
    const gfx::Range& selection_range) {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::RawKeyDown); 
  host->ImeSetComposition(text, underlines,
                          selection_range.start(),
                          selection_range.end());
  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::KeyUp);
}

void WebView::DownloadRequested(
    const GURL& url,
    const std::string& mimeType,
    const bool shouldPrompt,
    const base::string16& suggestedFilename,
    const std::string& cookies,
    const std::string& referrer) {
  OnDownloadRequested(url, mimeType, shouldPrompt,
      suggestedFilename, cookies, referrer);
}

CompositorFrameHandle* WebView::GetCompositorFrameHandle() const {
  return current_compositor_frame_.get();
}

void WebView::DidCommitCompositorFrame() {
  DCHECK(!received_surface_ids_.empty());

  while (!received_surface_ids_.empty()) {
    uint32 surface_id = received_surface_ids_.front();
    received_surface_ids_.pop();

    compositor_->DidSwapCompositorFrame(surface_id,
                                        previous_compositor_frames_);
  }
}

void WebView::EvictCurrentFrame() {
  current_compositor_frame_ = NULL;
  OnEvictCurrentFrame();
}

void WebView::UpdateFrameMetadata(
    const cc::CompositorFrameMetadata& metadata) {
  bool has_mobile_viewport = HasMobileViewport(metadata);
  bool has_fixed_page_scale = HasFixedPageScale(metadata);
  gesture_provider_->SetDoubleTapSupportForPageEnabled(
      !has_fixed_page_scale && !has_mobile_viewport);

  cc::CompositorFrameMetadata old = compositor_frame_metadata_;
  compositor_frame_metadata_ = metadata;

  OnFrameMetadataUpdated(old);
}

void WebView::ProcessAckedTouchEvent(bool consumed) {
  gesture_provider_->OnTouchEventAck(consumed);
}

void WebView::UpdateCursor(const content::WebCursor& cursor) {}

void WebView::TextInputStateChanged(ui::TextInputType type,
                                    bool show_ime_if_needed) {
  if (type == text_input_type_ &&
      show_ime_if_needed == show_ime_if_needed_) {
    return;
  }

  text_input_type_ = type;
  show_ime_if_needed_ = show_ime_if_needed;

  OnTextInputStateChanged();
}

void WebView::FocusedNodeChanged(bool is_editable_node) {
  focused_node_is_editable_ = is_editable_node;
  OnFocusedNodeChanged();

  did_scroll_focused_editable_node_into_view_ = false;
  MaybeResetAutoScrollTimer();
}

void WebView::ImeCancelComposition() {}

void WebView::SelectionBoundsChanged(const gfx::Rect& caret_rect,
                                     size_t selection_cursor_position,
                                     size_t selection_anchor_position) {
  if (caret_rect == caret_rect_ &&
      selection_cursor_position == selection_cursor_position_ &&
      selection_anchor_position == selection_anchor_position_) {
    return;
  }

  caret_rect_ = caret_rect;
  selection_cursor_position_ = selection_cursor_position;
  selection_anchor_position_ = selection_anchor_position;

  OnSelectionBoundsChanged();
}

void WebView::SelectionChanged() {}

bool WebView::IsInputPanelVisible() const {
  return false;
}

JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type,
    bool* did_suppress_message) {
  return NULL;
}

JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  return NULL;
}

void WebView::FrameAdded(WebFrame* frame) {}
void WebView::FrameRemoved(WebFrame* frame) {}

bool WebView::CanCreateWindows() const {
  return false;
}

} // namespace oxide
