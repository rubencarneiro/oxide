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

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "url/gurl.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_web_contents_view.h"

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

  if (changed_flags & content::INVALIDATE_TYPE_LOAD) {
    OnLoadingChanged();
  }

  if (changed_flags & (content::INVALIDATE_TYPE_URL |
                       content::INVALIDATE_TYPE_LOAD)) {
    OnCommandsUpdated();
  }
}

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnLoadingChanged() {}
void WebView::OnCommandsUpdated() {}

WebView::WebView() {}

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

const GURL& WebView::GetURL() const {
  return web_contents_->GetActiveURL();
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

} // namespace oxide
