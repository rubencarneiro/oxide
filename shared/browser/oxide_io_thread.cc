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

#include "oxide_io_thread.h"

#include "base/logging.h"
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

namespace {

IOThread* g_instance;

class SystemURLRequestContextGetter FINAL : public URLRequestContextGetter {
 public:
  SystemURLRequestContextGetter() {}

  net::URLRequestContext* GetURLRequestContext() FINAL {
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return IOThread::instance()->globals()->system_request_context();
  }
};

}

IOThread::Globals::Globals() {
  host_resolver_ =
      net::HostResolver::CreateDefaultResolver(IOThread::instance()->net_log());
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
      net::ProxyService::CreateUsingSystemProxyResolver(
        proxy_config_service, 4, IOThread::instance()->net_log()));

  throttler_manager_.reset(new net::URLRequestThrottlerManager());
}

IOThread::Globals::~Globals() {
  DCHECK(CalledOnValidThread());
}

void IOThread::Globals::Init() {
  DCHECK(CalledOnValidThread());

  system_request_context_.reset(new URLRequestContext());
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
  session_params.channel_id_service = context->channel_id_service();
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

net::HostResolver* IOThread::Globals::host_resolver() const {
  DCHECK(CalledOnValidThread());

  return host_resolver_.get();
}

net::CertVerifier* IOThread::Globals::cert_verifier() const {
  DCHECK(CalledOnValidThread());

  return cert_verifier_.get();
}

net::HttpAuthHandlerFactory*
IOThread::Globals::http_auth_handler_factory() const {
  DCHECK(CalledOnValidThread());

  return http_auth_handler_factory_.get();
}

net::ProxyService* IOThread::Globals::proxy_service() const {
  DCHECK(CalledOnValidThread());

  return proxy_service_.get();
}

net::URLRequestThrottlerManager* IOThread::Globals::throttler_manager() const {
  DCHECK(CalledOnValidThread());

  return throttler_manager_.get();
}

URLRequestContext* IOThread::Globals::system_request_context() const {
  DCHECK(CalledOnValidThread());

  return system_request_context_.get();
}

void IOThread::Init() {
  DCHECK(!globals_);
  globals_ = new Globals();
}

void IOThread::InitAsync() {
  DCHECK(globals_);
  globals_->Init();
}

void IOThread::CleanUp() {
  DCHECK(globals_);

  delete globals_;
  globals_ = NULL;

  system_request_context_getter_ = NULL;
}

// static
IOThread* IOThread::instance() {
  DCHECK(g_instance);
  return g_instance;
}

IOThread::IOThread() :
    net_log_(new net::NetLog()),
    globals_(NULL),
    system_request_context_getter_(new SystemURLRequestContextGetter()) {
  CHECK(!g_instance) << "Can't create more than one IOThread instance";
  DCHECK(!content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO)) <<
      "IOThread cannot be created after the IO thread has started";

  g_instance = this;
  content::BrowserThread::SetDelegate(content::BrowserThread::IO, this);
}

IOThread::~IOThread() {
  DCHECK_EQ(g_instance, this);
  DCHECK(!globals_) << "We're being deleted before Cleanup() was called";

  g_instance = NULL;
  content::BrowserThread::SetDelegate(content::BrowserThread::IO, NULL);
}

net::NetLog* IOThread::net_log() const {
  return net_log_.get();
}

IOThread::Globals* IOThread::globals() const {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  return globals_;
}

net::URLRequestContextGetter* IOThread::GetSystemURLRequestContext() {
  return system_request_context_getter_;
}

} // namespace oxide
