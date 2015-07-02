// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qt_web_context.h"

#include <vector>

#include <QDateTime>
#include <QDebug>
#include <QMetaMethod>
#include <QNetworkCookie>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QUrl>

#include "base/auto_reset.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/resource_type.h"
#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_monster.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"
#include "qt/core/api/oxideqstoragepermissionrequest_p.h"
#include "qt/core/browser/oxide_qt_url_request_delegated_job.h"
#include "qt/core/browser/oxide_qt_user_script.h"
#include "qt/core/glue/oxide_qt_web_context_proxy_client.h"
#include "shared/browser/media/oxide_media_capture_devices_context.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_agent_settings.h"
#include "shared/browser/oxide_user_script_master.h"
#include "shared/browser/permissions/oxide_temporary_saved_permission_context.h"

#include "oxide_qt_browser_startup.h"

namespace oxide {
namespace qt {

using oxide::MediaCaptureDevicesContext;
using oxide::UserAgentSettings;
using oxide::UserScriptMaster;

namespace {

const unsigned kDefaultDevtoolsPort = 8484;

bool g_default_is_application_owned;
bool g_initialized_default = false;
WebContext* g_default;

int GetNextCookieRequestId() {
  static int id = 0;
  if (id == std::numeric_limits<int>::max()) {
    int i = id;
    id = 0;
    return i;
  }
  return id++;
}

}

class WebContext::BrowserContextDelegate
    : public oxide::BrowserContextDelegate {
 public:
  BrowserContextDelegate(const base::WeakPtr<WebContext> context);
  ~BrowserContextDelegate() override;

  void Init(const QWeakPointer<WebContextProxyClient::IOClient>& io_client);

  WebContext* context() const { return context_getter_->GetContext(); }

  void SetAllowedExtraURLSchemes(const std::set<std::string>& schemes);

 private:
  QSharedPointer<WebContextProxyClient::IOClient> GetIOClient();

  // oxide::BrowserContextDelegate implementation
  int OnBeforeURLRequest(net::URLRequest* request,
                         const net::CompletionCallback& callback,
                         GURL* new_url) override;
  int OnBeforeSendHeaders(net::URLRequest* request,
                          const net::CompletionCallback& callback,
                          net::HttpRequestHeaders* headers) override;
  int OnBeforeRedirect(net::URLRequest* request,
                       const GURL& new_location) override;
  oxide::StoragePermission CanAccessStorage(const GURL& url,
                                            const GURL& first_party_url,
                                            bool write,
                                            oxide::StorageType type) override;
  std::string GetUserAgentOverride(const GURL& url) override;
  bool IsCustomProtocolHandlerRegistered(
      const std::string& scheme) const override;
  oxide::URLRequestDelegatedJob* CreateCustomURLRequestJob(
      net::URLRequest* request,
      net::NetworkDelegate* network_delegate) override;

  scoped_refptr<WebContextGetter> context_getter_;

  base::Lock io_client_lock_;
  QWeakPointer<WebContextProxyClient::IOClient> io_client_;

  mutable base::Lock url_schemes_lock_;
  std::set<std::string> allowed_extra_url_schemes_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContextDelegate);
};

struct WebContext::ConstructProperties {
  ConstructProperties()
      : max_cache_size_hint(0),
        cookie_policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
        session_cookie_mode(content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
        popup_blocker_enabled(true),
        devtools_enabled(false),
        devtools_port(kDefaultDevtoolsPort),
        legacy_user_agent_override_enabled(false),
        do_not_track(false) {}
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  int max_cache_size_hint;
  std::string accept_langs;
  net::StaticCookiePolicy::Type cookie_policy;
  content::CookieStoreConfig::SessionCookieMode session_cookie_mode;
  bool popup_blocker_enabled;
  bool devtools_enabled;
  int devtools_port;
  std::string devtools_ip;
  std::vector<std::string> host_mapping_rules;
  std::string default_audio_capture_device_id;
  std::string default_video_capture_device_id;
  std::vector<UserAgentSettings::UserAgentOverride> user_agent_overrides;
  bool legacy_user_agent_override_enabled;
  bool do_not_track;
};

class SetCookiesContext : public base::RefCounted<SetCookiesContext> {
 public:
  SetCookiesContext(int id)
      : id(id), remaining(0) {}

  int id;
  int remaining;
  QList<QNetworkCookie> failed;
};

QSharedPointer<WebContextProxyClient::IOClient>
WebContext::BrowserContextDelegate::GetIOClient() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  base::AutoLock lock(io_client_lock_);
  return io_client_.toStrongRef();
}

int WebContext::BrowserContextDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  QSharedPointer<WebContextProxyClient::IOClient> io_client = GetIOClient();
  if (!io_client) {
    return net::OK;
  }

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return net::OK;
  }

  OxideQBeforeURLRequestEvent event(
      QUrl(QString::fromStdString(request->url().spec())),
      QString::fromStdString(request->method()),
      QString::fromStdString(request->referrer()),
      info->IsMainFrame());

  io_client->OnBeforeURLRequest(&event);

  OxideQBeforeURLRequestEventPrivate* eventp =
      OxideQBeforeURLRequestEventPrivate::get(&event);
  *new_url = GURL(eventp->new_url.toString().toStdString());

  return eventp->request_cancelled ? net::ERR_ABORTED : net::OK;
}

int WebContext::BrowserContextDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  QSharedPointer<WebContextProxyClient::IOClient> io_client = GetIOClient();
  if (!io_client) {
    return net::OK;
  }

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return net::OK;
  }

  OxideQBeforeSendHeadersEvent event(
      QUrl(QString::fromStdString(request->url().spec())),
      QString::fromStdString(request->method()),
      QString::fromStdString(request->referrer()),
      info->IsMainFrame());

  OxideQBeforeSendHeadersEventPrivate* eventp =
      OxideQBeforeSendHeadersEventPrivate::get(&event);
  eventp->headers = headers;

  io_client->OnBeforeSendHeaders(&event);

  return eventp->request_cancelled ? net::ERR_ABORTED : net::OK;
}

int WebContext::BrowserContextDelegate::OnBeforeRedirect(
    net::URLRequest* request,
    const GURL& new_location) {
  QSharedPointer<WebContextProxyClient::IOClient> io_client = GetIOClient();
  if (!io_client) {
    return net::OK;
  }

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return net::OK;
  }

  OxideQBeforeRedirectEvent event(
      QUrl(QString::fromStdString(new_location.spec())),
      QString::fromStdString(request->method()),
      QString::fromStdString(request->referrer()),
      info->IsMainFrame(),
      QUrl(QString::fromStdString(request->original_url().spec())));

  io_client->OnBeforeRedirect(&event);

  OxideQBeforeRedirectEventPrivate* eventp =
      OxideQBeforeRedirectEventPrivate::get(&event);

  return eventp->request_cancelled ? net::ERR_ABORTED : net::OK;
}

oxide::StoragePermission WebContext::BrowserContextDelegate::CanAccessStorage(
    const GURL& url,
    const GURL& first_party_url,
    bool write,
    oxide::StorageType type) {
  QSharedPointer<WebContextProxyClient::IOClient> io_client = GetIOClient();
  if (!io_client) {
    return oxide::STORAGE_PERMISSION_UNDEFINED;
  }

  OxideQStoragePermissionRequest req(
      QUrl(QString::fromStdString(url.spec())),
      QUrl(QString::fromStdString(first_party_url.spec())),
      write,
      static_cast<OxideQStoragePermissionRequest::Type>(type));

  io_client->HandleStoragePermissionRequest(&req);

  return OxideQStoragePermissionRequestPrivate::get(&req)->permission;
}

std::string WebContext::BrowserContextDelegate::GetUserAgentOverride(
    const GURL& url) {
  QSharedPointer<WebContextProxyClient::IOClient> io_client = GetIOClient();
  if (!io_client) {
    return std::string();
  }

  QString user_agent = io_client->GetUserAgentOverride(
      QUrl(QString::fromStdString(url.spec())));

  return user_agent.toStdString();
}

bool WebContext::BrowserContextDelegate::IsCustomProtocolHandlerRegistered(
    const std::string& scheme) const {
  base::AutoLock lock(url_schemes_lock_);
  return allowed_extra_url_schemes_.find(scheme) !=
      allowed_extra_url_schemes_.end();
}

oxide::URLRequestDelegatedJob*
WebContext::BrowserContextDelegate::CreateCustomURLRequestJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  return new URLRequestDelegatedJob(context_getter_.get(),
                                    request,
                                    network_delegate);
}

WebContext::BrowserContextDelegate::BrowserContextDelegate(
    const base::WeakPtr<WebContext> context)
    : context_getter_(new WebContextGetter(context)) {}

WebContext::BrowserContextDelegate::~BrowserContextDelegate() {}

void WebContext::BrowserContextDelegate::Init(
    const QWeakPointer<WebContextProxyClient::IOClient>& io_client) {
  base::AutoLock lock(io_client_lock_);
  io_client_ = io_client;
}

void WebContext::BrowserContextDelegate::SetAllowedExtraURLSchemes(
    const std::set<std::string>& schemes) {
  base::AutoLock lock(url_schemes_lock_);
  allowed_extra_url_schemes_ = schemes;
}

WebContextGetter::WebContextGetter(const base::WeakPtr<WebContext>& context)
    : context_(context) {}

WebContextGetter::~WebContextGetter() {}

WebContext* WebContextGetter::GetContext() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return context_.get();
}

bool WebContext::IsInitialized() const {
  return context_.get() != nullptr;
}

void WebContext::UpdateUserScripts() {
  if (!context_.get()) {
    return;
  }

  std::vector<const oxide::UserScript *> scripts;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScript* script = UserScript::FromProxyHandle(user_scripts_.at(i));
    if (script->state() == UserScript::Loading ||
        script->state() == UserScript::Constructing) {
      return;
    } else if (script->state() == UserScript::Loaded) {
      scripts.push_back(script->impl());
    }
  }

  UserScriptMaster::Get(context_.get())
      ->SerializeUserScriptsAndSendUpdates(scripts);
}

void WebContext::CookieSetCallback(
    const scoped_refptr<SetCookiesContext>& ctxt,
    const QNetworkCookie& cookie,
    bool success) {
  DCHECK_GT(ctxt->remaining, 0);

  if (!success) {
    ctxt->failed.push_back(cookie);
  }

  if (--ctxt->remaining > 0 || handling_cookie_request_) {
    return;
  }

  DeliverCookiesSet(ctxt);
}

void WebContext::DeliverCookiesSet(
    const scoped_refptr<SetCookiesContext>& ctxt) {
  DCHECK_EQ(ctxt->remaining, 0);
  client_->CookiesSet(ctxt->id, ctxt->failed);
}

void WebContext::GotCookiesCallback(int request_id,
                                    const net::CookieList& cookies) {
  if (handling_cookie_request_) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContext::GotCookiesCallback,
                   weak_factory_.GetWeakPtr(), request_id, cookies));
    return;
  }

  QList<QNetworkCookie> qcookies;
  for (net::CookieList::const_iterator iter = cookies.begin();
       iter != cookies.end(); ++iter) {
    QNetworkCookie cookie;

    cookie.setName(iter->Name().c_str());
    cookie.setValue(iter->Value().c_str());
    cookie.setDomain(iter->Domain().c_str());
    cookie.setPath(iter->Path().c_str());
    if (!iter->ExpiryDate().is_null()) {
      cookie.setExpirationDate(QDateTime::fromMSecsSinceEpoch(
          iter->ExpiryDate().ToJsTime()));
    }
    cookie.setSecure(iter->IsSecure());
    cookie.setHttpOnly(iter->IsHttpOnly());

    qcookies.append(cookie);
  }

  client_->CookiesRetrieved(request_id, qcookies);
}

void WebContext::DeletedCookiesCallback(int request_id,
                                        int num_deleted) {
  client_->CookiesDeleted(request_id, num_deleted);
}

WebContext::WebContext(WebContextProxyClient* client)
    : client_(client),
      construct_props_(new ConstructProperties()),
      handling_cookie_request_(false),
      weak_factory_(this) {
  delegate_ = new BrowserContextDelegate(weak_factory_.GetWeakPtr());

  COMPILE_ASSERT(
      CookiePolicyAllowAll == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
      cookie_enums_allowall_doesnt_match);
  COMPILE_ASSERT(
      CookiePolicyBlockAll == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::BLOCK_ALL_COOKIES),
      cookie_enums_blockall_doesnt_match);
  COMPILE_ASSERT(
      CookiePolicyBlockThirdParty == static_cast<CookiePolicy>(
        net::StaticCookiePolicy::BLOCK_ALL_THIRD_PARTY_COOKIES),
      cookie_enums_blockall3rdparty_doesnt_match);

  COMPILE_ASSERT(
      SessionCookieModeEphemeral == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
      session_cookie_mode_enums_ephemeral_doesnt_match);
  COMPILE_ASSERT(
      SessionCookieModePersistent == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::PERSISTANT_SESSION_COOKIES),
      session_cookie_mode_enums_persistent_doesnt_match);
  COMPILE_ASSERT(
      SessionCookieModeRestored == static_cast<SessionCookieMode>(
        content::CookieStoreConfig::RESTORED_SESSION_COOKIES),
      session_cookie_mode_enums_restored_doesnt_match);
}

WebContext::~WebContext() {
  if (g_default == this) {
    g_default = nullptr;
  }

  if (context_.get()) {
    context_->SetDelegate(nullptr);
    MediaCaptureDevicesContext::Get(context_.get())->set_client(nullptr);
  }
}

// static
WebContext* WebContext::FromBrowserContext(oxide::BrowserContext* context) {
  BrowserContextDelegate* delegate =
      static_cast<BrowserContextDelegate*>(context->GetDelegate());
  if (!delegate) {
    return nullptr;
  }

  return delegate->context();
}

// static
WebContext* WebContext::FromProxyHandle(WebContextProxyHandle* handle) {
  return static_cast<WebContext*>(handle->proxy_.data());
}

// static
WebContext* WebContext::GetDefault() {
  return g_default;
}

// static
void WebContext::DestroyDefault() {
  if (!g_default || g_default_is_application_owned) {
    return;
  }

  g_default->client_->DestroyDefault();
  DCHECK(!g_default);
}

oxide::BrowserContext* WebContext::GetContext() {
  if (context_.get()) {
    return context_.get();
  }

  DCHECK(construct_props_);

  oxide::BrowserContext::Params params(
      construct_props_->data_path,
      construct_props_->cache_path,
      construct_props_->max_cache_size_hint,
      construct_props_->session_cookie_mode,
      construct_props_->devtools_enabled,
      construct_props_->devtools_port,
      construct_props_->devtools_ip);
  params.host_mapping_rules = construct_props_->host_mapping_rules;

  context_ = oxide::BrowserContext::Create(params);

  UserAgentSettings* ua_settings = UserAgentSettings::Get(context_.get());

  if (!construct_props_->product.empty()) {
    ua_settings->SetProduct(construct_props_->product);
  }
  if (!construct_props_->user_agent.empty()) {
    ua_settings->SetUserAgent(construct_props_->user_agent);
  }
  if (!construct_props_->accept_langs.empty()) {
    ua_settings->SetAcceptLangs(construct_props_->accept_langs);
  }
  ua_settings->SetUserAgentOverrides(construct_props_->user_agent_overrides);
  ua_settings->SetLegacyUserAgentOverrideEnabled(
      construct_props_->legacy_user_agent_override_enabled);

  context_->SetCookiePolicy(construct_props_->cookie_policy);
  context_->SetIsPopupBlockerEnabled(construct_props_->popup_blocker_enabled);
  context_->SetDoNotTrack(construct_props_->do_not_track);

  MediaCaptureDevicesContext* dc =
      MediaCaptureDevicesContext::Get(context_.get());

  if (!construct_props_->default_audio_capture_device_id.empty()) {
    if (!dc->SetDefaultAudioDeviceId(
        construct_props_->default_audio_capture_device_id)) {
      client_->DefaultAudioCaptureDeviceChanged();
    }
  }
  if (!construct_props_->default_video_capture_device_id.empty()) {
    if (!dc->SetDefaultVideoDeviceId(
        construct_props_->default_video_capture_device_id)) {
      client_->DefaultVideoCaptureDeviceChanged();
    }
  }

  dc->set_client(this);

  context_->SetDelegate(delegate_.get());

  construct_props_.reset();

  UpdateUserScripts();

  return context_.get();
}

QNetworkAccessManager* WebContext::GetCustomNetworkAccessManager() {
  return client_->GetCustomNetworkAccessManager();
}

void WebContext::init(
    const QWeakPointer<WebContextProxyClient::IOClient>& io_client) {
  // If we are in single process mode because it was set in the environment,
  // allow the first application-created context to become the default
  if (oxide::BrowserProcessMain::GetInstance()->GetProcessModel() ==
          oxide::PROCESS_MODEL_SINGLE_PROCESS &&
      BrowserStartup::GetInstance()->DidSelectProcessModelFromEnv() &&
      !g_initialized_default) {
    g_initialized_default = true;
    g_default_is_application_owned = true;
    g_default = this;
  }

  delegate_->Init(io_client);
}

void WebContext::makeDefault() {
  DCHECK(!g_initialized_default);
  DCHECK(!g_default || g_default == this);

  g_initialized_default = true;
  g_default_is_application_owned = false;
  g_default = this;
}

QString WebContext::product() const {
  if (IsInitialized()) {
    return QString::fromStdString(
        UserAgentSettings::Get(context_.get())->GetProduct());
  }

  return QString::fromStdString(construct_props_->product);
}

void WebContext::setProduct(const QString& product) {
  if (IsInitialized()) {
    UserAgentSettings::Get(context_.get())->SetProduct(
        product.toStdString());
  } else {
    construct_props_->product = product.toStdString();
  }
}

QString WebContext::userAgent() const {
  if (IsInitialized()) {
    return QString::fromStdString(
        UserAgentSettings::Get(context_.get())->GetUserAgent());
  }

  return QString::fromStdString(construct_props_->user_agent);
}

void WebContext::setUserAgent(const QString& user_agent) {
  if (IsInitialized()) {
    UserAgentSettings::Get(context_.get())->SetUserAgent(
        user_agent.toStdString());
  } else {
    construct_props_->user_agent = user_agent.toStdString();
  }
}

QUrl WebContext::dataPath() const {
  base::FilePath path;
  if (IsInitialized()) {
    path = context_->GetPath();
  } else {
    path = construct_props_->data_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContext::setDataPath(const QUrl& url) {
  DCHECK(!IsInitialized());
  DCHECK(url.isLocalFile() || url.isEmpty());
  construct_props_->data_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QUrl WebContext::cachePath() const {
  base::FilePath path;
  if (IsInitialized()) {
    path = context_->GetCachePath();
  } else {
    path = construct_props_->cache_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContext::setCachePath(const QUrl& url) {
  DCHECK(!IsInitialized());
  DCHECK(url.isLocalFile() || url.isEmpty());
  construct_props_->cache_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QString WebContext::acceptLangs() const {
  if (IsInitialized()) {
    return QString::fromStdString(
        UserAgentSettings::Get(context_.get())->GetAcceptLangs());
  }

  return QString::fromStdString(construct_props_->accept_langs);
}

void WebContext::setAcceptLangs(const QString& langs) {
  if (IsInitialized()) {
    UserAgentSettings::Get(context_.get())->SetAcceptLangs(
        langs.toStdString());
  } else {
    construct_props_->accept_langs = langs.toStdString();
  }
}

QList<UserScriptProxyHandle*>& WebContext::userScripts() {
  return user_scripts_;
}

void WebContext::updateUserScripts() {
  UpdateUserScripts();
}

bool WebContext::isInitialized() const {
  return IsInitialized();
}

WebContextProxy::CookiePolicy WebContext::cookiePolicy() const {
  if (IsInitialized()) {
    return static_cast<CookiePolicy>(context_->GetCookiePolicy());
  }

  return static_cast<CookiePolicy>(construct_props_->cookie_policy);
}

void WebContext::setCookiePolicy(CookiePolicy policy) {
  if (IsInitialized()) {
    context_->SetCookiePolicy(
        static_cast<net::StaticCookiePolicy::Type>(policy));
  } else {
    construct_props_->cookie_policy =
        static_cast<net::StaticCookiePolicy::Type>(policy);
  }
}

WebContextProxy::SessionCookieMode WebContext::sessionCookieMode() const {
  if (IsInitialized()) {
    return static_cast<SessionCookieMode>(context_->GetSessionCookieMode());
  }

  return static_cast<SessionCookieMode>(construct_props_->session_cookie_mode);
}

void WebContext::setSessionCookieMode(SessionCookieMode mode) {
  DCHECK(!IsInitialized());
  construct_props_->session_cookie_mode =
      static_cast<content::CookieStoreConfig::SessionCookieMode>(mode);
}

bool WebContext::popupBlockerEnabled() const {
  if (IsInitialized()) {
    return context_->IsPopupBlockerEnabled();
  }

  return construct_props_->popup_blocker_enabled;
}

void WebContext::setPopupBlockerEnabled(bool enabled) {
  if (IsInitialized()) {
    context_->SetIsPopupBlockerEnabled(enabled);
  } else {
    construct_props_->popup_blocker_enabled = enabled;
  }
}

bool WebContext::devtoolsEnabled() const {
  if (IsInitialized()) {
    return context_->GetDevtoolsEnabled();
  }

  return construct_props_->devtools_enabled;
}

void WebContext::setDevtoolsEnabled(bool enabled) {
  DCHECK(!IsInitialized());
  construct_props_->devtools_enabled = enabled;
}

int WebContext::devtoolsPort() const {
  if (IsInitialized()) {
    return context_->GetDevtoolsPort();
  }

  return construct_props_->devtools_port;
}

void WebContext::setDevtoolsPort(int port) {
  DCHECK(!IsInitialized());
  construct_props_->devtools_port = port;
}

QString WebContext::devtoolsBindIp() const {
  if (IsInitialized()) {
    return QString::fromStdString(context_->GetDevtoolsBindIp());
  }

  return QString::fromStdString(construct_props_->devtools_ip);
}

void WebContext::setDevtoolsBindIp(const QString& ip) {
  DCHECK(!IsInitialized());
  construct_props_->devtools_ip = ip.toStdString();
}

int WebContext::setCookies(const QUrl& url,
                           const QList<QNetworkCookie>& cookies) {
  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  scoped_refptr<net::CookieStore> cookie_store = context_->GetCookieStore();
  scoped_refptr<SetCookiesContext> ctxt = new SetCookiesContext(request_id);

  for (int i = 0; i < cookies.size(); ++i) {
    const QNetworkCookie& cookie = cookies.at(i);

    if (cookie.name().isEmpty()) {
      ctxt->failed.push_back(cookie);
      continue;
    }

    base::Time expiry;
    if (cookie.expirationDate().isValid()) {
      expiry = base::Time::FromJsTime(cookie.expirationDate().toMSecsSinceEpoch());
    }

    ctxt->remaining++;

    cookie_store->GetCookieMonster()->SetCookieWithDetailsAsync(
        GURL(url.toString().toStdString()),
        std::string(cookie.name().constData()),
        std::string(cookie.value().constData()),
        std::string(cookie.domain().toUtf8().constData()),
        std::string(cookie.path().toUtf8().constData()),
        expiry,
        cookie.isSecure(),
        cookie.isHttpOnly(),
        false,
        net::COOKIE_PRIORITY_DEFAULT,
        base::Bind(&WebContext::CookieSetCallback,
                   weak_factory_.GetWeakPtr(), ctxt, cookie));
  }

  if (ctxt->remaining == 0) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContext::DeliverCookiesSet,
                   weak_factory_.GetWeakPtr(), ctxt));
  }

  return request_id;
}

int WebContext::getCookies(const QUrl& url) {
  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  scoped_refptr<net::CookieStore> store = context_->GetCookieStore();
  store->GetCookieMonster()->GetAllCookiesForURLAsync(
      GURL(url.toString().toStdString()),
      base::Bind(&WebContext::GotCookiesCallback,
                 weak_factory_.GetWeakPtr(), request_id));

  return request_id;
}

int WebContext::getAllCookies() {
  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->GetAllCookiesAsync(
      base::Bind(&WebContext::GotCookiesCallback,
                 weak_factory_.GetWeakPtr(), request_id));

  return request_id;
}

int WebContext::deleteAllCookies() {
  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->DeleteAllAsync(
      base::Bind(&WebContext::DeletedCookiesCallback,
                 weak_factory_.GetWeakPtr(), request_id));

  return request_id;
}

QStringList WebContext::hostMappingRules() const {
  std::vector<std::string> v;
  if (!IsInitialized()) {
    v = construct_props_->host_mapping_rules;
  } else {
    v = context_->GetHostMappingRules();
  }

  QStringList rules;
  for (std::vector<std::string>::const_iterator it = v.cbegin();
       it != v.cend(); ++it) {
    rules.append(QString::fromStdString(*it));
  }

  return rules;
}

void WebContext::setHostMappingRules(const QStringList& rules) {
  DCHECK(!IsInitialized());

  construct_props_->host_mapping_rules.clear();

  for (QStringList::const_iterator it = rules.cbegin();
       it != rules.cend(); ++it) {
    construct_props_->host_mapping_rules.push_back((*it).toStdString());
  }
}

void WebContext::setAllowedExtraUrlSchemes(const QStringList& schemes) {
  std::set<std::string> set;
  for (int i = 0; i < schemes.size(); ++i) {
    set.insert(base::StringToLowerASCII(schemes.at(i).toStdString()));
  }
  delegate_->SetAllowedExtraURLSchemes(set);
}

int WebContext::maxCacheSizeHint() const {
  if (IsInitialized()) {
    return context_->GetMaxCacheSizeHint();
  }

  return construct_props_->max_cache_size_hint;
}

void WebContext::setMaxCacheSizeHint(int size) {
  DCHECK(!IsInitialized());
  construct_props_->max_cache_size_hint = size;
}

QString WebContext::defaultAudioCaptureDeviceId() const {
  if (IsInitialized()) {
    return QString::fromStdString(
        MediaCaptureDevicesContext::Get(context_.get())
          ->GetDefaultAudioDeviceId());
  }

  return QString::fromStdString(
      construct_props_->default_audio_capture_device_id);
}

bool WebContext::setDefaultAudioCaptureDeviceId(const QString& id) {
  if (IsInitialized()) {
    return MediaCaptureDevicesContext::Get(context_.get())
        ->SetDefaultAudioDeviceId(id.toStdString());
  }

  // XXX(chrisccoulson): We don't check if this is a valid ID here
  construct_props_->default_audio_capture_device_id = id.toStdString();
  client_->DefaultAudioCaptureDeviceChanged();
  return true;
}

QString WebContext::defaultVideoCaptureDeviceId() const {
  if (IsInitialized()) {
    return QString::fromStdString(
        MediaCaptureDevicesContext::Get(context_.get())
          ->GetDefaultVideoDeviceId());
  }

  return QString::fromStdString(
      construct_props_->default_video_capture_device_id);
}

bool WebContext::setDefaultVideoCaptureDeviceId(const QString& id) {
  if (IsInitialized()) {
    return MediaCaptureDevicesContext::Get(context_.get())
        ->SetDefaultVideoDeviceId(id.toStdString());
  }

  // XXX(chrisccoulson): We don't check if this is a valid ID here
  construct_props_->default_video_capture_device_id = id.toStdString();
  client_->DefaultVideoCaptureDeviceChanged();
  return true;
}

QList<WebContextProxy::UserAgentOverride>
WebContext::userAgentOverrides() const {
  QList<UserAgentOverride> rv;

  std::vector<UserAgentSettings::UserAgentOverride> overrides;
  if (IsInitialized()) {
    overrides =
        UserAgentSettings::Get(context_.get())->GetUserAgentOverrides();
  } else {
    overrides = construct_props_->user_agent_overrides;
  }

  for (const auto& entry : overrides) {
    rv.append(
        qMakePair(QString::fromStdString(entry.first),
                  QString::fromStdString(entry.second)));
  }

  return rv;
}

void WebContext::setUserAgentOverrides(
    const QList<UserAgentOverride>& overrides) {
  std::vector<UserAgentSettings::UserAgentOverride> o;
  for (auto it = overrides.begin(); it != overrides.end(); ++it) {
    o.push_back(
        std::make_pair((*it).first.toStdString(),
                       (*it).second.toStdString()));
  }

  if (IsInitialized()) {
    UserAgentSettings::Get(context_.get())->SetUserAgentOverrides(o);
  } else {
    construct_props_->user_agent_overrides = o;
  }
}

void WebContext::clearTemporarySavedPermissionStatuses() {
  if (!context_.get()) {
    return;
  }

  context_->GetTemporarySavedPermissionContext()->Clear();
  if (!context_->HasOffTheRecordContext()) {
    return;
  }

  context_->GetOffTheRecordContext()
      ->GetTemporarySavedPermissionContext()
      ->Clear();
}

void WebContext::setLegacyUserAgentOverrideEnabled(bool enabled) {
  if (IsInitialized()) {
    UserAgentSettings::Get(context_.get())->SetLegacyUserAgentOverrideEnabled(
        enabled);
  } else {
    construct_props_->legacy_user_agent_override_enabled = enabled;
  }
}

void WebContext::DefaultAudioDeviceChanged() {
  client_->DefaultAudioCaptureDeviceChanged();
}

void WebContext::DefaultVideoDeviceChanged() {
  client_->DefaultVideoCaptureDeviceChanged();
}

bool WebContext::doNotTrack() const {
    if (IsInitialized()) {
    return context_->GetDoNotTrack();
  }

  return construct_props_->do_not_track;
}

void WebContext::setDoNotTrack(bool dnt) {
  if (IsInitialized()) {
    context_->SetDoNotTrack(dnt);
  } else {
    construct_props_->do_not_track = dnt;
  }
}

} // namespace qt
} // namespace oxide
