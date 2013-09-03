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
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_message_handler.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_frame.h"

namespace oxide {

WebView::NotificationObserver::NotificationObserver(WebView* web_view) :
    web_view_(web_view) {}

void WebView::NotificationObserver::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  if (type != content::NOTIFICATION_WEB_CONTENTS_SWAPPED ||
      content::Source<content::WebContents>(source).ptr() !=
          web_view_->web_contents_.get()) {
    return;
  }

  web_view_->NotifyRenderViewHostSwappedIn();
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

  if (changed_flags & content::INVALIDATE_TYPE_LOAD) {
    OnLoadingChanged();
  }

  if (changed_flags & (content::INVALIDATE_TYPE_URL |
                       content::INVALIDATE_TYPE_LOAD)) {
    OnCommandsUpdated();
  }
}

void WebView::NotifyRenderViewHostSwappedIn() {
  WebFrame* root = NULL;
  content::RenderViewHostImpl* rvh =
      static_cast<content::RenderViewHostImpl *>(
        web_contents_->GetRenderViewHost());

  if (rvh->main_frame_id() != -1) {
    root = AllocWebFrame(rvh->main_frame_id());
    if (root) {
      root->SetView(this);
    }
  }

  root_frame_.reset(root);
  OnRootFrameChanged();
}

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnLoadingChanged() {}
void WebView::OnCommandsUpdated() {}
void WebView::OnRootFrameChanged() {}

WebFrame* WebView::AllocWebFrame(int64 frame_id) {
  return NULL;
}


WebView::WebView() :
    notification_observer_(this) {}

bool WebView::Init(BrowserContext* context,
                   WebContentsViewDelegate* delegate,
                   bool incognito,
                   const gfx::Size& initial_size) {
  DCHECK(!web_contents_) << "Called Init() more than once";
  DCHECK(context) << "Must supply a context";
  DCHECK(delegate) << "Must supply a delegate";
  DCHECK(process_handle_.Available()) <<
       "Failed to start the browser components first!";

  context->AddWebView(this);

  content::WebContents::CreateParams params(
      incognito ?
        context->GetOffTheRecordContext() :
        context->GetOriginalContext());
  params.initial_size = initial_size;
  web_contents_.reset(content::WebContents::Create(params));
  if (!web_contents_) {
    LOG(ERROR) << "Failed to create WebContents";
    return false;
  }

  web_contents_->SetDelegate(this);
  Observe(web_contents_.get());
  registrar_.Add(
      &notification_observer_,
      content::NOTIFICATION_WEB_CONTENTS_SWAPPED,
      content::Source<content::WebContents>(web_contents_.get()));

  static_cast<oxide::WebContentsView *>(
      web_contents_->GetView())->SetDelegate(delegate);

  return true;
}

void WebView::DestroyWebContents() {
  GetBrowserContext()->RemoveWebView(this);
  web_contents_.reset();
}

WebView::~WebView() {
  if (web_contents_) {
    GetBrowserContext()->RemoveWebView(this);
    web_contents_->SetDelegate(NULL);
  }
}

// static
WebView* WebView::FromWebContents(content::WebContents* web_contents) {
  return static_cast<WebView *>(web_contents->GetDelegate());
}

const GURL& WebView::GetURL() const {
  return web_contents_->GetVisibleURL();
}

void WebView::SetURL(const GURL& url) {
  content::NavigationController::LoadURLParams params(url);
  web_contents_->GetController().LoadURLWithParams(params);
}

std::string WebView::GetTitle() const {
  return base::UTF16ToUTF8(web_contents_->GetTitle());
}

bool WebView::CanGoBack() const {
  return web_contents_->GetController().CanGoBack();
}

bool WebView::CanGoForward() const {
  return web_contents_->GetController().CanGoForward();
}

void WebView::GoBack() {
  web_contents_->GetController().GoBack();
}

void WebView::GoForward() {
  web_contents_->GetController().GoForward();
}

void WebView::Stop() {
  web_contents_->Stop();
}

void WebView::Reload() {
  web_contents_->GetController().Reload(true);
}

bool WebView::IsIncognito() const {
  return web_contents_->GetBrowserContext()->IsOffTheRecord();
}

bool WebView::IsLoading() const {
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

WebFrame* WebView::GetRootFrame() const {
  return root_frame_.get();
}

WebFrame* WebView::FindFrameWithID(int64 frame_id) const {
  if (!root_frame_) {
    return NULL;
  }

  return root_frame_->FindFrameWithID(frame_id);
}

void WebView::DidCommitProvisionalLoadForFrame(
    int64 frame_id,
    bool is_main_frame,
    const GURL& url,
    content::PageTransition transition_type,
    content::RenderViewHost* render_view_host) {
  if (render_view_host != web_contents_->GetRenderViewHost()) {
    return;
  }

  if (!root_frame_) {
    WebFrame* root = AllocWebFrame(frame_id);
    if (root) {
      root->SetView(this);
    }
    root_frame_.reset(root);
    DCHECK(!root_frame_ || is_main_frame);
    OnRootFrameChanged();
  }

  WebFrame* frame = FindFrameWithID(frame_id);
  if (frame) {
    frame->SetURL(url);
  }
}

void WebView::FrameAttached(content::RenderViewHost* render_view_host,
                            int64 parent_frame_id,
                            int64 frame_id) {
  if (render_view_host != web_contents_->GetRenderViewHost()) {
    return;
  }

  WebFrame* parent = FindFrameWithID(parent_frame_id);
  if (!parent) {
    return;
  }

  WebFrame* frame = AllocWebFrame(frame_id);
  if (!frame) {
    return;
  }

  parent->AddChildFrame(frame);
}

void WebView::FrameDetached(content::RenderViewHost* render_view_host,
                            int64 frame_id) {
  if (render_view_host != web_contents_->GetRenderViewHost()) {
    return;
  }

  WebFrame* frame = FindFrameWithID(frame_id);
  if (!frame) {
    return;
  }

  frame->parent()->RemoveChildFrame(frame);
}

MessageDispatcherBrowser::MessageHandlerVector
WebView::GetMessageHandlers() const {
  return MessageDispatcherBrowser::MessageHandlerVector();
}

} // namespace oxide
