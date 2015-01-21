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
#include <libintl.h>
#include <vector>

#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/stringprintf.h"
#include "base/supports_user_data.h"
#include "base/synchronization/lock.h"
#include "base/threading/worker_pool.h"
#include "content/browser/loader/resource_dispatcher_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_crypto_delegate.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/user_agent.h"
#include "net/base/host_mapping_rules.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/cookies/cookie_monster.h"
#include "net/ftp/ftp_network_layer.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_persister.h"
#include "net/http/transport_security_state.h"
#include "net/socket/tcp_server_socket.h"
#include "net/ssl/channel_id_service.h"
#include "net/ssl/channel_id_store.h"
#include "net/ssl/default_channel_id_store.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/ftp_protocol_handler.h"
#include "net/url_request/url_request_intercepting_job_factory.h"
#include "net/url_request/url_request_job_factory_impl.h"

#include "shared/common/chrome_version.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"

#include "oxide_browser_context_delegate.h"
#include "oxide_browser_context_observer.h"
#include "oxide_browser_process_main.h"
#include "oxide_devtools_http_handler_delegate.h"
#include "oxide_http_user_agent_settings.h"
#include "oxide_io_thread.h"
#include "oxide_network_delegate.h"
#include "oxide_ssl_config_service.h"
#include "oxide_ssl_host_state_delegate.h"
#include "oxide_url_request_context.h"
#include "oxide_url_request_delegated_job_factory.h"
#include "oxide_user_script_master.h"

namespace oxide {

namespace {

base::LazyInstance<std::vector<BrowserContext *> > g_all_contexts;

const base::FilePath::CharType kCacheDirname[] = FILE_PATH_LITERAL("Cache");
const base::FilePath::CharType kCookiesFilename[] =
    FILE_PATH_LITERAL("cookies.sqlite");

const char kDataScheme[] = "data";
const char kFileScheme[] = "file";
const char kFtpScheme[] = "ftp";

const char kBrowserContextKey[] = "oxide_browser_context_data";

const char kDefaultAcceptLanguage[] = "en-us,en";

const char kDevtoolsDefaultServerIp[] = "127.0.0.1";
const int kBackLog = 1;

class TCPServerSocketFactory :
    public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, int port)
      : address_(address),
        port_(port) {}

 private:
  scoped_ptr<net::ServerSocket> CreateForHttpServer() override {
    scoped_ptr<net::TCPServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLog::Source()));
    if (socket->ListenWithAddressAndPort(address_, port_, kBackLog) != net::OK) {
      return scoped_ptr<net::ServerSocket>();
    }

    return socket.Pass();
  }

  scoped_ptr<net::ServerSocket> CreateForTethering(
      std::string* out_name) override {
    // Not supported
    return scoped_ptr<net::ServerSocket>();
  }

  std::string address_;
  int port_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

class ResourceContextData : public base::SupportsUserData::Data {
 public:
  ResourceContextData(BrowserContextIOData* context) :
      context_(context) {}
  virtual ~ResourceContextData() {}

  BrowserContextIOData* get() const { return context_; }

 private:
  BrowserContextIOData* context_;
};

class MainURLRequestContextGetter final : public URLRequestContextGetter {
 public:
  MainURLRequestContextGetter(
      BrowserContextIOData* context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) :
      context_(context),
      request_interceptors_(request_interceptors.Pass()) {
    std::swap(protocol_handlers_, *protocol_handlers);
  }

  net::URLRequestContext* GetURLRequestContext() final {
    DCHECK_CURRENTLY_ON(content::BrowserThread::IO);

    if (!url_request_context_) {
      DCHECK(context_);
      url_request_context_ = context_->CreateMainRequestContext(
          protocol_handlers_, request_interceptors_.Pass())->AsWeakPtr();
      context_ = nullptr;
    }

    return url_request_context_.get();
  }

 private:
  BrowserContextIOData* context_;
  content::ProtocolHandlerMap protocol_handlers_;
  content::URLRequestInterceptorScopedVector request_interceptors_;

  base::WeakPtr<URLRequestContext> url_request_context_;
};

} // namespace

class ResourceContext final : public content::ResourceContext {
 public:
  ResourceContext() :
      request_context_(nullptr) {}

  net::HostResolver* GetHostResolver() final {
    return IOThread::instance()->globals()->host_resolver();
  }

  net::URLRequestContext* GetRequestContext() final {
    CHECK(request_context_);
    return request_context_;
  }

 private:
  friend class BrowserContextIOData;

  net::URLRequestContext* request_context_;

  DISALLOW_COPY_AND_ASSIGN(ResourceContext);
};

struct BrowserContextSharedData {
  BrowserContextSharedData(BrowserContext* context,
                           const BrowserContext::Params& params)
      : ref_count(0),
        in_dtor(false),
        product(base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING)),
        user_agent_string_is_default(true),
        user_script_master(context),
        devtools_enabled(params.devtools_enabled),
        devtools_port(params.devtools_port),
        devtools_ip(params.devtools_ip) {}

  mutable int ref_count;
  mutable bool in_dtor;

  std::string product;
  bool user_agent_string_is_default;
  UserScriptMaster user_script_master;

  scoped_ptr<content::DevToolsHttpHandler> devtools_http_handler;
  bool devtools_enabled;
  int devtools_port;
  std::string devtools_ip;
};

struct BrowserContextSharedIOData {
  BrowserContextSharedIOData(const BrowserContext::Params& params)
      : path(params.path),
        cache_path(params.cache_path),
        cookie_policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
        session_cookie_mode(params.session_cookie_mode),
        popup_blocker_enabled(true),
        host_mapping_rules(params.host_mapping_rules) {

   accept_langs = dgettext(OXIDE_GETTEXT_DOMAIN, "AcceptLanguage");
   if (accept_langs == "AcceptLanguage") {
     accept_langs = kDefaultAcceptLanguage;
   }
  }

  mutable base::Lock lock;

  base::FilePath path;
  base::FilePath cache_path;

  std::string user_agent_string;
  std::string accept_langs;

  net::StaticCookiePolicy::Type cookie_policy;
  content::CookieStoreConfig::SessionCookieMode session_cookie_mode;
  bool popup_blocker_enabled;

  std::vector<std::string> host_mapping_rules;

  scoped_refptr<BrowserContextDelegate> delegate;
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

class BrowserContextIODataImpl : public BrowserContextIOData {
 public:
  BrowserContextIODataImpl(const BrowserContext::Params& params)
      : data_(params) {}

  BrowserContextSharedIOData& GetSharedData() final {
    return data_;
  }
  const BrowserContextSharedIOData& GetSharedData() const final {
    return data_;
  }

 private:
  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const final {
    return GetPath().empty() ?
        content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES :
        data_.session_cookie_mode;
  }
 
  bool IsOffTheRecord() const final {
    return false;
  }

  BrowserContextSharedIOData data_;
};

class OTRBrowserContextIODataImpl : public BrowserContextIOData {
 public:
  OTRBrowserContextIODataImpl(BrowserContextIODataImpl* original)
      : original_io_data_(original) {}

 private:
  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const final {
    return content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES;
  }

  bool IsOffTheRecord() const final {
    return true;
  }

  BrowserContextSharedIOData& GetSharedData() final {
    return original_io_data_->GetSharedData();
  }
  const BrowserContextSharedIOData& GetSharedData() const final {
    return original_io_data_->GetSharedData();
  }

  BrowserContextIODataImpl* original_io_data_;
};

void BrowserContextIOData::Init() {
  DCHECK(!cookie_store_.get());

  base::FilePath cookie_path;
  if (!IsOffTheRecord() && !GetPath().empty()) {
    cookie_path = GetPath().Append(kCookiesFilename);
  }
  cookie_store_ = content::CreateCookieStore(
      content::CookieStoreConfig(cookie_path,
                                 GetSessionCookieMode(),
                                 nullptr, nullptr));
}

BrowserContextIOData::BrowserContextIOData() :
    resource_context_(new ResourceContext()) {
  resource_context_->SetUserData(kBrowserContextKey,
                                 new ResourceContextData(this));
}

BrowserContextIOData::~BrowserContextIOData() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
}

// static
BrowserContextIOData* BrowserContextIOData::FromResourceContext(
    content::ResourceContext* resource_context) {
  return static_cast<ResourceContextData *>(
      resource_context->GetUserData(kBrowserContextKey))->get();
}

scoped_refptr<BrowserContextDelegate> BrowserContextIOData::GetDelegate() {
  BrowserContextSharedIOData& data = GetSharedData();
  base::AutoLock lock(data.lock);
  return data.delegate;
}

net::StaticCookiePolicy::Type BrowserContextIOData::GetCookiePolicy() const {
  const BrowserContextSharedIOData& data = GetSharedData();
  base::AutoLock lock(data.lock);
  return data.cookie_policy;
}

bool BrowserContextIOData::IsPopupBlockerEnabled() const {
  return GetSharedData().popup_blocker_enabled;
}

base::FilePath BrowserContextIOData::GetPath() const {
  return GetSharedData().path;
}

base::FilePath BrowserContextIOData::GetCachePath() const {
  const BrowserContextSharedIOData& data = GetSharedData();
  if (data.cache_path.empty()) {
    return GetPath();
  }

  return data.cache_path;
}

std::string BrowserContextIOData::GetAcceptLangs() const {
  const BrowserContextSharedIOData& data = GetSharedData();
  base::AutoLock lock(data.lock);
  return data.accept_langs;
}

std::string BrowserContextIOData::GetUserAgent() const {
  const BrowserContextSharedIOData& data = GetSharedData();
  base::AutoLock lock(data.lock);
  return data.user_agent_string;
}

URLRequestContext* BrowserContextIOData::CreateMainRequestContext(
    content::ProtocolHandlerMap& protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
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

  host_mapping_rules_.reset(new net::HostMappingRules());
  const std::vector<std::string>& host_mapping_rules =
      GetSharedData().host_mapping_rules;
  for (std::vector<std::string>::const_iterator it =
           host_mapping_rules.begin();
       it != host_mapping_rules.end(); ++it) {
    host_mapping_rules_->AddRuleFromString(*it);
  }

  main_request_context_.reset(new URLRequestContext());
  URLRequestContext* context = main_request_context_.get();
  net::URLRequestContextStorage* storage = context->storage();

  storage->set_ssl_config_service(ssl_config_service_.get());
  context->set_network_delegate(network_delegate_.get());
  context->set_http_user_agent_settings(http_user_agent_settings_.get());

  // TODO: We want persistent storage here (for non-incognito), but 
  //       SQLiteChannelIDStore is part of chrome
  storage->set_channel_id_service(
      new net::ChannelIDService(
          new net::DefaultChannelIDStore(nullptr),
          base::WorkerPool::GetTaskRunner(true)));

  context->set_http_server_properties(http_server_properties_->GetWeakPtr());

  DCHECK(cookie_store_.get());
  storage->set_cookie_store(cookie_store_.get());

  context->set_transport_security_state(transport_security_state_.get());

  net::HttpCache::BackendFactory* cache_backend = nullptr;
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
  session_params.channel_id_service = context->channel_id_service();
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
  session_params.host_mapping_rules = host_mapping_rules_.get();

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
      new URLRequestDelegatedJobFactory(job_factory.Pass(),
                                        this));

  for (content::URLRequestInterceptorScopedVector::reverse_iterator it =
          request_interceptors.rbegin();
       it != request_interceptors.rend();
       ++it) {
    top_job_factory.reset(new net::URLRequestInterceptingJobFactory(
        top_job_factory.Pass(), make_scoped_ptr(*it)));
  }
  request_interceptors.weak_clear();

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
  if (delegate.get()) {
    StoragePermission res =
        delegate->CanAccessStorage(url, first_party_url, write,
                                   STORAGE_TYPE_COOKIES);
    if (res != STORAGE_PERMISSION_UNDEFINED) {
      return res == STORAGE_PERMISSION_ALLOW;
    }
  }

  net::StaticCookiePolicy policy(GetCookiePolicy());
  if (write) {
    return policy.CanSetCookie(url, first_party_url) == net::OK;
  }

  return policy.CanGetCookies(url, first_party_url) == net::OK;
}

class BrowserContextImpl;

class OTRBrowserContextImpl : public BrowserContext {
 public:
  OTRBrowserContextImpl(BrowserContextImpl* original,
                        BrowserContextIODataImpl* original_io_data);

 private:
  friend class BrowserContextImpl; // For the destructor

  virtual ~OTRBrowserContextImpl() {}

  BrowserContext* GetOffTheRecordContext() final {
    return this;
  }
  BrowserContext* GetOriginalContext() final;

  BrowserContextSharedData& GetSharedData() final;
  const BrowserContextSharedData& GetSharedData() const final;

  bool HasOffTheRecordContext() const final { return true; }

  BrowserContextImpl* original_context_;
};

class BrowserContextImpl : public BrowserContext {
 public:
  BrowserContextImpl(const BrowserContext::Params& params);

  BrowserContextSharedData& GetSharedData() final {
    return data_;
  }
  const BrowserContextSharedData& GetSharedData() const final {
    return data_;
  }

 private:
  virtual ~BrowserContextImpl();

  BrowserContext* GetOffTheRecordContext() final;

  BrowserContext* GetOriginalContext() final {
    return this;
  }

  bool HasOffTheRecordContext() const final {
    return otr_context_ != nullptr;
  }

  BrowserContextSharedData data_;
  OTRBrowserContextImpl* otr_context_;
};

BrowserContextSharedData& OTRBrowserContextImpl::GetSharedData() {
  return original_context_->GetSharedData();
}

const BrowserContextSharedData&
OTRBrowserContextImpl::GetSharedData() const {
  return original_context_->GetSharedData();
}

BrowserContext* OTRBrowserContextImpl::GetOriginalContext() {
  return original_context_;
}

OTRBrowserContextImpl::OTRBrowserContextImpl(
    BrowserContextImpl* original,
    BrowserContextIODataImpl* original_io_data)
    : BrowserContext(new OTRBrowserContextIODataImpl(original_io_data)),
      original_context_(original) {}

BrowserContextImpl::~BrowserContextImpl() {
  if (otr_context_) {
    delete otr_context_;
    otr_context_ = nullptr;
  }
}

BrowserContext* BrowserContextImpl::GetOffTheRecordContext() {
  if (!otr_context_) {
    otr_context_ = new OTRBrowserContextImpl(
        this,
        static_cast<BrowserContextIODataImpl *>(io_data()));
  }

  return otr_context_;
}

BrowserContextImpl::BrowserContextImpl(const BrowserContext::Params& params)
    : BrowserContext(new BrowserContextIODataImpl(params)),
      data_(this, params),
      otr_context_(nullptr) {
  io_data()->GetSharedData().user_agent_string =
      content::BuildUserAgentFromProduct(data_.product);

  if (data_.devtools_enabled &&
      data_.devtools_port < 65535 &&
      data_.devtools_port > 1024) {
    net::IPAddressNumber unused;
    std::string ip =
        net::ParseIPLiteralToNumber(data_.devtools_ip, &unused) ?
          data_.devtools_ip : kDevtoolsDefaultServerIp;

    scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory> factory(
        new TCPServerSocketFactory(ip, data_.devtools_port));
    data_.devtools_http_handler.reset(
        content::DevToolsHttpHandler::Start(factory.Pass(),
                                            std::string(),
                                            new DevtoolsHttpHandlerDelegate(),
                                            base::FilePath()));
  }
}

scoped_ptr<content::ZoomLevelDelegate> BrowserContext::CreateZoomLevelDelegate(
    const base::FilePath& partition_path) {
  return nullptr;
}

net::URLRequestContextGetter* BrowserContext::GetRequestContext() {
  DCHECK(CalledOnValidThread());
  return GetStoragePartition(this, nullptr)->GetURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetRequestContextForRenderProcess(int renderer_child_id) {
  DCHECK(CalledOnValidThread());

  content::RenderProcessHost* host =
      content::RenderProcessHost::FromID(renderer_child_id);

  return host->GetStoragePartition()->GetURLRequestContext();
}

net::URLRequestContextGetter* BrowserContext::GetMediaRequestContext() {
  DCHECK(CalledOnValidThread());
  return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContext::GetMediaRequestContextForRenderProcess(int renderer_child_id) {
  DCHECK(CalledOnValidThread());

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
  return nullptr;
}

content::DownloadManagerDelegate*
    BrowserContext::GetDownloadManagerDelegate() {
  return nullptr;
}

content::BrowserPluginGuestManager* BrowserContext::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy* BrowserContext::GetSpecialStoragePolicy() {
  return nullptr;
}

content::PushMessagingService* BrowserContext::GetPushMessagingService() {
  return nullptr;
}

content::SSLHostStateDelegate* BrowserContext::GetSSLHostStateDelegate() {
  if (!ssl_host_state_delegate_) {
    ssl_host_state_delegate_.reset(new SSLHostStateDelegate());
  }

  return ssl_host_state_delegate_.get();
}

void BrowserContext::AddObserver(BrowserContextObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.AddObserver(observer);
}

void BrowserContext::RemoveObserver(BrowserContextObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.RemoveObserver(observer);
}

// static
void BrowserContext::Delete(const BrowserContext* context) {
  delete const_cast<BrowserContext *>(context)->GetOriginalContext();
}

BrowserContext::BrowserContext(BrowserContextIOData* io_data) :
    io_data_(io_data) {
  CHECK(BrowserProcessMain::GetInstance()->IsRunning()) <<
      "The main browser process components must be started before " <<
      "creating a context";

  g_all_contexts.Get().push_back(this);

  // Make sure that the cookie store is properly created
  io_data->Init();

  content::BrowserContext::EnsureResourceContextInitialized(this);
}

BrowserContext::~BrowserContext() {
  DCHECK(CalledOnValidThread());

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

  // Schedule io_data_ to be destroyed on the IO thread
  content::BrowserThread::DeleteSoon(content::BrowserThread::IO,
                                     FROM_HERE, io_data_);
  io_data_ = nullptr;
}

// static
scoped_refptr<BrowserContext> BrowserContext::Create(const Params& params) {
  scoped_refptr<BrowserContext> context = new BrowserContextImpl(params);
  return context;
}

// static
void BrowserContext::AssertNoContextsExist() {
  CHECK_EQ(g_all_contexts.Get().size(), static_cast<size_t>(0));
}

void BrowserContext::AddRef() const {
  DCHECK(CalledOnValidThread());

  const BrowserContextSharedData& data = GetSharedData();
  DCHECK(!data.in_dtor);
  data.ref_count++;
}

void BrowserContext::Release() const {
  DCHECK(CalledOnValidThread());

  const BrowserContextSharedData& data = GetSharedData();
  DCHECK(!data.in_dtor);
  DCHECK(data.ref_count > 0);
  if (--data.ref_count == 0) {
    data.in_dtor = true;
    Delete(this);
  }
}

net::URLRequestContextGetter* BrowserContext::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  DCHECK(CalledOnValidThread());
  DCHECK(!main_request_context_getter_.get());

  main_request_context_getter_ =
      new MainURLRequestContextGetter(io_data(),
                                      protocol_handlers,
                                      request_interceptors.Pass());

  return main_request_context_getter_.get();
}

BrowserContextDelegate* BrowserContext::GetDelegate() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetDelegate().get();
}

void BrowserContext::SetDelegate(BrowserContextDelegate* delegate) {
  DCHECK(CalledOnValidThread());

  BrowserContextSharedIOData& data = io_data()->GetSharedData();
  base::AutoLock lock(data.lock);
  data.delegate = delegate;
}

bool BrowserContext::IsOffTheRecord() const {
  DCHECK(CalledOnValidThread());
  return io_data()->IsOffTheRecord();
}

bool BrowserContext::IsSameContext(BrowserContext* other) const {
  DCHECK(CalledOnValidThread());
  return other->GetOriginalContext() == this ||
         (other->HasOffTheRecordContext() &&
          other->GetOffTheRecordContext() == this);
}

base::FilePath BrowserContext::GetPath() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetPath();
}

base::FilePath BrowserContext::GetCachePath() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetCachePath();
}

std::string BrowserContext::GetAcceptLangs() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetAcceptLangs();
}

void BrowserContext::SetAcceptLangs(const std::string& langs) {
  DCHECK(CalledOnValidThread());

  BrowserContextSharedIOData& data = io_data()->GetSharedData();
  base::AutoLock lock(data.lock);
  data.accept_langs = langs;
}

std::string BrowserContext::GetProduct() const {
  DCHECK(CalledOnValidThread());
  return GetSharedData().product;
}

void BrowserContext::SetProduct(const std::string& product) {
  DCHECK(CalledOnValidThread());

  BrowserContextSharedData& data = GetSharedData();
  data.product = product.empty() ?
      base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING) : product;
  if (data.user_agent_string_is_default) {
    SetUserAgent(std::string());
  }
}

std::string BrowserContext::GetUserAgent() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetUserAgent();
}

void BrowserContext::SetUserAgent(const std::string& user_agent) {
  DCHECK(CalledOnValidThread());

  {
    BrowserContextSharedIOData& data = io_data()->GetSharedData();
    base::AutoLock lock(data.lock);
    data.user_agent_string = user_agent.empty() ?
        content::BuildUserAgentFromProduct(GetSharedData().product) :
        user_agent;
    GetSharedData().user_agent_string_is_default = user_agent.empty();
  }

  for (content::RenderProcessHost::iterator it =
          content::RenderProcessHost::AllHostsIterator();
       !it.IsAtEnd(); it.Advance()) {
    content::RenderProcessHost* host = it.GetCurrentValue();
    if (IsSameContext(
            BrowserContext::FromContent(host->GetBrowserContext()))) {
      host->Send(new OxideMsg_SetUserAgent(GetUserAgent()));
    }
  }
}

net::StaticCookiePolicy::Type BrowserContext::GetCookiePolicy() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetCookiePolicy();
}

void BrowserContext::SetCookiePolicy(net::StaticCookiePolicy::Type policy) {
  DCHECK(CalledOnValidThread());

  BrowserContextSharedIOData& data = io_data()->GetSharedData();
  base::AutoLock lock(data.lock);
  data.cookie_policy = policy;
}

content::CookieStoreConfig::SessionCookieMode
BrowserContext::GetSessionCookieMode() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetSessionCookieMode();
}

bool BrowserContext::IsPopupBlockerEnabled() const {
  DCHECK(CalledOnValidThread());
  return io_data()->IsPopupBlockerEnabled();
}

void BrowserContext::SetIsPopupBlockerEnabled(bool enabled) {
  DCHECK(CalledOnValidThread());

  io_data()->GetSharedData().popup_blocker_enabled = enabled;

  FOR_EACH_OBSERVER(BrowserContextObserver,
                    GetOriginalContext()->observers_,
                    NotifyPopupBlockerEnabledChanged());
  if (HasOffTheRecordContext()) {
    FOR_EACH_OBSERVER(BrowserContextObserver,
                      GetOffTheRecordContext()->observers_,
                      NotifyPopupBlockerEnabledChanged());
  }
}

bool BrowserContext::GetDevtoolsEnabled() const {
  DCHECK(CalledOnValidThread());
  return GetSharedData().devtools_enabled;
}

int BrowserContext::GetDevtoolsPort() const {
  DCHECK(CalledOnValidThread());
  return GetSharedData().devtools_port;
}

std::string BrowserContext::GetDevtoolsBindIp() const {
  DCHECK(CalledOnValidThread());
  return GetSharedData().devtools_ip;
}

const std::vector<std::string>& BrowserContext::GetHostMappingRules() const {
  DCHECK(CalledOnValidThread());
  return io_data()->GetSharedData().host_mapping_rules;
}

UserScriptMaster& BrowserContext::UserScriptManager() {
  DCHECK(CalledOnValidThread());
  return GetSharedData().user_script_master;
}

content::ResourceContext* BrowserContext::GetResourceContext() {
  DCHECK(CalledOnValidThread());
  return io_data()->GetResourceContext();
}

scoped_refptr<net::CookieStore> BrowserContext::GetCookieStore() {
  DCHECK(CalledOnValidThread());
  return io_data()->cookie_store_;
}

} // namespace oxide
