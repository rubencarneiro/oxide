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
#include "base/callback.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/referrer.h"
#include "net/cookies/cookie_monster.h"
#include "net/url_request/url_request_context.h"
#include "url/gurl.h"

#include "oxide_browser_context.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_redirection_intercept_throttle.h"
#include "oxide_web_view.h"
#include "oxide_web_contents_view.h"

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
};

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
    content::ResourceContext* resource_context) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  DownloadRequestParams params;
  params.url = url;
  params.is_content_initiated = is_content_initiated;
  params.suggested_name = suggested_name;
  params.use_prompt = use_prompt;
  params.referrer = referrer.url;
  params.mime_type = mime_type;
  params.render_process_id = render_process_id;
  params.render_view_id = render_view_id;

  BrowserContextIOData* io_data =
      BrowserContextIOData::FromResourceContext(resource_context);

  if (io_data && io_data->CanAccessCookies(url, first_party_url, false)) {
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
    params.referrer.spec());
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
    int route_id) {
  return BrowserPlatformIntegration::GetInstance()->LaunchURLExternally(url);
}

bool ResourceDispatcherHostDelegate::ShouldDownloadUrl(const GURL& url,
    const GURL& first_party_url,
    bool is_content_initiated,
    const base::string16& suggested_name,
    const bool use_prompt,
    const content::Referrer& referrer,
    const std::string& mime_type,
    int render_process_id,
    int render_view_id,
    content::ResourceContext* resource_context) {
  DispatchDownloadRequest(url,
                          first_party_url,
                          is_content_initiated,
                          suggested_name,
                          use_prompt,
                          referrer,
                          mime_type,
                          render_process_id,
                          render_view_id,
                          resource_context);
  return false;
}

content::ResourceDispatcherHostLoginDelegate* ResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info,
    net::URLRequest* request) {
    LOG(ERROR) << "Intercepted ===================================== ";

    LoginPromptDelegate* delegate = new LoginPromptDelegate(auth_info, request);

    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&LoginPromptDelegate::DispatchAuthRequest, delegate));

    LOG(ERROR) << "Returning ===================================== ";

    return delegate;
}

ResourceDispatcherHostDelegate::ResourceDispatcherHostDelegate() {}

ResourceDispatcherHostDelegate::~ResourceDispatcherHostDelegate() {}

LoginPromptDelegate::LoginPromptDelegate(net::AuthChallengeInfo* auth_info,
                                         net::URLRequest* request) :
                                         request_(request) {
}

LoginPromptDelegate::~LoginPromptDelegate() {
    LOG(ERROR) << "Deleted ===================================== ";
}

void LoginPromptDelegate::OnRequestCancelled()
{
    LOG(ERROR) << "Cancelled ===================================== ";
}

void LoginPromptDelegate::DispatchAuthRequest() {
    LOG(ERROR) << "Called DoAuth ===================================== " << request_;

    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    int renderProcessId;
    int renderFrameId;
    content::ResourceRequestInfo::GetRenderFrameForRequest(request_, &renderProcessId,  &renderFrameId);
    content::RenderViewHost* rvh = content::RenderFrameHost::FromID(renderProcessId, renderFrameId)->GetRenderViewHost();
    if (!rvh) {
      LOG(ERROR) << "Invalid or non-existent render_process_id & render_frame_id:"
             << renderProcessId << ", " << renderFrameId
             << "during basic auth request dispatch";
      return;
    }

    WebView* webview = WebView::FromRenderViewHost(rvh);
    if (!webview) {
      return;
    }
    webview->BasicAuthenticationRequested();
}

//void LoginPromptDelegate::DoAuth(bool cancel) {
//    LOG(ERROR) << "Called DoAuth Login ===================================== " << cancel;

//    if (cancel) request_->CancelAuth();
//    else request_->SetAuth(net::AuthCredentials(base::ASCIIToUTF16("admin"),
//                                                base::ASCIIToUTF16("dusty")));
//}
} // namespace oxide
