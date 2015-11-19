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

#ifndef _OXIDE_SHARED_BROWSER_NETWORK_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_NETWORK_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "net/base/network_delegate.h"

namespace oxide {

class BrowserContextIOData;

class NetworkDelegate final : public net::NetworkDelegate {
 public:
  NetworkDelegate(BrowserContextIOData* context);

 private:
  int OnBeforeURLRequest(net::URLRequest* request,
                         const net::CompletionCallback& callback,
                         GURL* new_url) final;

  void OnResolveProxy(const GURL& url,
                      int load_flags,
                      const net::ProxyService& proxy_service,
                      net::ProxyInfo* result) final;

  void OnProxyFallback(const net::ProxyServer& bad_proxy, int net_error) final;

  int OnBeforeSendHeaders(net::URLRequest* request,
                          const net::CompletionCallback& callback,
                          net::HttpRequestHeaders* headers) final;

  void OnBeforeSendProxyHeaders(net::URLRequest* request,
                                const net::ProxyInfo& proxy_info,
                                net::HttpRequestHeaders* headers) final;

  void OnSendHeaders(net::URLRequest* request,
                     const net::HttpRequestHeaders& headers) final;

  int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) final;

  void OnBeforeRedirect(net::URLRequest* request,
                        const GURL& new_location) final;

  void OnResponseStarted(net::URLRequest* request) final;

  void OnNetworkBytesReceived(net::URLRequest* request,
                              int64_t bytes_received) final;

  void OnNetworkBytesSent(net::URLRequest* request,
                          int64_t bytes_sent) final;

  void OnCompleted(net::URLRequest* request, bool started) final;

  void OnURLRequestDestroyed(net::URLRequest* request) final;

  void OnURLRequestJobOrphaned(net::URLRequest* request) final;

  void OnPACScriptError(int line_number, const base::string16& error) final;

  AuthRequiredResponse OnAuthRequired(
      net::URLRequest* request,
      const net::AuthChallengeInfo& auth_info,
      const AuthCallback& callback,
      net::AuthCredentials* credentials) final;

  bool OnCanGetCookies(const net::URLRequest& request,
                       const net::CookieList& cookie_list) final;

  bool OnCanSetCookie(const net::URLRequest& request,
                      const std::string& cookie_line,
                      net::CookieOptions* options) final;

  bool OnCanAccessFile(const net::URLRequest& request,
                       const base::FilePath& path) const final;

  bool OnCanEnablePrivacyMode(
      const GURL& url,
      const GURL& first_party_for_cookies) const final;

  bool OnAreExperimentalCookieFeaturesEnabled() const final;

  bool OnCancelURLRequestWithPolicyViolatingReferrerHeader(
      const net::URLRequest& request,
      const GURL& target_url,
      const GURL& referrer_url) const final;

  BrowserContextIOData* context_;

  DISALLOW_COPY_AND_ASSIGN(NetworkDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NETWORK_DELEGATE_H_
