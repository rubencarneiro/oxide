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

#include "oxide_web_view_host.h"

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

namespace oxide {

void WebViewHost::NavigationStateChanged(const content::WebContents* source,
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

void WebViewHost::OnURLChanged() {}
void WebViewHost::OnTitleChanged() {}
void WebViewHost::OnLoadingChanged() {}
void WebViewHost::OnCommandsUpdated() {}

WebViewHost::WebViewHost() {}

bool WebViewHost::Init(bool incognito, const gfx::Size& initial_size) {
  DCHECK(!web_contents_) << "Called Init() more than once";

  if (!BrowserProcessMain::Exists()) {
    LOG(ERROR) << "Implementation needs to start the browser components first!";
    return false;
  }

  content::WebContents::CreateParams params(
      incognito ?
        BrowserContext::GetInstance()->GetOffTheRecordContext() :
        BrowserContext::GetInstance());
  params.initial_size = initial_size;
  web_contents_.reset(content::WebContents::Create(params));
  if (!web_contents_) {
    LOG(ERROR) << "Failed to create WebContents";
    return false;
  }

  web_contents_->SetDelegate(this);
  Observe(web_contents_.get());

  return true;
}

WebViewHost::~WebViewHost() {
  if (web_contents_) {
    web_contents_->SetDelegate(NULL);
  }
}

const GURL& WebViewHost::GetURL() const {
  return web_contents_->GetActiveURL();
}

void WebViewHost::SetURL(const GURL& url) {
  content::NavigationController::LoadURLParams params(url);
  web_contents_->GetController().LoadURLWithParams(params);
}

std::string WebViewHost::GetTitle() const {
  return base::UTF16ToUTF8(web_contents_->GetTitle());
}

bool WebViewHost::CanGoBack() const {
  return web_contents_->GetController().CanGoBack();
}

bool WebViewHost::CanGoForward() const {
  return web_contents_->GetController().CanGoForward();
}

void WebViewHost::GoBack() {
  web_contents_->GetController().GoBack();
}

void WebViewHost::GoForward() {
  web_contents_->GetController().GoForward();
}

void WebViewHost::Stop() {
  web_contents_->Stop();
}

void WebViewHost::Reload() {
  web_contents_->GetController().Reload(true);
}

bool WebViewHost::IsIncognito() const {
  return web_contents_->GetBrowserContext()->IsOffTheRecord();
}

bool WebViewHost::IsLoading() const {
  return web_contents_->IsLoading();
}

void WebViewHost::UpdateSize(const gfx::Size& size) {
  web_contents_->GetView()->SizeContents(size);
}

void WebViewHost::Shown() {
  web_contents_->WasShown();
}

void WebViewHost::Hidden() {
  web_contents_->WasHidden();
}

} // namespace oxide
