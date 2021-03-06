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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DELEGATE_H_

#include "base/memory/ref_counted.h"
#include "net/base/completion_callback.h"
#include "net/base/net_errors.h"

#include "shared/common/oxide_shared_export.h"

class GURL;

namespace base {
template <typename Type> class DeleteHelper;
}

namespace net {
class HttpRequestHeaders;
class HttpResponseHeaders;
class NetworkDelegate;
class URLRequest;
}

namespace oxide {

class BrowserContextDelegate;
class URLRequestDelegatedJob;

struct OXIDE_SHARED_EXPORT BrowserContextDelegateTraits {
  static void Destruct(const BrowserContextDelegate* x);
};

class BrowserContextDelegate :
    public base::RefCountedThreadSafe<BrowserContextDelegate, BrowserContextDelegateTraits> {
 public:
  // Called on the IO thread
  virtual int OnBeforeURLRequest(net::URLRequest* request,
                                 const net::CompletionCallback& callback,
                                 GURL* new_url) {
    return net::OK;
  }

  // Called on the IO thread
  virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                  const net::CompletionCallback& callback,
                                  net::HttpRequestHeaders* headers) {
    return net::OK;
  }

  // Called on the IO thread
  virtual int OnHeadersReceived(
      net::URLRequest* request,
      const net::CompletionCallback& callback,
      const net::HttpResponseHeaders* original_response_headers,
      scoped_refptr<net::HttpResponseHeaders>* override_response_headers) {
    return net::OK;
  }

  // Called on the IO thread
  virtual int OnBeforeRedirect(net::URLRequest* request,
                               const GURL& new_location) {
    return net::OK;
  }

  // Called on the IO thread
  virtual std::string GetUserAgentOverride(const GURL& url) {
    return std::string();
  }

  virtual bool IsCustomProtocolHandlerRegistered(
      const std::string& scheme) const {
    return false;
  }
  virtual URLRequestDelegatedJob* CreateCustomURLRequestJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) {
    return nullptr;
  }

 protected:
  friend class BrowserContextDelegateTraits;
  friend class base::DeleteHelper<BrowserContextDelegate>;

  BrowserContextDelegate() {}
  virtual ~BrowserContextDelegate() {}
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_DELEGATE_H_
