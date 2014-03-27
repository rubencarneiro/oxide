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

#include "oxide_browser_context.h"

#include <algorithm>
#include <vector>

#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/supports_user_data.h"
#include "base/threading/worker_pool.h"
#include "content/browser/loader/resource_dispatcher_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_crypto_delegate.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/net_errors.h"
#include "net/cookies/cookie_monster.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_persister.h"
#include "net/http/transport_security_state.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/server_bound_cert_store.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/protocol_intercept_job_factory.h"
#include "net/url_request/url_request_job_factory_impl.h"

#include "shared/common/oxide_constants.h"

#include "oxide_browser_context_impl.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_browser_context_observer.h"
#include "oxide_browser_process_main.h"
#include "oxide_http_user_agent_settings.h"
#include "oxide_io_thread.h"
#include "oxide_network_delegate.h"
#include "oxide_off_the_record_browser_context_impl.h"
#include "oxide_ssl_config_service.h"
#include "oxide_url_request_context.h"

namespace oxide {

namespace {

base::LazyInstance<std::vector<BrowserContext *> > g_all_contexts;

const char kBrowserContextKey[] = "oxide_browser_context_data";

class ContextData : public base::SupportsUserData::Data {
 public:
  ContextData(BrowserContextIOData* context) :
      context_(context) {}
  virtual ~ContextData() {}

  BrowserContextIOData* context() const { return context_; }

 private:
  BrowserContextIOData* context_;
};

class MainURLRequestContextGetter FINAL : public URLRequestContextGetter {
 public:
  MainURLRequestContextGetter(
      BrowserContextIOData* context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors) :
      context_(context),
      protocol_interceptors_(protocol_interceptors.Pass()) {
    std::swap(protocol_handlers_, *protocol_handlers);
  }

  net::URLRequestContext* GetURLRequestContext() FINAL {
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (!url_request_context_) {
      DCHECK(context_);
      url_request_context_ = context_->CreateMainRequestContext(
          protocol_handlers_, protocol_interceptors_.Pass())->AsWeakPtr();
      context_ = NULL;
    }

    return url_request_context_.get();
  }

 private:
  BrowserContextIOData* context_;
  content::ProtocolHandlerMap protocol_handlers_;
  content::ProtocolHandlerScopedVector protocol_interceptors_;

  base::WeakPtr<URLRequestContext> url_request_context_;
};

} // namespace

class ResourceContext FINAL : public content::ResourceContext {
 public:
  ResourceContext() :
      request_context_(NULL) {}

  net::HostResolver* GetHostResolver() FINAL {
    return IOThread::instance()->globals()->host_resolver();
  }

  net::URLRequestContext* GetRequestContext() FINAL {
    CHECK(request_context_);
    return request_context_;
  }

  bool AllowMicAccess(const GURL& origin) FINAL {
    return false;
  }

  bool AllowCameraAccess(const GURL& origin) FINAL {
    return false;
  }

 private:
  friend class BrowserContextIOData;

  net::URLRequestContext* request_context_;

  DISALLOW_COPY_AND_ASSIGN(ResourceContext);
};

// static
void BrowserContextDelegateTraits::Destruct(const BrowserContextDelegate* x) {
  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    delete x;
    return;
  }

  if (!content::BrowserThread::DeleteSoon(content::BrowserThread::UI,
                                          FROM_HERE, x)) {
    LOG(ERROR) <<
        "BrowserContextDelegate won't be deleted. This could be due to it "
        "being leaked until after Chromium shutdown has begun";
  }
}

void BrowserContextIOData::SetDelegate(BrowserContextDelegate* delegate) {
  base::AutoLock lock(delegate_lock_);
  delegate_ = delegate;
}

void BrowserContextIOData::SetCookiePolicy(net::StaticCookiePolicy::Type policy) {
  base::AutoLock lock(cookie_policy_lock_);
  cookie_policy_.set_type(policy);
}

void BrowserContextIOData::SetSessionCookieMode(
    content::CookieStoreConfig::SessionCookieMode mode) {
  session_cookie_mode_ = mode;
}

BrowserContextIOData::BrowserContextIOData() :
    cookie_policy_(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
    session_cookie_mode_(content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
    resource_context_(new ResourceContext()) {
  resource_context_->SetUserData(kBrowserContextKey, new ContextData(this));
}

BrowserContextIOData::~BrowserContextIOData() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
}

// static
BrowserContextIOData* BrowserContextIOData::FromResourceContext(
    content::ResourceContext* resource_context) {
  return static_cast<ContextData *>(
      resource_context->GetUserData(kBrowserContextKey))->context();
}

scoped_refptr<BrowserContextDelegate> BrowserContextIOData::GetDelegate() {
  base::AutoLock lock(delegate_lock_);
  return delegate_;
}

net::StaticCookiePolicy::Type BrowserContextIOData::GetCookiePolicy() const {
  return cookie_policy_.type();
}

content::CookieStoreConfig::SessionCookieMode
BrowserContextIOData::GetSessionCookieMode() const {
  return session_cookie_mode_;
}

URLRequestContext* BrowserContextIOData::CreateMainRequestContext(
    content::ProtocolHandlerMap& protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  DCHECK(!main_request_context_);

  IOThread::Globals* io_thread_globals = IOThread::instance()->globals();

  ssl_config_service_ = new SSLConfigService();
  http_user_agent_settings_.reset(new HttpUserAgentSettings(this));
  ftp_transaction_factory_.reset(
      new net::FtpNetworkLayer(io_thread_globals->host_resolver()));

  // TODO: We want persistent storage here (for non-incognito), but the
  //       persistent implementation used in Chrome uses the preferences
  //       system, which we don't want. We need our own implementation,
  //       backed either by sqlite or a text file
  http_server_properties_.reset(new net::HttpServerPropertiesImpl());

  network_delegate_.reset(new NetworkDelegate(this));
  transport_security_state_.reset(new net::TransportSecurityState());
  transport_security_persister_.reset(
      new net::TransportSecurityPersister(
        transport_security_state_.get(),
        GetPath(),
        content::BrowserThread::GetMessageLoopProxyForThread(
          content::BrowserThread::FILE),
        IsOffTheRecord()));

  main_request_context_.reset(new URLRequestContext());
  URLRequestContext* context = main_request_context_.get();
  net::URLRequestContextStorage* storage = context->storage();

  storage->set_ssl_config_service(ssl_config_service_.get());
  context->set_network_delegate(network_delegate_.get());
  context->set_http_user_agent_settings(http_user_agent_settings_.get());

  // TODO: We want persistent storage here (for non-incognito), but 
  //       SQLiteServerBoundCertStore is part of chrome
  storage->set_server_bound_cert_service(
      new net::ServerBoundCertService(
          new net::DefaultServerBoundCertStore(NULL),
          base::WorkerPool::GetTaskRunner(true)));

  context->set_http_server_properties(http_server_properties_->GetWeakPtr());

  base::FilePath cookie_path;
  if (!IsOffTheRecord() && !GetPath().empty()) {
    cookie_path = GetPath().Append(kCookiesFilename);
  }
  storage->set_cookie_store(content::CreateCookieStore(
      content::CookieStoreConfig(
        cookie_path,
        GetSessionCookieMode(),
        NULL, NULL)));

  context->set_transport_security_state(transport_security_state_.get());

  net::HttpCache::BackendFactory* cache_backend = NULL;
  if (IsOffTheRecord() || GetCachePath().empty()) {
    cache_backend = net::HttpCache::DefaultBackend::InMemory(0);
  } else {
    cache_backend = new net::HttpCache::DefaultBackend(
          net::DISK_CACHE,
          net::CACHE_BACKEND_DEFAULT,
          GetCachePath().Append(kCacheDirname),
          83886080, // XXX: 80MB - Make this configurable
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

  storage->set_http_transaction_factory(
      new net::HttpCache(session_params, cache_backend));

  scoped_ptr<net::URLRequestJobFactoryImpl> job_factory(
      new net::URLRequestJobFactoryImpl());

  bool set_protocol = false;
  for (content::ProtocolHandlerMap::iterator it = protocol_handlers.begin();
       it != protocol_handlers.end();
       ++it) {
    set_protocol = job_factory->SetProtocolHandler(it->first,
                                                   it->second.release());
    DCHECK(set_protocol);
  }
  protocol_handlers.clear();

  set_protocol = job_factory->SetProtocolHandler(
      oxide::kFileScheme,
      new net::FileProtocolHandler(
        content::BrowserThread::GetMessageLoopProxyForThread(
          content::BrowserThread::FILE)));
  DCHECK(set_protocol);
  set_protocol = job_factory->SetProtocolHandler(
      oxide::kDataScheme, new net::DataProtocolHandler());
  DCHECK(set_protocol);

  set_protocol = job_factory->SetProtocolHandler(
      oxide::kFtpScheme,
      new net::FtpProtocolHandler(ftp_transaction_factory_.get()));
  DCHECK(set_protocol);

  scoped_ptr<net::URLRequestJobFactory> top_job_factory(
      job_factory.PassAs<net::URLRequestJobFactory>());

  for (content::ProtocolHandlerScopedVector::reverse_iterator it =
         protocol_interceptors.rbegin();
       it != protocol_interceptors.rend();
       ++it) {
    top_job_factory.reset(new net::ProtocolInterceptJobFactory(
        top_job_factory.Pass(), make_scoped_ptr(*it)));
  }
  protocol_interceptors.weak_clear();

  storage->set_job_factory(top_job_factory.release());

  resource_context_->request_context_ = context;
  return main_request_context_.get();
}

content::ResourceContext* BrowserContextIOData::GetResourceContext() {
  return resource_context_.get();
}

bool BrowserContextIOData::CanAccessCookies(const GURL& url,
                                            const GURL& first_party_url,
                                            bool write) {
  scoped_refptr<BrowserContextDelegate> delegate(GetDelegate());
  if (delegate) {
    StoragePermission res =
        delegate->CanAccessStorage(url, first_party_url, write,
                                   STORAGE_TYPE_COOKIES);
    if (res != STORAGE_PERMISSION_UNDEFINED) {
      return res == STORAGE_PERMISSION_ALLOW;
    }
  }

  base::AutoLock lock(cookie_policy_lock_);
  if (write) {
    return cookie_policy_.CanSetCookie(url, first_party_url) == net::OK;
  }

  return cookie_policy_.CanGetCookies(url, first_party_url) == net::OK;
}

BrowserContext::IODataHandle::~IODataHandle() {
  // Schedule this to be destroyed on the IO thread
  content::BrowserThread::DeleteSoon(content::BrowserThread::IO,
                                     FROM_HERE, io_data_);
  io_data_ = NULL;
}

net::URLRequestContextGetter* BrowserContext::GetRequestContext() {
  return GetStoragePartition(this, NULL)->GetURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetRequestContextForRenderProcess(int renderer_child_id) {
  content::RenderProcessHost* host =
      content::RenderProcessHost::FromID(renderer_child_id);

  return host->GetStoragePartition()->GetURLRequestContext();
}

net::URLRequestContextGetter* BrowserContext::GetMediaRequestContext() {
  return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetMediaRequestContextForRenderProcess(int renderer_child_id) {
  content::RenderProcessHost* host =
      content::RenderProcessHost::FromID(renderer_child_id);

  return host->GetStoragePartition()->GetMediaURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory) {
  // We don't return any storage partition names from
  // ContentBrowserClient::GetStoragePartitionConfigForSite(), so it's a
  // bug to hit this
  NOTREACHED() << "Invalid request for request context for storage partition";
  return NULL;
}

void BrowserContext::RequestMidiSysExPermission(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame,
    const MidiSysExPermissionCallback& callback) {
  callback.Run(false);
}

void BrowserContext::CancelMidiSysExPermissionRequest(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame) {}

void BrowserContext::RequestProtectedMediaIdentifierPermission(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    int group_id,
    const GURL& requesting_frame,
    const ProtectedMediaIdentifierPermissionCallback& callback) {
  callback.Run(false);
}

void BrowserContext::CancelProtectedMediaIdentifierPermissionRequests(
    int group_id) {}

content::DownloadManagerDelegate*
    BrowserContext::GetDownloadManagerDelegate() {
  return NULL;
}

content::GeolocationPermissionContext*
    BrowserContext::GetGeolocationPermissionContext() {
  return NULL;
}

quota::SpecialStoragePolicy* BrowserContext::GetSpecialStoragePolicy() {
  return NULL;
}

void BrowserContext::AddObserver(BrowserContextObserver* observer) {
  observers_.AddObserver(observer);
}

void BrowserContext::RemoveObserver(BrowserContextObserver* observer) {
  observers_.RemoveObserver(observer);
}

BrowserContext::BrowserContext(BrowserContextIOData* io_data) :
    io_data_handle_(io_data) {
  CHECK(BrowserProcessMain::Exists()) <<
      "The main browser process components must be started before " <<
      "creating a context";

  g_all_contexts.Get().push_back(this);

  content::BrowserContext::EnsureResourceContextInitialized(this);
}

void BrowserContext::OnUserAgentChanged() {
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOriginalContext()->observers_,
                    NotifyUserAgentStringChanged());
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOffTheRecordContext()->observers_,
                    NotifyUserAgentStringChanged());
}

BrowserContext::~BrowserContext() {
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    observers_,
                    OnBrowserContextDestruction());

  std::vector<BrowserContext *>::iterator it;
  for (std::vector<BrowserContext *>::iterator it = g_all_contexts.Get().begin();
       it != g_all_contexts.Get().end(); ++it) {
    if (*it == this) {
      g_all_contexts.Get().erase(it);
      break;
    }
  }
}

// static
BrowserContext* BrowserContext::Create(const Params& params) {
  return new BrowserContextImpl(params);
}

// static
std::vector<BrowserContext *>& BrowserContext::GetAllContexts() {
  return g_all_contexts.Get();
}

// static
void BrowserContext::AssertNoContextsExist() {
  CHECK_EQ(g_all_contexts.Get().size(), static_cast<size_t>(0));
}

net::URLRequestContextGetter* BrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  DCHECK(!main_request_context_getter_);

  main_request_context_getter_ =
      new MainURLRequestContextGetter(io_data(),
                                      protocol_handlers,
                                      protocol_interceptors.Pass());

  return main_request_context_getter_;
}

void BrowserContext::SetDelegate(BrowserContextDelegate* delegate) {
  GetOriginalContext()->io_data()->SetDelegate(delegate);
  GetOffTheRecordContext()->io_data()->SetDelegate(delegate);
}

bool BrowserContext::IsOffTheRecord() const {
  return io_data()->IsOffTheRecord();
}

bool BrowserContext::IsSameContext(BrowserContext* other) const {
  return other->GetOriginalContext() == this ||
         other->GetOffTheRecordContext() == this;
}

base::FilePath BrowserContext::GetPath() const {
  return io_data()->GetPath();
}

base::FilePath BrowserContext::GetCachePath() const {
  return io_data()->GetCachePath();
}

std::string BrowserContext::GetAcceptLangs() const {
  return io_data()->GetAcceptLangs();
}

std::string BrowserContext::GetUserAgent() const {
  return io_data()->GetUserAgent();
}

net::StaticCookiePolicy::Type BrowserContext::GetCookiePolicy() const {
  return io_data()->GetCookiePolicy();
}

void BrowserContext::SetCookiePolicy(net::StaticCookiePolicy::Type policy) {
  GetOriginalContext()->io_data()->SetCookiePolicy(policy);
  GetOffTheRecordContext()->io_data()->SetCookiePolicy(policy);
}

content::CookieStoreConfig::SessionCookieMode BrowserContext::GetSessionCookieMode() const {
  return io_data()->GetSessionCookieMode();
}

content::ResourceContext* BrowserContext::GetResourceContext() {
  return io_data()->GetResourceContext();
}

} // namespace oxide
