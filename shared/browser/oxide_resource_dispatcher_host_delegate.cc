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

#include "oxide_resource_dispatcher_host_delegate.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/referrer.h"
#include "net/base/mime_util.h"
#include "net/cookies/cookie_monster.h"
#include "net/http/http_content_disposition.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request_context.h"
#include "url/gurl.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_redirection_intercept_throttle.h"
#include "oxide_web_view.h"

namespace oxide {

struct ResourceDispatcherHostDelegate::DownloadRequestParams {
  GURL url;
  bool is_content_initiated;
  base::string16 suggested_name;
  bool use_prompt;
  GURL referrer;
  std::string mime_type;
  int render_process_id;
  int render_view_id;
  std::string user_agent;
};

void ResourceDispatcherHostDelegate::DownloadStarting(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    int child_id,
    int route_id,
    int request_id,
    bool is_content_initiated,
    bool must_download,
    const std::string& suggested_filename,
    ScopedVector<content::ResourceThrottle>* throttles) {
  std::string suggested_name = suggested_filename;
  std::string mime_type;

  net::HttpResponseHeaders* response_headers = request->response_headers();
  if (response_headers) {
    if (suggested_name.empty()) {
      std::string disposition;
      response_headers->GetNormalizedHeader(
          "content-disposition",
          &disposition);
      net::HttpContentDisposition content_disposition(disposition, std::string());
      suggested_name = content_disposition.filename();
    }

    response_headers->GetMimeType(&mime_type);
  }
  request->Cancel();

  if (!suggested_name.empty() && mime_type.empty()) {
    base::FilePath::StringType ext =
        base::FilePath(suggested_name).Extension();
    if (!ext.empty()) {
      ext.erase(ext.begin());
    }
    net::GetMimeTypeFromExtension(ext, &mime_type);
  }

  // POST request cannot be repeated in general, so prevent client from
  // retrying the same request, even if it is with a GET.
  if ("GET" == request->method()) {
    content::Referrer referrer;
    referrer.url = GURL(request->referrer());
    DispatchDownloadRequest(
        request->url(),
        request->first_party_for_cookies(),
        is_content_initiated,
        base::UTF8ToUTF16(suggested_name),
        false,
        referrer,
        mime_type,
        child_id,
        route_id,
        resource_context,
        request);
  }
}

void ResourceDispatcherHostDelegate::DispatchDownloadRequest(
    const GURL& url,
    const GURL& first_party_url,
    bool is_content_initiated,
    const base::string16& suggested_name,
    const bool use_prompt,
    const content::Referrer& referrer,
    const std::string& mime_type,
    int render_process_id,
    int render_view_id,
    content::ResourceContext* resource_context,
    net::URLRequest* url_request) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  BrowserContextIOData* io_data =
      BrowserContextIOData::FromResourceContext(resource_context);

  DownloadRequestParams params;
  params.url = url;
  params.is_content_initiated = is_content_initiated;
  params.suggested_name = suggested_name;
  params.use_prompt = use_prompt;
  params.referrer = referrer.url;
  params.mime_type = mime_type;
  params.render_process_id = render_process_id;
  params.render_view_id = render_view_id;

  net::HttpRequestHeaders headers;
  std::string user_agent;
  if (url_request->is_pending()) {
    url_request->extra_request_headers().GetHeader(
        net::HttpRequestHeaders::kUserAgent, &params.user_agent);
  } else if (io_data->GetDelegate().get()) {
    scoped_refptr<BrowserContextDelegate> delegate(
        BrowserContextIOData::FromResourceContext(resource_context)->GetDelegate());
    if (delegate.get()) {
      params.user_agent = delegate->GetUserAgentOverride(url);
    }
  }
  if (params.user_agent.empty()) {
    params.user_agent = io_data->GetUserAgent();
  }

  if (url_request->is_pending()) {
    std::string cookies;
    url_request->extra_request_headers().GetHeader(
        net::HttpRequestHeaders::kCookie, &cookies);
    DispatchDownloadRequestWithCookies(params, cookies);
  } else if (io_data && io_data->CanAccessCookies(url, first_party_url, false)) {
    net::CookieStore* cookie_store =
        GetCookieStoreForContext(resource_context);
    if (cookie_store) {
      net::CookieOptions cookie_options;
      cookie_options.set_include_httponly();

      cookie_store->GetCookiesWithOptionsAsync(
          url, cookie_options,
          base::Bind(&ResourceDispatcherHostDelegate::DispatchDownloadRequestWithCookies,
          params));
      return;
    }
  }

  // Fallback with default with empty cookies
  DispatchDownloadRequestWithCookies(params, std::string());
}

void ResourceDispatcherHostDelegate::DispatchDownloadRequestWithCookies(
    const DownloadRequestParams & params,
    const std::string& cookies) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&ResourceDispatcherHostDelegate::DispatchDownloadRequestWithCookies,
                   params, cookies));
    return;
  }
  content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(
          params.render_process_id, params.render_view_id);
  if (!rvh) {
    LOG(ERROR) << "Invalid or non-existent render_process_id & render_view_id:"
               << params.render_process_id << ", " << params.render_view_id
               << "during download url delegate dispatch";
    return;
  }

  WebView* webview = WebView::FromRenderViewHost(rvh);
  if (!webview) {
    return;
  }
  webview->DownloadRequested(
    params.url,
    params.mime_type,
    params.use_prompt,
    params.suggested_name,
    cookies,
    params.referrer.spec(),
    params.user_agent);
}

net::CookieStore* ResourceDispatcherHostDelegate::GetCookieStoreForContext(
    content::ResourceContext* resource_context) {
  return resource_context && resource_context->GetRequestContext()
      ? resource_context->GetRequestContext()->cookie_store()
      : nullptr;
}

void ResourceDispatcherHostDelegate::RequestBeginning(
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    content::AppCacheService* appcache_service,
    content::ResourceType resource_type,
    ScopedVector<content::ResourceThrottle>* throttles) {
  throttles->push_back(
      new RedirectionInterceptThrottle(request, resource_context));
}

bool ResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    int route_id,
    bool is_main_frame,
    ui::PageTransition page_transition,
    bool has_user_gesture) {
  return BrowserPlatformIntegration::GetInstance()->LaunchURLExternally(url);
}

content::ResourceDispatcherHostLoginDelegate*
    ResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info,
    net::URLRequest* request) {
  // Chromium will own the delegate
  return new ResourceDispatcherHostLoginDelegate(auth_info, request);
}

ResourceDispatcherHostDelegate::ResourceDispatcherHostDelegate() {}

ResourceDispatcherHostDelegate::~ResourceDispatcherHostDelegate() {}

ResourceDispatcherHostLoginDelegate::ResourceDispatcherHostLoginDelegate(
    net::AuthChallengeInfo* auth_info,
    net::URLRequest* request)
    : request_(request) {
  if (auth_info) {
    host_ = auth_info->challenger.ToString();
    realm_ = auth_info->realm;
  }

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&ResourceDispatcherHostLoginDelegate::DispatchRequest,
          this));
}

ResourceDispatcherHostLoginDelegate::~ResourceDispatcherHostLoginDelegate() {}

void ResourceDispatcherHostLoginDelegate::SetCancelledCallback(
    const base::Closure& cancelled_callback) {
  cancelled_callback_ = cancelled_callback;
}

void ResourceDispatcherHostLoginDelegate::OnRequestCancelled()
{
  request_ = nullptr;

  if (!cancelled_callback_.is_null()) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        cancelled_callback_);
  }
}

void ResourceDispatcherHostLoginDelegate::Deny() {
  if (!request_) {
    return;
  }

  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegate::Deny, this));
    return;
  }

  request_->CancelAuth();
  content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(
      request_);
  request_ = nullptr;
}

void ResourceDispatcherHostLoginDelegate::Allow(std::string username,
                                                std::string password)
{
  if (!request_) {
    return;
  }

  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegate::Allow, this,
                   username, password));
    return;
  }

  request_->SetAuth(net::AuthCredentials(base::UTF8ToUTF16(username),
                                         base::UTF8ToUTF16(password)));
  content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(
        request_);
  request_ = nullptr;
}

WebView* ResourceDispatcherHostLoginDelegate::GetWebView(
    net::URLRequest* request) {
  int processId;
  int frameId;
  content::ResourceRequestInfo::GetRenderFrameForRequest(request, &processId,
                                                         &frameId);
  content::RenderFrameHost* rfh = content::RenderFrameHost::FromID(processId,
                                                                   frameId);
  if (!rfh) {
    return nullptr;
  }

  content::RenderViewHost* rvh = rfh->GetRenderViewHost();
  if (!rvh) {
    return nullptr;
  }

  return WebView::FromRenderViewHost(rvh);
}

void ResourceDispatcherHostLoginDelegate::DispatchRequest() {
  Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if (!request_) {
    // While we were switching threads the request was cancelled, so we
    // don't need to do anything else
    return;
  }

  WebView* webview = GetWebView(request_);
  if (!webview) {
    // Deny the request if we can not get access to the webview, as there is
    // no other sensible thing to do.
    Deny();
    return;
  }

  webview->BasicAuthenticationRequested(this);
}

std::string ResourceDispatcherHostLoginDelegate::Host() const {
  return host_;
}

std::string ResourceDispatcherHostLoginDelegate::Realm() const {
  return realm_;
}

} // namespace oxide
