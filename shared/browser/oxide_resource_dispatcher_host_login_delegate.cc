// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_resource_dispatcher_host_login_delegate.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "net/base/auth.h"
#include "net/url_request/url_request.h"

#include "oxide_web_view.h"

namespace oxide {

ResourceDispatcherHostLoginDelegate::ResourceDispatcherHostLoginDelegate(
    net::AuthChallengeInfo* auth_info,
    net::URLRequest* request)
    : request_(request) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  host_ = auth_info->challenger.ToString();
  realm_ = auth_info->realm;

  int render_process_id;
  int render_frame_id;
  content::ResourceRequestInfo::GetRenderFrameForRequest(
      request, &render_process_id, &render_frame_id);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&ResourceDispatcherHostLoginDelegate::DispatchRequest,
          this, render_process_id, render_frame_id));
}

ResourceDispatcherHostLoginDelegate::~ResourceDispatcherHostLoginDelegate() {}

void ResourceDispatcherHostLoginDelegate::SetCancelledCallback(
    const base::Closure& cancelled_callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  cancelled_callback_ = cancelled_callback;
}

void ResourceDispatcherHostLoginDelegate::OnRequestCancelled() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  request_ = nullptr;

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &ResourceDispatcherHostLoginDelegate::DispatchCancelledCallback,
          this));
}

void ResourceDispatcherHostLoginDelegate::DispatchCancelledCallback() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (!cancelled_callback_.is_null()) {
    cancelled_callback_.Run();
  }
}

void ResourceDispatcherHostLoginDelegate::Deny() {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegate::Deny, this));
    return;
  }

  if (!request_) {
    return;
  }

  request_->CancelAuth();
  content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(
      request_);
  request_ = nullptr;
}

void ResourceDispatcherHostLoginDelegate::Allow(const std::string &username,
                                                const std::string &password)
{
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegate::Allow, this,
                   username, password));
    return;
  }

  if (!request_) {
    return;
  }

  request_->SetAuth(net::AuthCredentials(base::UTF8ToUTF16(username),
                                         base::UTF8ToUTF16(password)));
  content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(
        request_);
  request_ = nullptr;
}

void ResourceDispatcherHostLoginDelegate::DispatchRequest(
    int render_process_id, int render_frame_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!rfh) {
    Deny();
    return;
  }

  WebView* webview = WebView::FromRenderFrameHost(rfh);
  if (!webview) {
    Deny();
    return;
  }

  webview->HttpAuthenticationRequested(this);
}

std::string ResourceDispatcherHostLoginDelegate::Host() const {
  return host_;
}

std::string ResourceDispatcherHostLoginDelegate::Realm() const {
  return realm_;
}

} // namespace oxide
