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

#include "oxide_io_thread_globals.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/net_log.h"
#include "net/cert/cert_verifier.h"
#include "net/dns/host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/proxy/proxy_service.h"
#include "net/url_request/url_request_throttler_manager.h"

namespace oxide {

IOThreadGlobals::Data::Data(IOThreadGlobals* owner) :
    owner_(owner) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

  host_resolver_ = net::HostResolver::CreateDefaultResolver(owner->net_log());
  cert_verifier_.reset(net::CertVerifier::CreateDefault());
  http_auth_handler_factory_.reset(
      net::HttpAuthHandlerFactory::CreateDefault(host_resolver_.get()));

  net::ProxyConfigService* proxy_config_service =
      net::ProxyService::CreateSystemProxyConfigService(
          content::BrowserThread::GetMessageLoopProxyForThread(
              content::BrowserThread::IO),
          content::BrowserThread::UnsafeGetMessageLoopForThread(
              content::BrowserThread::FILE));

  proxy_service_.reset(
      net::ProxyService::CreateUsingSystemProxyResolver(proxy_config_service,
                                                        4,
                                                        owner_->net_log()));

  throttler_manager_.reset(new net::URLRequestThrottlerManager());
}

IOThreadGlobals::Data::~Data() {
  DCHECK(CalledOnValidThread());
}

net::HostResolver* IOThreadGlobals::Data::host_resolver() const {
  DCHECK(CalledOnValidThread());

  return host_resolver_.get();
}

net::CertVerifier* IOThreadGlobals::Data::cert_verifier() const {
  DCHECK(CalledOnValidThread());

  return cert_verifier_.get();
}

net::HttpAuthHandlerFactory* 
IOThreadGlobals::Data::http_auth_handler_factory() const {
  DCHECK(CalledOnValidThread());

  return http_auth_handler_factory_.get();
}

net::ProxyService* IOThreadGlobals::Data::proxy_service() const {
  DCHECK(CalledOnValidThread());

  return proxy_service_.get();
}

net::URLRequestThrottlerManager*
IOThreadGlobals::Data::throttler_manager() const {
  DCHECK(CalledOnValidThread());

  return throttler_manager_.get();
}

IOThreadGlobals::IOThreadGlobals() :
    net_log_(new net::NetLog()),
    data_(NULL) {
  DCHECK(!content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO)) <<
      "IOThreadGlobals cannot be created after the IO thread has started";

  content::BrowserThread::SetDelegate(content::BrowserThread::IO, this);
}

// static
IOThreadGlobals* IOThreadGlobals::GetInstance() {
  return Singleton<IOThreadGlobals>::get();
}

IOThreadGlobals::~IOThreadGlobals() {
  DCHECK(!data_) << "We're being deleted before Cleanup() was called";

  content::BrowserThread::SetDelegate(content::BrowserThread::IO, NULL);
}

void IOThreadGlobals::Init() {
  DCHECK(!data_);

  data_ = new Data(this);
}

void IOThreadGlobals::InitAsync() {}

void IOThreadGlobals::CleanUp() {
  DCHECK(data_);

  delete data_;
  data_ = NULL;
}

} // namespace oxide
