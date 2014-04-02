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

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
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
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/menu_item.h"
#include "net/base/net_errors.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_javascript_dialog_manager.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_frame.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_preferences.h"

namespace oxide {

namespace {

void FillLoadURLParamsFromOpenURLParams(
    content::NavigationController::LoadURLParams* load_params,
    const content::OpenURLParams& params) {
  load_params->override_user_agent =
      content::NavigationController::UA_OVERRIDE_TRUE;

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

class ScopedNewContentsHolder {
 public:
  ScopedNewContentsHolder(content::WebContents* contents,
                          bool* was_blocked) :
      contents_(contents), was_blocked_(was_blocked) {}

  ScopedNewContentsHolder(content::WebContents* contents) :
      contents_(contents), was_blocked_(NULL) {}

  ~ScopedNewContentsHolder() {
    if (contents_ && was_blocked_) {
      *was_blocked_ = true;
    }
  }

  content::WebContents* Release() {
    return contents_.release();
  }

  content::WebContents* contents() const {
    return contents_.get();
  }

 private:
  scoped_ptr<content::WebContents> contents_;
  bool* was_blocked_;
};

}

class WebView::PendingContents FINAL : public content::WebContentsObserver {
 public:
  PendingContents(content::WebContentsObserver* owner,
                  content::WebContents* contents,
                  const GURL& target_url,
                  int source_frame_id) :
      content::WebContentsObserver(contents),
      owner_(owner),
      target_url_(target_url),
      source_frame_id_(source_frame_id) {}

  void WebContentsDestroyed(content::WebContents* contents) {
    owner_->WebContentsDestroyed(contents);
  }

  GURL target_url() const { return target_url_; }
  int source_frame_id() const { return source_frame_id_; }

 private:
  content::WebContentsObserver* owner_;
  GURL target_url_;
  int source_frame_id_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(PendingContents);
};

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

bool WebView::InitCreatedWebView(WebView* view,
                                 content::WebContents* contents) {
  RenderWidgetHostView* rwhv = static_cast<RenderWidgetHostView *>(
      contents->GetRenderWidgetHostView());
  if (rwhv) {
    rwhv->Init(view);
  }

  Params params;
  params.contents = contents;

  return view->Init(params);
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return 0;
}

ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(size_t index) const {
  return NULL;
}

void WebView::NotifyUserAgentStringChanged() {
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents_->SetUserAgentOverride(context_->GetUserAgent());
}

void WebView::NotifyPopupBlockerEnabledChanged() {
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  rvh->UpdateWebkitPreferences(rvh->GetWebkitPreferences());
}

void WebView::WebPreferencesDestroyed() {
  OnWebPreferencesChanged();
  WebPreferencesValueChanged();
}

void WebView::WebPreferencesValueChanged() {
  if (!web_contents_) {
    return;
  }
  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  rvh->UpdateWebkitPreferences(rvh->GetWebkitPreferences());
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

content::WebContents* WebView::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  DCHECK_EQ(source, web_contents_.get());

  if (params.disposition != CURRENT_TAB &&
      params.disposition != NEW_FOREGROUND_TAB &&
      params.disposition != NEW_BACKGROUND_TAB &&
      params.disposition != NEW_POPUP &&
      params.disposition != NEW_WINDOW) {
    LOG(WARNING) << "Unexpected disposition";
    return NULL;
  }

  // Block popups
  if ((params.disposition == NEW_FOREGROUND_TAB ||
       params.disposition == NEW_BACKGROUND_TAB ||
       params.disposition == NEW_WINDOW ||
       params.disposition == NEW_POPUP) &&
      !params.user_gesture && context_->IsPopupBlockerEnabled()) {
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

  // Give the application a chance to block the navigation
  if (params.is_renderer_initiated &&
      top_level &&
      !ShouldHandleNavigation(params.url,
                              params.disposition,
                              params.user_gesture)) {
    return NULL;
  }

  if (params.disposition == CURRENT_TAB) {
    content::NavigationController::LoadURLParams load_params(params.url);
    FillLoadURLParamsFromOpenURLParams(&load_params, params);

    web_contents_->GetController().LoadURLWithParams(load_params);

    return web_contents_.get();
  }

  WebView* new_view = CreateNewWebView(params.url,
                                       GetContainerBounds(),
                                       params.disposition,
                                       params.user_gesture);
  if (!new_view) {
    return NULL;
  }

  // XXX(chrisccoulson): Is there a way to tell when the opener shouldn't
  // be suppressed?
  bool opener_suppressed = true;

  content::WebContents::CreateParams contents_params(
      GetBrowserContext(),
      opener_suppressed ? NULL : web_contents_->GetSiteInstance());
  contents_params.initial_size = GetContainerSize();
  contents_params.initially_hidden = params.disposition == NEW_BACKGROUND_TAB;
  contents_params.opener = opener_suppressed ? NULL : web_contents_.get();

  scoped_ptr<content::WebContents> contents(
      content::WebContents::Create(contents_params));
  if (!contents) {
    LOG(ERROR) << "Failed to create new WebContents for navigation";
    return NULL;
  }
  
  if (!InitCreatedWebView(new_view, contents.release())) {
    return NULL;
  }

  content::NavigationController::LoadURLParams load_params(params.url);
  FillLoadURLParamsFromOpenURLParams(&load_params, params);

  new_view->GetWebContents()->GetController().LoadURLWithParams(load_params);

  return new_view->GetWebContents();
}

void WebView::NavigationStateChanged(const content::WebContents* source,
                                     unsigned changed_flags) {
  DCHECK_EQ(source, web_contents_.get());

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

void WebView::WebContentsCreated(content::WebContents* source,
                                 int source_frame_id,
                                 const base::string16& frame_name,
                                 const GURL& target_url,
                                 content::WebContents* new_contents) {
  pending_contents_[new_contents] = linked_ptr<PendingContents>(
      new PendingContents(this, new_contents, target_url, source_frame_id));
}

void WebView::AddNewContents(content::WebContents* source,
                             content::WebContents* new_contents,
                             WindowOpenDisposition disposition,
                             const gfx::Rect& initial_pos,
                             bool user_gesture,
                             bool* was_blocked) {
  DCHECK_EQ(source, web_contents_.get());
  DCHECK_EQ(GetBrowserContext(),
            BrowserContext::FromContent(new_contents->GetBrowserContext()));

  ScopedNewContentsHolder holder(new_contents, was_blocked);

  if (disposition != NEW_FOREGROUND_TAB &&
      disposition != NEW_BACKGROUND_TAB &&
      disposition != NEW_POPUP &&
      disposition != NEW_WINDOW) {
    LOG(WARNING) << "Unexpected disposition";
    return;
  }

  PendingContentsMap::iterator it = pending_contents_.find(holder.contents());
  if (it == pending_contents_.end()) {
    return;
  }

  linked_ptr<PendingContents> pending(it->second);
  pending_contents_.erase(it);

  WebView* new_view = CreateNewWebView(pending->target_url(),
                                       initial_pos,
                                       disposition,
                                       user_gesture);
  if (!new_view) {
    return;
  }

  InitCreatedWebView(new_view, holder.Release());
}

void WebView::LoadProgressChanged(content::WebContents* source,
                                  double progress) {
  DCHECK_EQ(source, web_contents_.get());

  OnLoadProgressChanged(progress);
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  while (root_frame_->ChildCount() > 0) {
    root_frame_->ChildAt(0)->Destroy();
  }
}

void WebView::WebContentsDestroyed(content::WebContents* contents) {
  if (contents == web_contents_.get()) {
    return;
  }

  pending_contents_.erase(contents);
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
    frame->SetURL(url);
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

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
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

void WebView::OnWebPreferencesChanged() {}

bool WebView::ShouldHandleNavigation(const GURL& url,
                                     WindowOpenDisposition disposition,
                                     bool user_gesture) {
  return true;
}

WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return NULL;
}

WebView* WebView::CreateNewWebView(const GURL& target_url,
                                   const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition,
                                   bool user_gesture) {
  return NULL;
}

WebView::WebView() :
    root_frame_(NULL) {}

bool WebView::Init(const Params& params) {
  CHECK(!web_contents_);

  if (params.contents) {
    CHECK(!params.context);
    CHECK(params.contents->GetBrowserContext());

    context_ = BrowserContext::FromContent(params.contents->GetBrowserContext());
    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        params.contents));

    UpdateVisibility(IsVisible());
    UpdateSize(GetContainerSize());
  } else {
    CHECK(params.context);

    context_ = params.incognito ?
        params.context->GetOffTheRecordContext() :
        params.context->GetOriginalContext();

    content::WebContents::CreateParams params(context_);
    params.initial_size = GetContainerSize();
    params.initially_hidden = !IsVisible();
    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        content::WebContents::Create(params)));
    if (!web_contents_) {
      LOG(ERROR) << "Failed to create WebContents";
      return false;
    }
  }

  BrowserContextObserver::Observe(context_);

  web_contents_->SetDelegate(this);
  WebContentsObserver::Observe(web_contents_.get());
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents_->SetUserAgentOverride(context_->GetUserAgent());

  web_contents_->GetMutableRendererPrefs()->browser_handles_non_local_top_level_requests = true;

  registrar_.Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                 content::NotificationService::AllBrowserContextsAndSources());
  registrar_.Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                 content::NotificationService::AllBrowserContextsAndSources());

  root_frame_ = CreateWebFrame(web_contents_->GetFrameTree()->root());

  return true;
}

WebView::~WebView() {
  if (root_frame_) {
    root_frame_->Destroy();
  }
  if (web_contents_) {
    web_contents_->SetDelegate(NULL);
  }
}

// static
WebView* WebView::FromWebContents(content::WebContents* web_contents) {
  return static_cast<WebView *>(web_contents->GetDelegate());
}

// static
WebView* WebView::FromRenderViewHost(content::RenderViewHost* rvh) {
  return FromWebContents(content::WebContents::FromRenderViewHost(rvh));
}

const GURL& WebView::GetURL() const {
  if (!web_contents_) {
    return GURL::EmptyGURL();
  }
  return web_contents_->GetVisibleURL();
}

void WebView::SetURL(const GURL& url) {
  if (!url.is_valid()) {
    return;
  }

  content::NavigationController::LoadURLParams params(url);
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE;
  params.transition_type = content::PAGE_TRANSITION_TYPED;
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
  if (!context_) {
    return false;
  }
  return context_->IsOffTheRecord();
}

bool WebView::IsLoading() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->IsLoading();
}

void WebView::UpdateSize(const gfx::Size& size) {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->SetSize(size);
}

void WebView::UpdateVisibility(bool visible) {
  if (visible) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
  }
}

BrowserContext* WebView::GetBrowserContext() const {
  return context_;
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
  return web_preferences();
}

void WebView::SetWebPreferences(WebPreferences* prefs) {
  if (prefs == web_preferences()) {
    return;
  }

  WebPreferencesObserver::Observe(prefs);
  OnWebPreferencesChanged();
  WebPreferencesValueChanged();
}

gfx::Size WebView::GetContainerSize() {
  return GetContainerBounds().size();
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

content::JavaScriptDialogManager* WebView::GetJavaScriptDialogManager() {
  return JavaScriptDialogManager::GetInstance();
}

} // namespace oxide
