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

#include "oxide_network_delegate.h"

#include "base/memory/ref_counted.h"
#include "net/base/net_errors.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_user_agent_settings.h"

namespace oxide {

namespace {
const char kDoNotTrackHeaderName[] = "DNT";
}

int NetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return net::OK;
  }

  bool do_not_track = context_->GetDoNotTrack();
  if (do_not_track) {
    request->SetExtraRequestHeaderByName(
        kDoNotTrackHeaderName, "1", true);
  }

  return delegate->OnBeforeURLRequest(request, callback, new_url);
}

void NetworkDelegate::OnResolveProxy(const GURL& url,
                                     int load_flags,
                                     const net::ProxyService& proxy_service,
                                     net::ProxyInfo* result) {}

void NetworkDelegate::OnProxyFallback(const net::ProxyServer& bad_proxy,
                                      int net_error) {}

int NetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return net::OK;
  }

  return delegate->OnBeforeSendHeaders(request, callback, headers);
}

void NetworkDelegate::OnBeforeSendProxyHeaders(
    net::URLRequest* request,
    const net::ProxyInfo& proxy_info,
    net::HttpRequestHeaders* headers) {}

void NetworkDelegate::OnSendHeaders(net::URLRequest* request,
                                    const net::HttpRequestHeaders& headers) {}

int NetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {
  scoped_refptr<BrowserContextDelegate> delegate(context_->GetDelegate());
  if (!delegate.get()) {
    return net::OK;
  }

  return delegate->OnHeadersReceived(
      request, callback, original_response_headers, override_response_headers);
}

void NetworkDelegate::OnBeforeRedirect(net::URLRequest* request,
                                       const GURL& new_location) {
  std::string user_agent =
      context_->GetUserAgentSettings()
        ->GetUserAgentOverrideForURL(new_location);
  if (!user_agent.empty()) {
    request->SetExtraRequestHeaderByName(net::HttpRequestHeaders::kUserAgent,
                                         user_agent, true);
  }
}

void NetworkDelegate::OnResponseStarted(net::URLRequest* request) {}

void NetworkDelegate::OnNetworkBytesReceived(const net::URLRequest& request,
                                             int64_t bytes_received) {}

void NetworkDelegate::OnNetworkBytesSent(const net::URLRequest& request,
                                         int64_t bytes_sent) {}

void NetworkDelegate::OnCompleted(net::URLRequest* request, bool started) {}

void NetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request) {}

void NetworkDelegate::OnURLRequestJobOrphaned(net::URLRequest* request) {}

void NetworkDelegate::OnPACScriptError(int line_number,
                                       const base::string16& error) {}

net::NetworkDelegate::AuthRequiredResponse NetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) {
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool NetworkDelegate::OnCanGetCookies(const net::URLRequest& request,
                                      const net::CookieList& cookie_list) {
  return context_->CanAccessCookies(request.url(),
                                    request.first_party_for_cookies(),
                                    false);
}

bool NetworkDelegate::OnCanSetCookie(const net::URLRequest& request,
                                     const std::string& cookie_line,
                                     net::CookieOptions* options) {
  return context_->CanAccessCookies(request.url(),
                                    request.first_party_for_cookies(),
                                    true);
}

bool NetworkDelegate::OnCanAccessFile(const net::URLRequest& request,
                                      const base::FilePath& path) const {
  return true;
}

bool NetworkDelegate::OnCanEnablePrivacyMode(
    const GURL& url,
    const GURL& first_party_for_cookies) const {
  bool cookie_read_allowed =
      context_->CanAccessCookies(url, first_party_for_cookies, false);
  bool cookie_write_allowed =
      context_->CanAccessCookies(url, first_party_for_cookies, true);
  return !(cookie_read_allowed && cookie_write_allowed);
}

bool NetworkDelegate::OnAreExperimentalCookieFeaturesEnabled() const {
  return false;
}

bool NetworkDelegate::OnCancelURLRequestWithPolicyViolatingReferrerHeader(
    const net::URLRequest& request,
    const GURL& target_url,
    const GURL& referrer_url) const {
  return true;
}

NetworkDelegate::NetworkDelegate(BrowserContextIOData* context) :
    context_(context) {}

} // namespace oxide
