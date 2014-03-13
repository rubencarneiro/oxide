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

#include <vector>

#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
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
#include "net/http/transport_security_state.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/server_bound_cert_store.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/url_request_job_factory_impl.h"

#include "shared/common/oxide_constants.h"

#include "oxide_browser_context_impl.h"
#include "oxide_browser_context_delegate.h"
#include "oxide_browser_context_observer.h"
#include "oxide_browser_process_main.h"
#include "oxide_http_user_agent_settings.h"
#include "oxide_io_thread_delegate.h"
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

} // namespace

class ResourceContext FINAL : public content::ResourceContext {
 public:
  ResourceContext() :
      request_context_(NULL) {}

  net::HostResolver* GetHostResolver() FINAL {
    return BrowserProcessMain::instance()->io_thread_delegate()->host_resolver();
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

BrowserContextIOData::BrowserContextIOData() :
    initialized_(false),
    cookie_policy_(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
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

void BrowserContextIOData::Init(
    content::ProtocolHandlerMap& protocol_handlers) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  DCHECK(!initialized_);

  initialized_ = true;

  IOThreadDelegate* io_thread_delegate =
      BrowserProcessMain::instance()->io_thread_delegate();

  ssl_config_service_ = new SSLConfigService();
  http_user_agent_settings_.reset(new HttpUserAgentSettings(this));
  ftp_transaction_factory_.reset(
      new net::FtpNetworkLayer(io_thread_delegate->host_resolver()));
  network_delegate_.reset(new NetworkDelegate(this));

  // TODO: We want persistent storage here (for non-incognito), but the
  //       persistent implementation used in Chrome uses the preferences
  //       system, which we don't want. We need our own implementation,
  //       backed either by sqlite or a text file
  http_server_properties_.reset(new net::HttpServerPropertiesImpl());

  URLRequestContext* context = new URLRequestContext();
  net::URLRequestContextStorage* storage = context->storage();

  context->set_net_log(io_thread_delegate->net_log());
  context->set_host_resolver(io_thread_delegate->host_resolver());
  context->set_cert_verifier(io_thread_delegate->cert_verifier());
  context->set_http_auth_handler_factory(
      io_thread_delegate->http_auth_handler_factory());
  context->set_proxy_service(io_thread_delegate->proxy_service());
  storage->set_ssl_config_service(ssl_config_service_.get());
  context->set_network_delegate(network_delegate_.get());
  context->set_http_user_agent_settings(http_user_agent_settings_.get());
  context->set_throttler_manager(io_thread_delegate->throttler_manager());

  // TODO: We want persistent storage here (for non-incognito), but 
  //       SQLiteServerBoundCertStore is part of chrome
  storage->set_server_bound_cert_service(
      new net::ServerBoundCertService(
          new net::DefaultServerBoundCertStore(NULL),
          base::WorkerPool::GetTaskRunner(true)));

  context->set_http_server_properties(http_server_properties_->GetWeakPtr());

  base::FilePath cookie_path;
  if (!IsOffTheRecord()) {
    DCHECK(!GetPath().empty());
    cookie_path = GetPath().Append(kCookiesFilename);
  }
  storage->set_cookie_store(content::CreateCookieStore(
      content::CookieStoreConfig(
        cookie_path,
        content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES,
        NULL, NULL)));

  storage->set_transport_security_state(new net::TransportSecurityState());
  // TODO: Need to implement net::TransportSecurityState::Delegate in order
  //       to have persistence for non-incognito mode. There is an
  //       implementation in Chrome which is backed by a json file

  net::HttpCache::BackendFactory* cache_backend = NULL;
  if (IsOffTheRecord()) {
    cache_backend = net::HttpCache::DefaultBackend::InMemory(0);
  } else {
    DCHECK(!GetCachePath().empty());
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

  net::URLRequestJobFactoryImpl* job_factory =
      new net::URLRequestJobFactoryImpl();
  storage->set_job_factory(job_factory);

  bool set_protocol = false;
  for (content::ProtocolHandlerMap::iterator it = protocol_handlers.begin();
       it != protocol_handlers.end();
       ++it) {
    set_protocol = job_factory->SetProtocolHandler(it->first,
                                                   it->second.release());
    DCHECK(set_protocol);
  }

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

  main_request_context_.reset(context);
  resource_context_->request_context_ = context;
  protocol_handlers.clear();
}

URLRequestContext* BrowserContextIOData::GetMainRequestContext() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  DCHECK(initialized_);
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
BrowserContext* BrowserContext::Create(const base::FilePath& path,
                                       const base::FilePath& cache_path) {
  return new BrowserContextImpl(path, cache_path);
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
    content::ProtocolHandlerMap* protocol_handlers) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  DCHECK(!main_request_context_getter_);

  main_request_context_getter_ =
      URLRequestContextGetter::CreateMain(protocol_handlers, io_data());

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

void BrowserContext::SetAcceptLangs(const std::string& langs) {
  io_data()->SetAcceptLangs(langs);
}

std::string BrowserContext::GetProduct() const {
  return io_data()->GetProduct();
}

void BrowserContext::SetProduct(const std::string& product) {
  io_data()->SetProduct(product);
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOriginalContext()->observers_,
                    NotifyUserAgentStringChanged());
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOffTheRecordContext()->observers_,
                    NotifyUserAgentStringChanged());
}

std::string BrowserContext::GetUserAgent() const {
  return io_data()->GetUserAgent();
}

void BrowserContext::SetUserAgent(const std::string& user_agent) {
  io_data()->SetUserAgent(user_agent);
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOriginalContext()->observers_,
                    NotifyUserAgentStringChanged());
  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOffTheRecordContext()->observers_,
                    NotifyUserAgentStringChanged());
}

net::StaticCookiePolicy::Type BrowserContext::GetCookiePolicy() const {
  return io_data()->GetCookiePolicy();
}

void BrowserContext::SetCookiePolicy(net::StaticCookiePolicy::Type policy) {
  GetOriginalContext()->io_data()->SetCookiePolicy(policy);
  GetOffTheRecordContext()->io_data()->SetCookiePolicy(policy);
}

content::ResourceContext* BrowserContext::GetResourceContext() {
  return io_data()->GetResourceContext();
}

} // namespace oxide
