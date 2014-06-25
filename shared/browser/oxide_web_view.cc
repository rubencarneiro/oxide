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

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/supports_user_data.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/menu_item.h"
#include "content/public/common/url_constants.h"
#include "net/base/net_errors.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"
#include "url/url_constants.h"
#include "webkit/common/webpreferences.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/common/oxide_content_client.h"
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
  RenderWidgetHostView* rwhv = static_cast<RenderWidgetHostView *>(
      contents->GetRenderWidgetHostView());
  if (rwhv) {
    rwhv->Init(view);
  }

  WebView::Params params;
  params.contents = contents.Pass();

  view->Init(&params);
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
      BrowserProcessMain::instance()->GetSharedGLContext();
  if (!share_context) {
    return true;
  }

  if (share_context->GetImplementation() != gfx::GetGLImplementation()) {
    return true;
  }

  result = false;
  return false;
}

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

RenderWidgetHostView* WebView::GetRenderWidgetHostView() {
  if (!web_contents_) {
    return NULL;
  }

  return static_cast<RenderWidgetHostView *>(
      web_contents_->GetRenderWidgetHostView());
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

  if (current_compositor_frame_) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = frame;

  OnSwapCompositorFrame();
}

void WebView::WebPreferencesDestroyed() {
  initial_preferences_ = NULL;
  OnWebPreferencesDestroyed();
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
  contents_params.initial_size = GetContainerSizeDip();
  contents_params.initially_hidden = disposition == NEW_BACKGROUND_TAB;
  contents_params.opener = opener_suppressed ? NULL : web_contents_.get();

  ScopedNewContentsHolder contents(
      content::WebContents::Create(contents_params));
  if (!contents) {
    LOG(ERROR) << "Failed to create new WebContents for navigation";
    return NULL;
  }

  WebViewContentsHelper::Attach(contents.get(), web_contents_.get());

  WebView* new_view = CreateNewWebView(GetContainerBoundsPix(), disposition);
  if (!new_view) {
    return NULL;
  }

  InitCreatedWebView(new_view, contents.Pass());

  content::NavigationController::LoadURLParams load_params(local_params.url);
  FillLoadURLParamsFromOpenURLParams(&load_params, local_params);

  new_view->GetWebContents()->GetController().LoadURLWithParams(load_params);

  return new_view->GetWebContents();
}

void WebView::NavigationStateChanged(unsigned changed_flags) {
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

void WebView::HandleKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  OnUnhandledKeyboardEvent(event);
}

void WebView::RenderProcessGone(base::TerminationStatus status) {
  geolocation_permission_requests_.CancelAllPending();
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  while (root_frame_->ChildCount() > 0) {
    root_frame_->ChildAt(0)->Destroy();
  }

  if (old_host && old_host->GetView()) {
    static_cast<RenderWidgetHostView *>(old_host->GetView())->SetWebView(NULL);
  }
  if (new_host && new_host->GetView()) {
    static_cast<RenderWidgetHostView *>(new_host->GetView())->SetWebView(this);
  }
}

void WebView::DidStartProvisionalLoadForFrame(
    int64 frame_id,
    int64 parent_frame_id,
    bool is_main_frame,
    const GURL& validated_url,
    bool is_error_frame,
    bool is_iframe_srcdoc,
    content::RenderViewHost* render_view_host) {
  if (!is_main_frame) {
    return;
  }

  OnLoadStarted(validated_url, is_error_frame);
}

void WebView::DidCommitProvisionalLoadForFrame(
    int64 frame_id,
    const base::string16& frame_unique_name,
    bool is_main_frame,
    const GURL& url,
    content::PageTransition transition_type,
    content::RenderViewHost* render_view_host) {
  content::FrameTreeNode* node =
      web_contents_->GetFrameTree()->FindByRoutingID(
        frame_id, render_view_host->GetProcess()->GetID());
  DCHECK(node);

  WebFrame* frame = WebFrame::FromFrameTreeNode(node);
  if (frame) {
    frame->URLChanged();
  }
}

void WebView::DidFailProvisionalLoad(
    int64 frame_id,
    const base::string16& frame_unique_name,
    bool is_main_frame,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description,
    content::RenderViewHost* render_view_host) {
  if (!is_main_frame) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::DidFinishLoad(int64 frame_id,
                            const GURL& validated_url,
                            bool is_main_frame,
                            content::RenderViewHost* render_view_host) {
  if (!is_main_frame) {
    return;
  }

  OnLoadSucceeded(validated_url);
}

void WebView::DidFailLoad(int64 frame_id,
                          const GURL& validated_url,
                          bool is_main_frame,
                          int error_code,
                          const base::string16& error_description,
                          content::RenderViewHost* render_view_host) {
  if (!is_main_frame) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  if (load_details.is_navigation_to_different_page()) {
    geolocation_permission_requests_.CancelAllPending();
  }
  OnNavigationEntryCommitted();
}

void WebView::FrameDetached(content::RenderViewHost* rvh,
                            int64 frame_routing_id) {
  if (!root_frame_) {
    return;
  }

  content::FrameTreeNode* node =
      web_contents_->GetFrameTree()->FindByRoutingID(
        frame_routing_id, rvh->GetProcess()->GetID());
  DCHECK(node);

  WebFrame* frame = WebFrame::FromFrameTreeNode(node);
  frame->Destroy();
}

void WebView::FrameAttached(content::RenderViewHost* rvh,
                            int64 parent_frame_routing_id,
                            int64 frame_routing_id) {
  if (!root_frame_) {
    return;
  }

  content::FrameTree* tree = web_contents_->GetFrameTree();
  int process_id = rvh->GetProcess()->GetID();

  content::FrameTreeNode* parent_node =
      tree->FindByRoutingID(parent_frame_routing_id, process_id);
  content::FrameTreeNode* node =
      tree->FindByRoutingID(frame_routing_id, process_id);

  DCHECK(parent_node && node);

  WebFrame* parent = WebFrame::FromFrameTreeNode(parent_node);
  DCHECK(parent);

  WebFrame* frame = CreateWebFrame(node);
  DCHECK(frame);
  frame->SetParent(parent);
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

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnIconChanged(const GURL& icon) {}
void WebView::OnCommandsUpdated() {}

void WebView::OnLoadProgressChanged(double progress) {}

void WebView::OnLoadStarted(const GURL& validated_url,
                            bool is_error_frame) {}
void WebView::OnLoadStopped(const GURL& validated_url) {}
void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {}
void WebView::OnLoadSucceeded(const GURL& validated_url) {}

void WebView::OnNavigationEntryCommitted() {}
void WebView::OnNavigationListPruned(bool from_front, int count) {}
void WebView::OnNavigationEntryChanged(int index) {}

void WebView::OnWebPreferencesDestroyed() {}

void WebView::OnRequestGeolocationPermission(
    scoped_ptr<GeolocationPermissionRequest> request) {}

void WebView::OnUnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {}

bool WebView::OnAddMessageToConsole(int32 level,
                                    const base::string16& message,
                                    int32 line_no,
                                    const base::string16& source_id) {
  return false;
}

void WebView::OnToggleFullscreenMode(bool enter) {}

bool WebView::ShouldHandleNavigation(const GURL& url,
                                     WindowOpenDisposition disposition,
                                     bool user_gesture) {
  return true;
}

WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return NULL;
}

WebView* WebView::CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) {
  NOTREACHED() <<
      "Your CanCreateWindows() implementation should be returning false!";
  return NULL;
}

void WebView::OnEvictCurrentFrame() {}

WebView::WebView()
    : web_contents_helper_(NULL),
      compositor_(Compositor::Create(this, ShouldUseSoftwareCompositing())),
      initial_preferences_(NULL),
      root_frame_(NULL),
      is_fullscreen_(false) {
}

float WebView::GetDeviceScaleFactor() const {
  return frame_metadata_.device_scale_factor;
}

float WebView::GetPageScaleFactor() const {
  return frame_metadata_.page_scale_factor;
}

void WebView::PageScaleFactorChanged() {}

const gfx::Vector2dF& WebView::GetRootScrollOffset() const {
  return frame_metadata_.root_scroll_offset;
}

void WebView::RootScrollOffsetXChanged() {}
void WebView::RootScrollOffsetYChanged() {}

const gfx::SizeF& WebView::GetRootLayerSize() const {
  return frame_metadata_.root_layer_size;
}

void WebView::RootLayerWidthChanged() {}
void WebView::RootLayerHeightChanged() {}

const gfx::SizeF& WebView::GetViewportSize() const {
  return frame_metadata_.viewport_size;
}

void WebView::ViewportWidthChanged() {}
void WebView::ViewportHeightChanged() {}

WebView::~WebView() {
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

    WasResized();
    VisibilityChanged();

    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->SetWebView(this);
    }

    // Update our preferences in case something has changed (like
    // CanCreateWindows())
    UpdateWebPreferences();
  } else {
    CHECK(params->context) << "Didn't specify a BrowserContext or WebContents";

    BrowserContext* context = params->incognito ?
        params->context->GetOffTheRecordContext() :
        params->context->GetOriginalContext();

    content::WebContents::CreateParams content_params(context);
    content_params.initial_size = GetContainerSizeDip();
    content_params.initially_hidden = !IsVisible();
    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        content::WebContents::Create(content_params)));
    CHECK(web_contents_.get()) << "Failed to create WebContents";

    WebViewContentsHelper::Attach(web_contents_.get());

    compositor_->SetViewportSize(GetContainerSizePix());
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
  if (!initial_url_.is_empty() && params->context) {
    SetURL(initial_url_);
    initial_url_ = GURL();
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
  web_contents_->GetController().LoadURLWithParams(params);
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
    compositor_->SetViewportSize(GetContainerSizePix());
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(GetContainerSizeDip());
  }
}

void WebView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

  if (visible) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
  }
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

gfx::Size WebView::GetContainerSizePix() const {
  return GetContainerBoundsPix().size();
}

gfx::Rect WebView::GetContainerBoundsDip() const {
  return gfx::ScaleToEnclosingRect(
      GetContainerBoundsPix(), 1.0f / GetScreenInfo().deviceScaleFactor);
}

gfx::Size WebView::GetContainerSizeDip() const {
  return GetContainerBoundsDip().size();
}

void WebView::ShowPopupMenu(const gfx::Rect& bounds,
                            int selected_item,
                            const std::vector<content::MenuItem>& items,
                            bool allow_multiple_selection) {
  DCHECK(!active_popup_menu_ || active_popup_menu_->WasHidden());

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  WebPopupMenu* menu = CreatePopupMenu(rvh);
  if (!menu) {
    static_cast<content::RenderViewHostImpl *>(rvh)->DidCancelPopupMenu();
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
    int id,
    const GURL& origin,
    const base::Callback<void(bool)>& callback) {
  scoped_ptr<GeolocationPermissionRequest> request(
      new GeolocationPermissionRequest(
        &geolocation_permission_requests_, id, origin,
        web_contents_->GetLastCommittedURL().GetOrigin(), callback));
  OnRequestGeolocationPermission(request.Pass());
}

void WebView::CancelGeolocationPermissionRequest(int id) {
  geolocation_permission_requests_.CancelPendingRequestWithID(id);
}

void WebView::UpdateWebPreferences() {
  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (rvh) {
    rvh->UpdateWebkitPreferences(rvh->GetWebkitPreferences());
  }
}

CompositorFrameHandle* WebView::GetCompositorFrameHandle() {
  return current_compositor_frame_;
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

void WebView::GotNewCompositorFrameMetadata(
    const cc::CompositorFrameMetadata& metadata) {
  gfx::Vector2dF root_scroll_offset = frame_metadata_.root_scroll_offset;
  float page_scale_factor = frame_metadata_.page_scale_factor;
  gfx::SizeF root_layer_size = frame_metadata_.root_layer_size;
  gfx::SizeF viewport_size = frame_metadata_.viewport_size;
  frame_metadata_ = metadata;
  if (metadata.page_scale_factor != page_scale_factor) {
    PageScaleFactorChanged();
  }
  if (metadata.root_scroll_offset.x() != root_scroll_offset.x()) {
    RootScrollOffsetXChanged();
  }
  if (metadata.root_scroll_offset.y() != root_scroll_offset.y()) {
    RootScrollOffsetYChanged();
  }
  if (metadata.root_layer_size.width() != root_layer_size.width()) {
    RootLayerWidthChanged();
  }
  if (metadata.root_layer_size.height() != root_layer_size.height()) {
    RootLayerHeightChanged();
  }
  if (metadata.viewport_size.width() != viewport_size.width()) {
    ViewportWidthChanged();
  }
  if (metadata.viewport_size.height() != viewport_size.height()) {
    ViewportHeightChanged();
  }
}

JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type,
    bool* did_suppress_message) {
  return NULL;
}

JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  return NULL;
}

FilePicker* WebView::CreateFilePicker(content::RenderViewHost* rvh) {
  return NULL;
}

void WebView::FrameAdded(WebFrame* frame) {}
void WebView::FrameRemoved(WebFrame* frame) {}

bool WebView::CanCreateWindows() const {
  return false;
}

} // namespace oxide
