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

#include "net/base/net_errors.h"

namespace oxide {

int NetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  return net::OK;
}

int NetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  return net::OK;
}

void NetworkDelegate::OnSendHeaders(net::URLRequest* request,
                                    const net::HttpRequestHeaders& headers) {}

int NetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers) {
  return net::OK;
}

void NetworkDelegate::OnBeforeRedirect(net::URLRequest* request,
                                       const GURL& new_location) {}

void NetworkDelegate::OnResponseStarted(net::URLRequest* request) {}

void NetworkDelegate::OnRawBytesRead(const net::URLRequest& request,
                                     int bytes_read) {}

void NetworkDelegate::OnCompleted(net::URLRequest* request, bool started) {}

void NetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request) {}

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
  return true;
}

bool NetworkDelegate::OnCanSetCookie(const net::URLRequest& request,
                                     const std::string& cookie_line,
                                     net::CookieOptions* options) {
  return true;
}

bool NetworkDelegate::OnCanAccessFile(const net::URLRequest& request,
                                      const base::FilePath& path) const {
  return true;
}

bool NetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const {
  return false;
}

int NetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) {
  return net::OK;
}

} // namespace oxide
