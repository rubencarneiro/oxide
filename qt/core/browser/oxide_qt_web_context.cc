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

#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkCookie>
#include <QDateTime>
#include <QDebug>
#include <QMetaMethod>

#include "base/auto_reset.h"
#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/resource_type.h"
#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
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
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "shared/browser/oxide_user_script_master.h"

namespace oxide {
namespace qt {

namespace {

const unsigned kDefaultDevtoolsPort = 8484;

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

class SetCookiesContext : public base::RefCounted<SetCookiesContext> {
 public:
  SetCookiesContext(int id)
      : id(id), remaining(0) {}

  int id;
  int remaining;
  QList<QNetworkCookie> failed;
};

WebContext::ConstructProperties::ConstructProperties() :
    max_cache_size_hint(0),
    cookie_policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
    session_cookie_mode(content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
    popup_blocker_enabled(true),
    devtools_enabled(false),
    devtools_port(kDefaultDevtoolsPort) {}

// static
WebContext* WebContext::Create(WebContextAdapter* adapter) {
  return new WebContext(adapter);
}

WebContext::WebContext(WebContextAdapter* adapter)
    : adapter_(adapter),
      construct_props_(new ConstructProperties()),
      handling_cookie_request_(false) {}

void WebContext::Init(
    const QWeakPointer<WebContextAdapter::IODelegate>& io_delegate) {
  base::AutoLock lock(io_delegate_lock_);
  io_delegate_ = io_delegate;
}

void WebContext::Destroy() {
  if (context_.get()) {
    context_->SetDelegate(nullptr);
  }
  adapter_ = nullptr;

  base::AutoLock lock(io_delegate_lock_);
  io_delegate_.clear();
}

WebContextAdapter* WebContext::GetAdapter() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return adapter_;
}

QSharedPointer<WebContextAdapter::IODelegate>
WebContext::GetIODelegate() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  base::AutoLock lock(io_delegate_lock_);
  return io_delegate_.toStrongRef();
}

void WebContext::UpdateUserScripts() {
  if (!context_.get()) {
    return;
  }

  std::vector<const oxide::UserScript *> scripts;

  for (int i = 0; i < adapter_->user_scripts_.size(); ++i) {
    UserScript* script = UserScript::FromAdapter(adapter_->user_scripts_.at(i));
    if (script->state() == UserScript::Loading ||
        script->state() == UserScript::Constructing) {
      return;
    } else if (script->state() == UserScript::Loaded) {
      scripts.push_back(script->impl());
    }
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

int WebContext::OnBeforeURLRequest(net::URLRequest* request,
                                   const net::CompletionCallback& callback,
                                   GURL* new_url) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return net::OK;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return net::OK;
  }

  OxideQBeforeURLRequestEvent* event =
      new OxideQBeforeURLRequestEvent(
        QUrl(QString::fromStdString(request->url().spec())),
        QString::fromStdString(request->method()),
        QString::fromStdString(request->referrer()),
        info->IsMainFrame());

  OxideQBeforeURLRequestEventPrivate* eventp =
      OxideQBeforeURLRequestEventPrivate::get(event);
  eventp->request_cancelled = &cancelled;
  eventp->new_url = new_url;

  io_delegate->OnBeforeURLRequest(event);

  return cancelled ? net::ERR_ABORTED : net::OK;
}

int WebContext::OnBeforeSendHeaders(net::URLRequest* request,
                                    const net::CompletionCallback& callback,
                                    net::HttpRequestHeaders* headers) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return net::OK;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return net::OK;
  }

  OxideQBeforeSendHeadersEvent* event =
      new OxideQBeforeSendHeadersEvent(
        QUrl(QString::fromStdString(request->url().spec())),
        QString::fromStdString(request->method()),
        QString::fromStdString(request->referrer()),
        info->IsMainFrame());

  OxideQBeforeSendHeadersEventPrivate* eventp =
      OxideQBeforeSendHeadersEventPrivate::get(event);
  eventp->request_cancelled = &cancelled;
  eventp->headers = headers;

  io_delegate->OnBeforeSendHeaders(event);

  return cancelled ? net::ERR_ABORTED : net::OK;
}

void WebContext::OnBeforeRedirect(net::URLRequest* request,
                                  const GURL& new_location) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);
  if (!info) {
    // Requests created outside of the ResourceDispatcher won't have
    // a ResourceRequestInfo
    return;
  }

  OxideQBeforeRedirectEvent* event =
      new OxideQBeforeRedirectEvent(
        QUrl(QString::fromStdString(new_location.spec())),
        QString::fromStdString(request->method()),
        QString::fromStdString(request->referrer()),
        info->IsMainFrame(),
        QUrl(QString::fromStdString(request->original_url().spec())));

  OxideQBeforeRedirectEventPrivate* eventp =
      OxideQBeforeRedirectEventPrivate::get(event);
  eventp->request_cancelled = &cancelled;

  io_delegate->OnBeforeRedirect(event);

  if (cancelled) {
    request->Cancel();
  }
}

oxide::StoragePermission WebContext::CanAccessStorage(
    const GURL& url,
    const GURL& first_party_url,
    bool write,
    oxide::StorageType type) {
  oxide::StoragePermission result = oxide::STORAGE_PERMISSION_UNDEFINED;

  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return result;
  }

  OxideQStoragePermissionRequest* req =
      new OxideQStoragePermissionRequest(
        QUrl(QString::fromStdString(url.spec())),
        QUrl(QString::fromStdString(first_party_url.spec())),
        write,
        static_cast<OxideQStoragePermissionRequest::Type>(type));

  OxideQStoragePermissionRequestPrivate::get(req)->permission = &result;

  io_delegate->HandleStoragePermissionRequest(req);

  return result;
}

bool WebContext::GetUserAgentOverride(const GURL& url,
                                      std::string* user_agent) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return false;
  }

  QString new_user_agent;
  bool overridden = io_delegate->GetUserAgentOverride(
      QUrl(QString::fromStdString(url.spec())), &new_user_agent);

  *user_agent = new_user_agent.toStdString();
  return overridden;
}

bool WebContext::IsCustomProtocolHandlerRegistered(
    const std::string& scheme) const {
  base::AutoLock lock(url_schemes_lock_);
  return allowed_extra_url_schemes_.find(scheme) !=
      allowed_extra_url_schemes_.end();
}

oxide::URLRequestDelegatedJob* WebContext::CreateCustomURLRequestJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  return new URLRequestDelegatedJob(this, request, network_delegate);
}

WebContext::~WebContext() {}

// static
WebContext* WebContext::FromAdapter(WebContextAdapter* adapter) {
  return adapter->context_;
}

// static
WebContext* WebContext::FromBrowserContext(oxide::BrowserContext* context) {
  return static_cast<WebContext *>(context->GetDelegate());
}

int WebContext::SetCookies(const QUrl& url,
                           const QList<QNetworkCookie>& cookies) {
  if (!IsInitialized()) {
    return -1;
  }

  if (cookies.size() == 0) {
    return -1;
  }

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
        net::COOKIE_PRIORITY_DEFAULT,
        base::Bind(&WebContext::CookieSetCallback, this, ctxt, cookie));
  }

  if (ctxt->remaining == 0) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContext::DeliverCookiesSet, this, ctxt));
  }

  return request_id;
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

  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  adapter->CookiesSet(ctxt->id, ctxt->failed);
}

int WebContext::GetCookies(const QUrl& url) {
  if (!IsInitialized()) {
    return -1;
  }

  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  scoped_refptr<net::CookieStore> store = context_->GetCookieStore();
  store->GetCookieMonster()->GetAllCookiesForURLAsync(
      GURL(url.toString().toStdString()),
      base::Bind(&WebContext::GotCookiesCallback, this, request_id));

  return request_id;
}

int WebContext::GetAllCookies() {
  if (!IsInitialized()) {
    return -1;
  }

  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->GetAllCookiesAsync(
      base::Bind(&WebContext::GotCookiesCallback, this, request_id));

  return request_id;
}

void WebContext::GotCookiesCallback(int request_id,
                                    const net::CookieList& cookies) {
  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  if (handling_cookie_request_) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContext::GotCookiesCallback,
                   this, request_id, cookies));
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

  adapter->CookiesRetrieved(request_id, qcookies);
}

int WebContext::DeleteAllCookies() {
  if (!IsInitialized()) {
    return -1;
  }

  int request_id = GetNextCookieRequestId();

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->DeleteAllAsync(
      base::Bind(&WebContext::DeletedCookiesCallback,
                 this, request_id));

  return request_id;
}

void WebContext::DeletedCookiesCallback(int request_id,
                                        int num_deleted) {
  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  adapter->CookiesDeleted(request_id, num_deleted);
}

void WebContext::SetAllowedExtraURLSchemes(
    const std::set<std::string>& schemes) {
  base::AutoLock lock(url_schemes_lock_);
  allowed_extra_url_schemes_ = schemes;
}

// static
WebContext* WebContext::GetDefault() {
  WebContextAdapter* adapter = WebContextAdapter::GetDefault();
  if (!adapter) {
    return nullptr;
  }

  return FromAdapter(adapter);
}

// static
void WebContext::DestroyDefault() {
  WebContextAdapter::DestroyDefault();
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

  if (!construct_props_->product.empty()) {
    context_->SetProduct(construct_props_->product);
  }
  if (!construct_props_->user_agent.empty()) {
    context_->SetUserAgent(construct_props_->user_agent);
  }
  if (!construct_props_->accept_langs.empty()) {
    context_->SetAcceptLangs(construct_props_->accept_langs);
  }
  context_->SetCookiePolicy(construct_props_->cookie_policy);
  context_->SetIsPopupBlockerEnabled(construct_props_->popup_blocker_enabled);

  context_->SetDelegate(this);

  construct_props_.reset();

  UpdateUserScripts();

  return context_.get();
}

QNetworkAccessManager* WebContext::GetCustomNetworkAccessManager() {
  return GetAdapter()->GetCustomNetworkAccessManager();
}

bool WebContext::IsInitialized() const {
  return context_.get() != nullptr;
}

std::string WebContext::GetProduct() const {
  if (IsInitialized()) {
    return context_->GetProduct();
  }

  return construct_props_->product;
}

void WebContext::SetProduct(const std::string& product) {
  if (IsInitialized()) {
    context_->SetProduct(product);
  } else {
    construct_props_->product = product;
  }
}

std::string WebContext::GetUserAgent() const {
  if (IsInitialized()) {
    return context_->GetUserAgent();
  }

  return construct_props_->user_agent;
}

void WebContext::SetUserAgent(const std::string& user_agent) {
  if (IsInitialized()) {
    context_->SetUserAgent(user_agent);
  } else {
    construct_props_->user_agent = user_agent;
  }
}

base::FilePath WebContext::GetDataPath() const {
  if (IsInitialized()) {
    return context_->GetPath();
  }

  return construct_props_->data_path;
}

void WebContext::SetDataPath(const base::FilePath& path) {
  DCHECK(!IsInitialized());
  construct_props_->data_path = path;
}

base::FilePath WebContext::GetCachePath() const {
  if (IsInitialized()) {
    return context_->GetCachePath();
  }

  return construct_props_->cache_path;
}

void WebContext::SetCachePath(const base::FilePath& path) {
  DCHECK(!IsInitialized());
  construct_props_->cache_path = path;
}

std::string WebContext::GetAcceptLangs() const {
  if (IsInitialized()) {
    return context_->GetAcceptLangs();
  }

  return construct_props_->accept_langs;
}

void WebContext::SetAcceptLangs(const std::string& langs) {
  if (IsInitialized()) {
    context_->SetAcceptLangs(langs);
  } else {
    construct_props_->accept_langs = langs;
  }
}

net::StaticCookiePolicy::Type WebContext::GetCookiePolicy() const {
  if (IsInitialized()) {
    return context_->GetCookiePolicy();
  }

  return construct_props_->cookie_policy;
}

void WebContext::SetCookiePolicy(net::StaticCookiePolicy::Type policy) {
  if (IsInitialized()) {
    context_->SetCookiePolicy(policy);
  } else {
    construct_props_->cookie_policy = policy;
  }
}

content::CookieStoreConfig::SessionCookieMode
WebContext::GetSessionCookieMode() const {
  if (IsInitialized()) {
    return context_->GetSessionCookieMode();
  }

  return construct_props_->session_cookie_mode;
}

void WebContext::SetSessionCookieMode(
    content::CookieStoreConfig::SessionCookieMode mode) {
  DCHECK(!IsInitialized());
  construct_props_->session_cookie_mode = mode;
}

bool WebContext::GetPopupBlockerEnabled() const {
  if (IsInitialized()) {
    return context_->IsPopupBlockerEnabled();
  }

  return construct_props_->popup_blocker_enabled;
}

void WebContext::SetPopupBlockerEnabled(bool enabled) {
  if (IsInitialized()) {
    context_->SetIsPopupBlockerEnabled(enabled);
  } else {
    construct_props_->popup_blocker_enabled = enabled;
  }
}

bool WebContext::GetDevtoolsEnabled() const {
  if (IsInitialized()) {
    return context_->GetDevtoolsEnabled();
  }

  return construct_props_->devtools_enabled;
}

void WebContext::SetDevtoolsEnabled(bool enabled) {
  if (IsInitialized()) {
    qWarning() << "Cannot change the devtools enabled after inititialization";
    return;
  }

  construct_props_->devtools_enabled = enabled;
}

int WebContext::GetDevtoolsPort() const {
  if (IsInitialized()) {
    return context_->GetDevtoolsPort();
  }

  return construct_props_->devtools_port;
}

void WebContext::SetDevtoolsPort(int port) {
  if (IsInitialized()) {
    qWarning() << "Cannot change the devtools port after inititialization";
    return;
  }

  construct_props_->devtools_port = port;
}

std::string WebContext::GetDevtoolsBindIp() const {
  if (IsInitialized()) {
    return context_->GetDevtoolsBindIp();
  }

  return construct_props_->devtools_ip;
}

void WebContext::SetDevtoolsBindIp(const std::string& ip) {
  if (IsInitialized()) {
    qWarning() << "Cannot change the devtools bound ip after inititialization";
    return;
  }

  construct_props_->devtools_ip = ip;
}

std::vector<std::string> WebContext::GetHostMappingRules() const {
  if (!IsInitialized()) {
    return construct_props_->host_mapping_rules;
  }

  return context_->GetHostMappingRules();
}

void WebContext::SetHostMappingRules(const std::vector<std::string>& rules) {
  DCHECK(!IsInitialized());
  construct_props_->host_mapping_rules = rules;
}

int WebContext::GetMaxCacheSizeHint() const {
  if (IsInitialized()) {
    return context_->GetMaxCacheSizeHint();
  }

  return construct_props_->max_cache_size_hint;
}

void WebContext::SetMaxCacheSizeHint(int size) {
  DCHECK(!IsInitialized());
  construct_props_->max_cache_size_hint = size;
}

} // namespace qt
} // namespace oxide
