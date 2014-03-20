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
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_layer.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_state.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/url_request_throttler_manager.h"

#include "oxide_url_request_context.h"

namespace oxide {

IOThreadGlobals::Data::Data() {
  net_log_.reset(new net::NetLog());
  host_resolver_ = net::HostResolver::CreateDefaultResolver(net_log_.get());
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
                                                        net_log_.get()));

  throttler_manager_.reset(new net::URLRequestThrottlerManager());
}

IOThreadGlobals::Data::~Data() {
  DCHECK(CalledOnValidThread());
}

net::NetLog* IOThreadGlobals::Data::net_log() const {
  DCHECK(CalledOnValidThread());

  return net_log_.get();
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

void IOThreadGlobals::Data::InitializeRequestContext(
    scoped_ptr<URLRequestContext> request_context) {
  DCHECK(CalledOnValidThread());

  system_request_context_ = request_context.Pass();
  URLRequestContext* context = system_request_context_.get();
  net::URLRequestContextStorage* storage = context->storage();

  storage->set_ssl_config_service(new net::SSLConfigServiceDefaults());
  storage->set_http_server_properties(
      scoped_ptr<net::HttpServerProperties>(
        new net::HttpServerPropertiesImpl()));
  storage->set_cookie_store(new net::CookieMonster(NULL, NULL));
  storage->set_transport_security_state(new net::TransportSecurityState());

  net::HttpNetworkSession::Params session_params;
  session_params.host_resolver = context->host_resolver();
  session_params.cert_verifier = context->cert_verifier();
  session_params.server_bound_cert_service =
      context->server_bound_cert_service();
  session_params.transport_security_state =
      context->transport_security_state();
  session_params.proxy_service = context->proxy_service();
  session_params.ssl_config_service = context->ssl_config_service();
  session_params.http_auth_handler_factory =
      context->http_auth_handler_factory();
  session_params.network_delegate = context->network_delegate();
  session_params.http_server_properties = context->http_server_properties();
  session_params.net_log = context->net_log();

  storage->set_http_transaction_factory(
      new net::HttpNetworkLayer(new net::HttpNetworkSession(session_params)));
}

IOThreadGlobals::IOThreadGlobals() :
    data_(NULL),
    system_request_context_getter_(URLRequestContextGetter::CreateSystem()) {
  DCHECK(!content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO)) <<
      "IOThreadGlobals cannot be created after the IO thread has started";

  content::BrowserThread::SetDelegate(content::BrowserThread::IO, this);
}

void IOThreadGlobals::Init() {
  if (!data_) {
    data_ = new Data();
  }
}

void IOThreadGlobals::InitAsync() {}

void IOThreadGlobals::CleanUp() {
  DCHECK(data_);

  delete data_;
  data_ = NULL;
}

// static
IOThreadGlobals* IOThreadGlobals::GetInstance() {
  return Singleton<IOThreadGlobals>::get();
}

IOThreadGlobals::~IOThreadGlobals() {
  DCHECK(!data_) << "We're being deleted before Cleanup() was called";

  content::BrowserThread::SetDelegate(content::BrowserThread::IO, NULL);
}

net::URLRequestContextGetter* IOThreadGlobals::GetSystemURLRequestContext() {
  return system_request_context_getter_.get();
}

void IOThreadGlobals::InitializeSystemURLRequestContext(
    scoped_ptr<URLRequestContext> request_context) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  Init();

  data_->InitializeRequestContext(request_context.Pass());
}

} // namespace oxide
