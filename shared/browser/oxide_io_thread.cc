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

#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "net/cert/cert_verifier.h"
#include "net/cert/ct_known_logs.h"
#include "net/cert/ct_log_verifier.h"
#include "net/cert/ct_policy_enforcer.h"
#include "net/cert/multi_log_ct_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_layer.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_state.h"
#include "net/log/net_log.h"
#if defined(OS_LINUX)
#include "net/cert_net/nss_ocsp.h"
#endif
#include "net/proxy/proxy_service.h"
#include "net/ssl/channel_id_service.h"
#include "net/ssl/channel_id_store.h"
#include "net/ssl/default_channel_id_store.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "net/url_request/url_request_throttler_manager.h"

#include "shared/browser/ssl/oxide_ssl_config_service.h"

#include "oxide_browser_platform_integration.h"
#include "oxide_url_request_context.h"

namespace oxide {

namespace {

IOThread* g_instance;

class SystemURLRequestContextGetter final : public URLRequestContextGetter {
 public:
  SystemURLRequestContextGetter() {}

  net::URLRequestContext* GetURLRequestContext() final {
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return IOThread::instance()->globals()->system_request_context();
  }
};

}

IOThread::Globals::Globals() {}

IOThread::Globals::~Globals() {
  DCHECK(CalledOnValidThread());
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

net::CTVerifier* IOThread::Globals::cert_transparency_verifier() const {
  DCHECK(CalledOnValidThread());

  return cert_transparency_verifier_.get();
}

net::CTPolicyEnforcer* IOThread::Globals::ct_policy_enforcer() const {
  DCHECK(CalledOnValidThread());

  return ct_policy_enforcer_.get();
}

URLRequestContext* IOThread::Globals::system_request_context() const {
  DCHECK(CalledOnValidThread());

  return system_request_context_.get();
}

void IOThread::InitSystemRequestContext() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (system_request_context_getter_.get()) {
    return;
  }

  system_request_context_getter_ = new SystemURLRequestContextGetter();

  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&IOThread::InitSystemRequestContextOnIOThread,
                 // |this| is only deleted after the IO thread has stopped
                 // processing events
                 base::Unretained(this)));
}

void IOThread::InitSystemRequestContextOnIOThread() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  globals()->system_request_context_.reset(new URLRequestContext());
  URLRequestContext* context = globals()->system_request_context_.get();
  net::URLRequestContextStorage* storage = context->storage();

#if defined(OS_LINUX)
  net::SetURLRequestContextForNSSHttpIO(context);
#endif

  storage->set_ssl_config_service(new SSLConfigService());
  storage->set_channel_id_service(
      base::WrapUnique(new net::ChannelIDService(
          new net::DefaultChannelIDStore(nullptr),
          base::WorkerPool::GetTaskRunner(true))));
  storage->set_http_server_properties(
      base::WrapUnique(new net::HttpServerPropertiesImpl()));
  storage->set_cookie_store(
      base::WrapUnique(new net::CookieMonster(nullptr, nullptr)));
  storage->set_transport_security_state(
      base::WrapUnique(new net::TransportSecurityState()));

  net::HttpNetworkSession::Params session_params;
  session_params.host_resolver = context->host_resolver();
  session_params.cert_verifier = context->cert_verifier();
  session_params.channel_id_service = context->channel_id_service();
  session_params.transport_security_state =
      context->transport_security_state();
  session_params.cert_transparency_verifier =
      context->cert_transparency_verifier();
  session_params.ct_policy_enforcer = context->ct_policy_enforcer();
  session_params.proxy_service = context->proxy_service();
  session_params.ssl_config_service = context->ssl_config_service();
  session_params.http_auth_handler_factory =
      context->http_auth_handler_factory();
  session_params.http_server_properties = context->http_server_properties();
  session_params.net_log = context->net_log();

  storage->set_http_transaction_factory(
      base::WrapUnique(new net::HttpNetworkLayer(
        new net::HttpNetworkSession(session_params))));

  storage->set_job_factory(
      base::WrapUnique(new net::URLRequestJobFactoryImpl()));
}

void IOThread::Init() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

  BrowserPlatformIntegration::GetInstance()->BrowserThreadInit(
      content::BrowserThread::IO);

#if defined(OS_LINUX)
  net::SetMessageLoopForNSSHttpIO();
#endif

  DCHECK(!globals_);
  globals_ = new Globals();

  globals()->host_resolver_ =
      net::HostResolver::CreateDefaultResolver(
        IOThread::instance()->net_log());
  globals()->cert_verifier_ = net::CertVerifier::CreateDefault();
  globals()->http_auth_handler_factory_ =
      net::HttpAuthHandlerFactory::CreateDefault(
        globals()->host_resolver_.get());

  std::unique_ptr<net::ProxyConfigService> proxy_config_service =
      net::ProxyService::CreateSystemProxyConfigService(
          content::BrowserThread::GetMessageLoopProxyForThread(
              content::BrowserThread::IO),
          content::BrowserThread::GetMessageLoopProxyForThread(
              content::BrowserThread::FILE));

  globals()->proxy_service_ =
      net::ProxyService::CreateUsingSystemProxyResolver(
        std::move(proxy_config_service), 4, IOThread::instance()->net_log());

  globals()->throttler_manager_ =
      base::MakeUnique<net::URLRequestThrottlerManager>();

  std::vector<scoped_refptr<const net::CTLogVerifier>> ct_logs =
      net::ct::CreateLogVerifiersForKnownLogs();
  std::unique_ptr<net::MultiLogCTVerifier> ct_verifier =
      base::MakeUnique<net::MultiLogCTVerifier>();
  ct_verifier->AddLogs(ct_logs);

  globals()->cert_transparency_verifier_ = std::move(ct_verifier);
  globals()->ct_policy_enforcer_ = base::MakeUnique<net::CTPolicyEnforcer>();

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&IOThread::InitSystemRequestContext,
                 // |this| is only deleted after we've stopped processing
                 // events on the UI thread
                 base::Unretained(this)));
}

void IOThread::CleanUp() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

#if defined(OS_LINUX)
  net::ShutdownNSSHttpIO();
  net::SetURLRequestContextForNSSHttpIO(nullptr);
#endif

  DCHECK(globals_);
  delete globals_;
  globals_ = nullptr;

  system_request_context_getter_ = nullptr;

  BrowserPlatformIntegration::GetInstance()->BrowserThreadCleanUp(
      content::BrowserThread::IO);
}

// static
IOThread* IOThread::instance() {
  DCHECK(g_instance);
  return g_instance;
}

IOThread::IOThread()
    : net_log_(new net::NetLog()),
      globals_(nullptr) {
  CHECK(!g_instance) << "Can't create more than one IOThread instance";
  DCHECK(!content::BrowserThread::IsThreadInitialized(content::BrowserThread::IO)) <<
      "IOThread cannot be created after the IO thread has started";

  g_instance = this;
  content::BrowserThread::SetDelegate(content::BrowserThread::IO, this);
}

IOThread::~IOThread() {
  DCHECK_EQ(g_instance, this);
  DCHECK(!globals_) << "We're being deleted before Cleanup() was called";

  g_instance = nullptr;
  content::BrowserThread::SetDelegate(content::BrowserThread::IO, nullptr);
}

net::NetLog* IOThread::net_log() const {
  return net_log_.get();
}

IOThread::Globals* IOThread::globals() const {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  return globals_;
}

net::URLRequestContextGetter* IOThread::GetSystemURLRequestContext() {
  if (!system_request_context_getter_.get()) {
    InitSystemRequestContext();
  }
  return system_request_context_getter_.get();
}

} // namespace oxide
