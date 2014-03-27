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
#include "url/gurl.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"

#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_file_picker.h"
#include "oxide_javascript_dialog_manager.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_frame.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_preferences.h"

namespace oxide {

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

void WebView::NotifyUserAgentStringChanged() {
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents_->SetUserAgentOverride(context_->GetUserAgent());
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

void WebView::LoadProgressChanged(content::WebContents* source,
                                  double progress) {
  DCHECK_EQ(source, web_contents_.get());

  OnLoadProgressChanged(progress);
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  // Make sure the new RWHV gets the correct size
  GetWebContents()->GetView()->SizeContents(
      GetWebContents()->GetView()->GetContainerSize());

  while (root_frame_->ChildCount() > 0) {
    delete root_frame_->ChildAt(0);
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
  delete frame;
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

WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return NULL;
}

WebView::WebView() {}

WebView::~WebView() {
  if (web_contents_) {
    web_contents_->SetDelegate(NULL);
  }
}

bool WebView::Init(BrowserContext* context,
                   bool incognito,
                   const gfx::Size& initial_size) {
  DCHECK(context);

  if (web_contents_) {
    LOG(ERROR) << "Called Init() more than once";
    return false;
  }

  context = incognito ?
      context->GetOffTheRecordContext() :
      context->GetOriginalContext();

  BrowserContextObserver::Observe(context);
  context_ = context;

  content::WebContents::CreateParams params(context);
  params.initial_size = initial_size;
  web_contents_.reset(static_cast<content::WebContentsImpl *>(
      content::WebContents::Create(params)));
  if (!web_contents_) {
    LOG(ERROR) << "Failed to create WebContents";
    return false;
  }

  web_contents_->SetDelegate(this);
  WebContentsObserver::Observe(web_contents_.get());
  // See https://launchpad.net/bugs/1279900 and the comment in
  // HttpUserAgentSettings::GetUserAgent()
  web_contents_->SetUserAgentOverride(context->GetUserAgent());

  registrar_.Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                 content::NotificationService::AllBrowserContextsAndSources());
  registrar_.Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                 content::NotificationService::AllBrowserContextsAndSources());

  root_frame_.reset(CreateWebFrame(web_contents_->GetFrameTree()->root()));

  return true;
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
  web_contents_->GetView()->SizeContents(size);
}

void WebView::Shown() {
  web_contents_->WasShown();
}

void WebView::Hidden() {
  web_contents_->WasHidden();
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
  return root_frame_.get();
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

void WebView::RunFileChooser(content::WebContents* web_contents,
                             const content::FileChooserParams& params) {
  DCHECK(!active_file_picker_);
  content::RenderViewHost* rvh = web_contents->GetRenderViewHost();
  FilePicker* filePicker = CreateFilePicker(rvh);
  if (!filePicker) {
    std::vector<ui::SelectedFileInfo> empty;
    rvh->FilesSelectedInChooser(empty, params.mode);
    return;
  }
  active_file_picker_ = filePicker->GetWeakPtr();
  active_file_picker_->Run(params);
}

} // namespace oxide
