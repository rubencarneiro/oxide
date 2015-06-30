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

#ifndef _OXIDE_SHARED_BROWSER_RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE_H_

#include "base/callback.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

namespace net {
class URLRequest;
class AuthChallengeInfo;
}

namespace oxide {

class ResourceDispatcherHostDelegate;
class WebView;

class ResourceDispatcherHostLoginDelegate
    : public content::ResourceDispatcherHostLoginDelegate {
 public:
  ResourceDispatcherHostLoginDelegate(net::AuthChallengeInfo* auth_info,
                                      net::URLRequest* request);
  ~ResourceDispatcherHostLoginDelegate() override;
  void OnRequestCancelled() override;

  void Deny();
  void Allow(const std::string& username, const std::string& password);

  void SetCancelledCallback(const base::Closure& cancelled_callback);

  std::string Host() const;
  std::string Realm() const;

private:
  friend class ResourceDispatcherHostDelegate;
  void DispatchRequest(WebView* webview);
  void DispatchCancelledCallback();
  WebView* GetWebView(net::URLRequest* request);

  net::URLRequest* request_;
  std::string host_;
  std::string realm_;
  base::Closure cancelled_callback_;
};

} // namespace oxide

#endif // OXIDE_SHARED_BROWSER_RESOURCE_DISPATCHER_HOST_LOGIN_DELEGATE
