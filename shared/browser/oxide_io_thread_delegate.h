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

#ifndef _OXIDE_SHARED_BROWSER_IO_THREAD_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_IO_THREAD_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/threading/non_thread_safe.h"
#include "content/public/browser/browser_thread_delegate.h"

namespace net {

class CertVerifier;
class HostResolver;
class HttpAuthHandlerFactory;
class NetLog;
class NetworkDelegate;
class ProxyService;
class URLRequestThrottlerManager;

} // namespace net

namespace oxide {

// This object manages the lifetime of objects that are tied to the
// IO thread
class IOThreadDelegate FINAL : public content::BrowserThreadDelegate {
 public:
  IOThreadDelegate();
  ~IOThreadDelegate();

  net::NetLog* net_log() const {
    return net_log_.get();
  }

  net::HostResolver* host_resolver() const {
    return data_->host_resolver();
  }

  net::CertVerifier* cert_verifier() const {
    return data_->cert_verifier();
  }

  net::HttpAuthHandlerFactory* http_auth_handler_factory() const {
    return data_->http_auth_handler_factory();
  }

  net::ProxyService* proxy_service() const {
    return data_->proxy_service();
  }

  net::NetworkDelegate* network_delegate() const {
    return data_->network_delegate();
  }

  net::URLRequestThrottlerManager* throttler_manager() const {
    return data_->throttler_manager();
  }

  // Called on the IO thread
  void Init() FINAL;

  // Called on the IO thread
  void InitAsync() FINAL;

  // Called on the IO thread
  void CleanUp() FINAL;

 private:

  class Data FINAL : public base::NonThreadSafe {
   public:
    Data(IOThreadDelegate* owner);
    ~Data();

    net::HostResolver* host_resolver() const;
    net::CertVerifier* cert_verifier() const;
    net::HttpAuthHandlerFactory* http_auth_handler_factory() const;
    net::ProxyService* proxy_service() const;
    net::NetworkDelegate* network_delegate() const;
    net::URLRequestThrottlerManager* throttler_manager() const;

   private:
    IOThreadDelegate* owner_;

    // host_resolver_ needs to outlive http_auth_handler_factory_
    scoped_ptr<net::HostResolver> host_resolver_;
    scoped_ptr<net::CertVerifier> cert_verifier_;
    scoped_ptr<net::HttpAuthHandlerFactory> http_auth_handler_factory_;
    scoped_ptr<net::ProxyService> proxy_service_;
    scoped_ptr<net::NetworkDelegate> network_delegate_;
    scoped_ptr<net::URLRequestThrottlerManager> throttler_manager_;

    DISALLOW_IMPLICIT_CONSTRUCTORS(Data);
  };

  scoped_ptr<net::NetLog> net_log_;

  Data* data_;

  DISALLOW_COPY_AND_ASSIGN(IOThreadDelegate);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_IO_THREAD_DELEGATE_H_
