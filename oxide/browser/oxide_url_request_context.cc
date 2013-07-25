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

#include "oxide_url_request_context.h"

#include <algorithm>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/threading/worker_pool.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "net/base/cache_type.h"
#include "net/cookies/cookie_monster.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_state.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/server_bound_cert_store.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_job_factory_impl.h"

#include "oxide/common/oxide_constants.h"

#include "oxide_browser_process_main.h"
#include "oxide_io_thread_delegate.h"

namespace {

class DefaultURLRequestContext FINAL : public net::URLRequestContext {
 public:
  DefaultURLRequestContext() :
      storage_(this) {}

  class Storage FINAL {
   public:
    Storage(DefaultURLRequestContext* owner) :
        owner_(owner) {}

    void set_net_log(net::NetLog* net_log) {
      owner_->set_net_log(net_log);
    }

    void set_host_resolver(net::HostResolver* host_resolver) {
      owner_->set_host_resolver(host_resolver);
    }

    void set_cert_verifier(net::CertVerifier* cert_verifier) {
      owner_->set_cert_verifier(cert_verifier);
    }

    void set_server_bound_cert_service(
        net::ServerBoundCertService* server_bound_cert_service) {
      server_bound_cert_service_.reset(server_bound_cert_service);
      owner_->set_server_bound_cert_service(server_bound_cert_service);
    }

    void set_fraudulent_certificate_reporter(
        net::FraudulentCertificateReporter* fraudulent_certificate_reporter) {
      owner_->set_fraudulent_certificate_reporter(
          fraudulent_certificate_reporter);
    }

    void set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory* http_auth_handler_factory) {
      owner_->set_http_auth_handler_factory(http_auth_handler_factory);
    }

    void set_proxy_service(net::ProxyService* proxy_service) {
      owner_->set_proxy_service(proxy_service);
    }

    void set_ssl_config_service(net::SSLConfigService* ssl_config_service) {
      owner_->set_ssl_config_service(ssl_config_service);
    }

    void set_network_delegate(net::NetworkDelegate* network_delegate) {
      owner_->set_network_delegate(network_delegate);
    }

    void set_http_server_properties(
        net::HttpServerProperties* http_server_properties) {
      http_server_properties_.reset(http_server_properties);
      owner_->set_http_server_properties(http_server_properties);
    }

    void set_http_user_agent_settings(
        net::HttpUserAgentSettings* http_user_agent_settings) {
      owner_->set_http_user_agent_settings(http_user_agent_settings);
    }

    void set_cookie_store(net::CookieStore* cookie_store) {
      // We hold a strong reference to this, but net::URLRequestContext
      // already has a scoped_refptr
      owner_->set_cookie_store(cookie_store);
    }

    void set_transport_security_state(
        net::TransportSecurityState* transport_security_state) {
      transport_security_state_.reset(transport_security_state);
      owner_->set_transport_security_state(transport_security_state);
    }

    void set_http_transaction_factory(
        net::HttpTransactionFactory* http_transaction_factory) {
      http_transaction_factory_.reset(http_transaction_factory);
      owner_->set_http_transaction_factory(http_transaction_factory);
    }

    void set_job_factory(net::URLRequestJobFactory* job_factory) {
      job_factory_.reset(job_factory);
      owner_->set_job_factory(job_factory);
    }

    void set_throttler_manager(
        net::URLRequestThrottlerManager* throttler_manager) {
      owner_->set_throttler_manager(throttler_manager);
    }

    net::FtpTransactionFactory* ftp_transaction_factory() const {
      return ftp_transaction_factory_.get();
    }

    void set_ftp_transaction_factory(
        net::FtpTransactionFactory* ftp_transaction_factory) {
      ftp_transaction_factory_.reset(ftp_transaction_factory);
    }

   private:
    DefaultURLRequestContext* owner_;

    // This needs to outlive job_factory_
    scoped_ptr<net::FtpTransactionFactory> ftp_transaction_factory_;

    scoped_ptr<net::ServerBoundCertService> server_bound_cert_service_;
    scoped_ptr<net::HttpServerProperties> http_server_properties_;
    scoped_ptr<net::TransportSecurityState> transport_security_state_;
    scoped_ptr<net::HttpTransactionFactory> http_transaction_factory_;
    scoped_ptr<net::URLRequestJobFactory> job_factory_;
  };

  Storage* storage() { return &storage_; }

 private:
  Storage storage_;

  DISALLOW_COPY_AND_ASSIGN(DefaultURLRequestContext);
};

class DefaultURLRequestContextGetter FINAL :
    public oxide::URLRequestContextGetter {
 public:
  DefaultURLRequestContextGetter(
      content::ProtocolHandlerMap* protocol_handlers,
      base::FilePath data_path,
      base::FilePath cache_path,
      bool is_incognito) :
      data_path_(data_path),
      cache_path_(cache_path),
      is_incognito_(is_incognito) {
    std::swap(protocol_handlers_, *protocol_handlers);
  }

  net::URLRequestContext* GetURLRequestContext() OVERRIDE {
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (!url_request_context_) {
      DefaultURLRequestContext* context = new DefaultURLRequestContext();

      oxide::IOThreadDelegate* io_thread_delegate =
          oxide::BrowserProcessMain::io_thread_delegate();

      context->storage()->set_net_log(io_thread_delegate->net_log());
      context->storage()->set_host_resolver(
          io_thread_delegate->host_resolver());
      context->storage()->set_cert_verifier(
          io_thread_delegate->cert_verifier());
      context->storage()->set_http_auth_handler_factory(
          io_thread_delegate->http_auth_handler_factory());
      context->storage()->set_proxy_service(
          io_thread_delegate->proxy_service());
      context->storage()->set_ssl_config_service(
          io_thread_delegate->ssl_config_service());
      context->storage()->set_network_delegate(
          io_thread_delegate->network_delegate());
      context->storage()->set_http_user_agent_settings(
          io_thread_delegate->http_user_agent_settings());
      context->storage()->set_throttler_manager(
          io_thread_delegate->throttler_manager());

      // TODO: We want persistent storage here (for non-incognito), but 
      //       SQLiteServerBoundCertStore is part of chrome
      context->storage()->set_server_bound_cert_service(
          new net::ServerBoundCertService(
              new net::DefaultServerBoundCertStore(NULL),
              base::WorkerPool::GetTaskRunner(true)));

      // TODO: We want persistent storage here (for non-incognito), but the
      //       persistent implementation used in Chrome uses the preferences
      //       system, which we don't want. We need our own implementation,
      //       backed either by sqlite or a text file
      context->storage()->set_http_server_properties(
          new net::HttpServerPropertiesImpl());

      if (is_incognito_) {
        context->storage()->set_cookie_store(
            new net::CookieMonster(NULL, NULL));
      } else {
        DCHECK(!data_path_.empty());
        context->storage()->set_cookie_store(
            content::CreatePersistentCookieStore(
                data_path_.Append(oxide::kCookiesFilename),
                false,
                NULL,
                NULL));
      }

      context->storage()->set_transport_security_state(
          new net::TransportSecurityState());
      // TODO: Need to implement net::TransportSecurityState::Delegate in order
      //       to have persistence for non-incognito mode. There is an
      //       implementation in Chrome which is backed by a json file

      net::HttpCache::BackendFactory* cache_backend = NULL;
      if (is_incognito_) {
        cache_backend = net::HttpCache::DefaultBackend::InMemory(0);
      } else {
        DCHECK(!cache_path_.empty());
        cache_backend = new net::HttpCache::DefaultBackend(
              net::DISK_CACHE,
              net::CACHE_BACKEND_DEFAULT,
              cache_path_.Append(oxide::kCacheDirname),
              0, // Use the default max size
              content::BrowserThread::GetMessageLoopProxyForThread(
                  content::BrowserThread::CACHE));
      }

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
      session_params.http_server_properties =
          context->http_server_properties();
      session_params.net_log = context->net_log();

      context->storage()->set_http_transaction_factory(
          new net::HttpCache(session_params, cache_backend));

      net::URLRequestJobFactoryImpl* job_factory =
          new net::URLRequestJobFactoryImpl();
      context->storage()->set_job_factory(job_factory);

      bool set_protocol = false;
      for (content::ProtocolHandlerMap::iterator it =
               protocol_handlers_.begin();
           it != protocol_handlers_.end();
           ++it) {
        set_protocol = job_factory->SetProtocolHandler(it->first,
                                                       it->second.release());
        DCHECK(set_protocol);
      }

      set_protocol = job_factory->SetProtocolHandler(
          oxide::kFileScheme,
          new net::FileProtocolHandler());
      DCHECK(set_protocol);
      set_protocol = job_factory->SetProtocolHandler(
          oxide::kDataScheme, new net::DataProtocolHandler());
      DCHECK(set_protocol);

      context->storage()->set_ftp_transaction_factory(
          new net::FtpNetworkLayer(context->host_resolver()));
      set_protocol = job_factory->SetProtocolHandler(
          oxide::kFtpScheme, new net::FtpProtocolHandler(
              context->storage()->ftp_transaction_factory()));
      DCHECK(set_protocol);

      url_request_context_.reset(context);

      protocol_handlers_.clear();
      data_path_.clear();
      cache_path_.clear();
    }

    return url_request_context_.get();
  }

 private:
  content::ProtocolHandlerMap protocol_handlers_;
  base::FilePath data_path_;
  base::FilePath cache_path_;
  bool is_incognito_;

  DISALLOW_COPY_AND_ASSIGN(DefaultURLRequestContextGetter);
};

}

namespace oxide {

URLRequestContextGetter::URLRequestContextGetter() {}

// static
URLRequestContextGetter* URLRequestContextGetter::Create(
    content::ProtocolHandlerMap* protocol_handlers,
    base::FilePath data_path,
    base::FilePath cache_path,
    bool is_incognito) {
  return new DefaultURLRequestContextGetter(protocol_handlers,
                                            data_path,
                                            cache_path,
                                            is_incognito);
}

scoped_refptr<base::SingleThreadTaskRunner>
URLRequestContextGetter::GetNetworkTaskRunner() const {
  return content::BrowserThread::GetMessageLoopProxyForThread(
      content::BrowserThread::IO);
}

} // namespace oxide
