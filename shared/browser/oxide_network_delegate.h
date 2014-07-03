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

class NetworkDelegate FINAL : public net::NetworkDelegate {
 public:
  NetworkDelegate(BrowserContextIOData* context);

 private:
  int OnBeforeURLRequest(net::URLRequest* request,
                         const net::CompletionCallback& callback,
                         GURL* new_url) FINAL;

  int OnBeforeSendHeaders(net::URLRequest* request,
                          const net::CompletionCallback& callback,
                          net::HttpRequestHeaders* headers) FINAL;

  void OnSendHeaders(net::URLRequest* request,
                     const net::HttpRequestHeaders& headers) FINAL;

  int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
      GURL* allowed_unsafe_redirect_url) FINAL;

  void OnBeforeRedirect(net::URLRequest* request,
                        const GURL& new_location) FINAL;

  void OnResponseStarted(net::URLRequest* request) FINAL;

  void OnRawBytesRead(const net::URLRequest& request, int bytes_read) FINAL;

  void OnCompleted(net::URLRequest* request, bool started) FINAL;

  void OnURLRequestDestroyed(net::URLRequest* request) FINAL;

  void OnPACScriptError(int line_number, const base::string16& error) FINAL;

  AuthRequiredResponse OnAuthRequired(
      net::URLRequest* request,
      const net::AuthChallengeInfo& auth_info,
      const AuthCallback& callback,
      net::AuthCredentials* credentials) FINAL;

  bool OnCanGetCookies(const net::URLRequest& request,
                       const net::CookieList& cookie_list) FINAL;

  bool OnCanSetCookie(const net::URLRequest& request,
                      const std::string& cookie_line,
                      net::CookieOptions* options) FINAL;

  bool OnCanAccessFile(const net::URLRequest& request,
                       const base::FilePath& path) const FINAL;

  bool OnCanThrottleRequest(const net::URLRequest& request) const FINAL;

  int OnBeforeSocketStreamConnect(
      net::SocketStream* socket,
      const net::CompletionCallback& callback) FINAL;

  BrowserContextIOData* context_;

  DISALLOW_COPY_AND_ASSIGN(NetworkDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_NETWORK_DELEGATE_H_
