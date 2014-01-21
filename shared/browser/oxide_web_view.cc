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
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "net/base/net_errors.h"
#include "url/gurl.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_incoming_message.h"
#include "oxide_message_handler.h"
#include "oxide_outgoing_message_request.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_frame.h"
#include "oxide_web_preferences.h"

namespace oxide {

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

void WebView::SendErrorForV8Message(long long frame_id,
                                    const std::string& world_id,
                                    int serial,
                                    OxideMsg_SendMessage_Error::Value error_code,
                                    const std::string& error_desc) {
  OxideMsg_SendMessage_Params params;
  params.frame_id = frame_id;
  params.world_id = world_id;
  params.serial = serial;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = error_code;
  params.args = error_desc;

  // FIXME: This is clearly broken for OOPIF, and we don't even know if this
  //        is going to the correct RVH without OOPIF
  web_contents()->Send(new OxideMsg_SendMessage(
      web_contents()->GetRenderViewHost()->GetRoutingID(),
      params));
}

bool WebView::TryDispatchV8MessageToTarget(MessageTarget* target,
                                           WebFrame* source_frame,
                                           const std::string& world_id,
                                           int serial,
                                           const std::string& msg_id,
                                           const std::string& args) {
  for (size_t i = 0; i < target->GetMessageHandlerCount(); ++i) {
    MessageHandler* handler = target->GetMessageHandlerAt(i);

    if (!handler->IsValid()) {
      continue;
    }

    if (handler->msg_id() != msg_id) {
      continue;
    }

    const std::vector<std::string>& world_ids = handler->world_ids();

    for (std::vector<std::string>::const_iterator it = world_ids.begin();
         it != world_ids.end(); ++it) {
      if ((*it) == world_id) {
        handler->OnReceiveMessage(
            new IncomingMessage(source_frame, serial, world_id,
                                msg_id, args));
        return true;
      }
    }
  }

  return false;
}

void WebView::DispatchV8Message(const OxideMsg_SendMessage_Params& params) {
  WebFrame* frame = FindFrameWithID(params.frame_id);

  if (params.type == OxideMsg_SendMessage_Type::Message) {
    if (!frame) {
      // FIXME: In an OOPIF world, how do we know which process to dispatch
      //        this error too, if we couldn't find a frame?
      SendErrorForV8Message(params.frame_id, params.world_id, params.serial,
                            OxideMsg_SendMessage_Error::INVALID_DESTINATION,
                            "Invalid frame ID");
      return;
    }

    if (!TryDispatchV8MessageToTarget(frame, frame, params.world_id,
                                      params.serial, params.msg_id,
                                      params.args)) {
      if (!TryDispatchV8MessageToTarget(this, frame, params.world_id,
                                        params.serial, params.msg_id,
                                        params.args)) {
        SendErrorForV8Message(params.frame_id, params.world_id, params.serial,
                              OxideMsg_SendMessage_Error::NO_HANDLER,
                              "No handler was found for message");
      }
    }

    return;
  }

  if (!frame) {
    return;
  }

  for (size_t i = 0; i < frame->GetOutgoingMessageRequestCount(); ++i) {
    OutgoingMessageRequest* request = frame->GetOutgoingMessageRequestAt(i);

    if (request->serial() == params.serial) {
      request->OnReceiveResponse(params.args, params.error);
      return;
    }
  }
}

void WebView::OnFrameCreated(int64 parent_frame_id,
                             int64 frame_id) {
  WebFrame* parent = FindFrameWithID(parent_frame_id);
  if (!parent) {
    LOG(ERROR) << "Got FrameCreated with non-existant parent";
    return;
  }

  WebFrame* frame = CreateWebFrame();
  frame->set_identifier(frame_id);
  frame->set_view(this);
  frame->SetParent(parent);
}

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnCommandsUpdated() {}

void WebView::OnLoadProgressChanged(double progress) {}

void WebView::OnRootFrameChanged() {}

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

WebView::WebView() :
    root_frame_(NULL),
    web_preferences_(NULL) {}

WebView::~WebView() {
  if (web_preferences_) {
    web_preferences_->RemoveWebView(this);
  }

  if (web_contents_) {
    GetBrowserContext()->RemoveWebView(this);
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

  context->AddWebView(this);

  content::WebContents::CreateParams params(context);
  params.initial_size = initial_size;
  web_contents_.reset(content::WebContents::Create(params));
  if (!web_contents_) {
    LOG(ERROR) << "Failed to create WebContents";
    return false;
  }

  web_contents_->SetDelegate(this);
  WebContentsObserver::Observe(web_contents_.get());

  registrar_.reset(new content::NotificationRegistrar);
  registrar_->Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                  content::NotificationService::AllBrowserContextsAndSources());
  registrar_->Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                  content::NotificationService::AllBrowserContextsAndSources());

  return true;
}

void WebView::Shutdown() {
  if (!web_contents_) {
    LOG(ERROR) << "Called Shutdown() on a webview that isn't initialized";
    return;
  }

  registrar_.reset();

  WebContentsObserver::Observe(NULL);

  if (root_frame_) {
    root_frame_->DestroyFrame();
    root_frame_ = NULL;
  }

  GetBrowserContext()->RemoveWebView(this);
  web_contents_.reset();
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
  if (!web_contents_) {
    return false;
  }
  return web_contents_->GetBrowserContext()->IsOffTheRecord();
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
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
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

WebFrame* WebView::FindFrameWithID(int64 frame_id) const {
  if (!root_frame_) {
    return NULL;
  }

  std::queue<WebFrame *> q;
  q.push(const_cast<WebFrame *>(root_frame_));

  while (!q.empty()) {
    WebFrame* f = q.front();
    q.pop();

    if (f->identifier() == frame_id) {
      return f;
    }

    f->AddChildrenToQueue(&q);
  }

  return NULL;
}

WebPreferences* WebView::GetWebPreferences() {
  if (!web_preferences_) {
    SetWebPreferences(
        ContentClient::instance()->browser()->GetDefaultWebPreferences(),
        false);
  }

  return web_preferences_;
}

void WebView::SetWebPreferences(WebPreferences* prefs, bool send_update) {
  if (web_preferences_) {
    web_preferences_->RemoveWebView(this);
  }

  web_preferences_ = prefs;
  if (prefs) {
    prefs->AddWebView(this);
  }

  if (!send_update || !web_contents_) {
    return;
  }

  web_contents()->GetRenderViewHost()->UpdateWebkitPreferences(
      web_contents()->GetRenderViewHost()->GetWebkitPreferences());
}

void WebView::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (content::Source<content::NavigationController>(source).ptr() != &web_contents()->GetController()) {
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

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  // Make sure the new RWHV gets the correct size
  web_contents()->GetView()->SizeContents(
      web_contents()->GetView()->GetContainerSize());

  if (root_frame_) {
    root_frame_->DestroyFrame();
  }

  if (new_host) {
    root_frame_ = CreateWebFrame();
    root_frame_->set_view(this);
  } else {
    root_frame_ = NULL;
  }

  OnRootFrameChanged();
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
  if (is_main_frame) {
    root_frame_->set_identifier(frame_id);
  }

  WebFrame* frame = FindFrameWithID(frame_id);
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
                            int64 frame_id) {
  WebFrame* frame = FindFrameWithID(frame_id);
  if (!frame) {
    return;
  }

  frame->DestroyFrame();
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

bool WebView::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WebView, message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_SendMessage, DispatchV8Message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_FrameCreated, OnFrameCreated)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

size_t WebView::GetMessageHandlerCount() const {
  return 0;
}

MessageHandler* WebView::GetMessageHandlerAt(size_t index) const {
  return NULL;
}

WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return NULL;
}

} // namespace oxide
