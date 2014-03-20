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

#ifndef _OXIDE_SHARED_BROWSER_IO_THREAD_GLOBALS_H_
#define _OXIDE_SHARED_BROWSER_IO_THREAD_GLOBALS_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/threading/non_thread_safe.h"
#include "content/public/browser/browser_thread_delegate.h"

template <typename T> struct DefaultSingletonTraits;

namespace net {

class CertVerifier;
class HostResolver;
class HttpAuthHandlerFactory;
class NetLog;
class ProxyService;
class URLRequestContextGetter;
class URLRequestThrottlerManager;

} // namespace net

namespace oxide {

class URLRequestContext;
class URLRequestContextGetter;

// This object manages the lifetime of objects that are tied to the
// IO thread
class IOThreadGlobals FINAL : public content::BrowserThreadDelegate {
 public:
  static IOThreadGlobals* GetInstance();
  ~IOThreadGlobals();

  net::NetLog* net_log() const {
    return data_->net_log();
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

  net::URLRequestThrottlerManager* throttler_manager() const {
    return data_->throttler_manager();
  }

  net::URLRequestContextGetter* GetSystemURLRequestContext();

  void InitializeSystemURLRequestContext(
      scoped_ptr<URLRequestContext> request_context);

 private:
   friend struct DefaultSingletonTraits<IOThreadGlobals>;

  class Data FINAL : public base::NonThreadSafe {
    friend class IOThreadGlobals;

    Data();
    ~Data();

    void InitializeRequestContext(scoped_ptr<URLRequestContext> request_context);

    net::NetLog* net_log() const;
    net::HostResolver* host_resolver() const;
    net::CertVerifier* cert_verifier() const;
    net::HttpAuthHandlerFactory* http_auth_handler_factory() const;
    net::ProxyService* proxy_service() const;
    net::URLRequestThrottlerManager* throttler_manager() const;

    scoped_ptr<net::NetLog> net_log_;
    // host_resolver_ needs to outlive http_auth_handler_factory_
    scoped_ptr<net::HostResolver> host_resolver_;
    scoped_ptr<net::CertVerifier> cert_verifier_;
    scoped_ptr<net::HttpAuthHandlerFactory> http_auth_handler_factory_;
    scoped_ptr<net::ProxyService> proxy_service_;
    scoped_ptr<net::URLRequestThrottlerManager> throttler_manager_;

    scoped_ptr<URLRequestContext> system_request_context_;

    DISALLOW_COPY_AND_ASSIGN(Data);
  };

  IOThreadGlobals();

  // Called on the IO thread
  void Init() FINAL;
  void InitAsync() FINAL;
  void CleanUp() FINAL;

  Data* data_;
  scoped_refptr<URLRequestContextGetter> system_request_context_getter_;

  DISALLOW_COPY_AND_ASSIGN(IOThreadGlobals);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_IO_THREAD_GLOBALS_H_
