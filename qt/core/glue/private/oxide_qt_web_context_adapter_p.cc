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

#include "../oxide_qt_web_context_adapter_p.h"

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
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/resource_type.h"
#include "net/base/net_errors.h"
#include "net/cookies/cookie_monster.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"

#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"
#include "qt/core/api/oxideqstoragepermissionrequest_p.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_context_delegate.h"
#include "shared/browser/oxide_user_script_master.h"

#include "../oxide_qt_user_script_adapter.h"
#include "../oxide_qt_user_script_adapter_p.h"

namespace {
const unsigned kDefaultDevtoolsPort = 8484;
}

namespace oxide {
namespace qt {

class SetCookiesContext : public base::RefCounted<SetCookiesContext> {
 public:
  SetCookiesContext(int id)
      : id(id), remaining(0) {}

  int id;
  int remaining;
  QList<QNetworkCookie> failed;
};

WebContextAdapterPrivate::ConstructProperties::ConstructProperties() :
    cookie_policy(net::StaticCookiePolicy::ALLOW_ALL_COOKIES),
    session_cookie_mode(content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES),
    popup_blocker_enabled(true),
    devtools_enabled(false),
    devtools_port(kDefaultDevtoolsPort) {}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::Create(
    WebContextAdapter* adapter) {
  return new WebContextAdapterPrivate(adapter);
}

WebContextAdapterPrivate::WebContextAdapterPrivate(
    WebContextAdapter* adapter)
    : adapter_(adapter),
      construct_props_(new ConstructProperties()),
      handling_cookie_request_(false) {}

void WebContextAdapterPrivate::Init(
    const QWeakPointer<WebContextAdapter::IODelegate>& io_delegate) {
  base::AutoLock lock(io_delegate_lock_);
  io_delegate_ = io_delegate;
}

void WebContextAdapterPrivate::Destroy() {
  if (context_.get()) {
    context_->SetDelegate(NULL);
  }
  adapter_ = NULL;

  base::AutoLock lock(io_delegate_lock_);
  io_delegate_.clear();
}

QSharedPointer<WebContextAdapter::IODelegate>
WebContextAdapterPrivate::GetIODelegate() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  base::AutoLock lock(io_delegate_lock_);
  return io_delegate_.toStrongRef();
}

void WebContextAdapterPrivate::UpdateUserScripts() {
  if (!context_.get()) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScriptAdapterPrivate* script =
        UserScriptAdapterPrivate::get(user_scripts_.at(i));
    if (script->state == UserScriptAdapterPrivate::Loading ||
        script->state == UserScriptAdapterPrivate::Constructing) {
      return;
    } else if (script->state == UserScriptAdapterPrivate::Loaded) {
      scripts.push_back(&script->user_script);
    }
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

int WebContextAdapterPrivate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return net::OK;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);

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

void WebContextAdapterPrivate::OnBeforeRedirect(
      net::URLRequest* request,
      const GURL& new_location) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);

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

int WebContextAdapterPrivate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {
  QSharedPointer<WebContextAdapter::IODelegate> io_delegate = GetIODelegate();
  if (!io_delegate) {
    return net::OK;
  }

  bool cancelled = false;

  const content::ResourceRequestInfo* info =
      content::ResourceRequestInfo::ForRequest(request);

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

oxide::StoragePermission WebContextAdapterPrivate::CanAccessStorage(
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

bool WebContextAdapterPrivate::GetUserAgentOverride(const GURL& url,
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

WebContextAdapterPrivate::~WebContextAdapterPrivate() {}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::get(
    WebContextAdapter* adapter) {
  return adapter->priv;
}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::FromBrowserContext(
    oxide::BrowserContext* context) {
  return static_cast<WebContextAdapterPrivate *>(context->GetDelegate());
}

void WebContextAdapterPrivate::SetCookies(
    int request_id,
    const QUrl& url,
    const QList<QNetworkCookie>& cookies) {
  DCHECK_GT(request_id, -1);
  DCHECK_GT(cookies.size(), 0);

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
        base::Bind(&WebContextAdapterPrivate::CookieSetCallback,
                   this, ctxt, cookie));
  }

  if (ctxt->remaining == 0) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContextAdapterPrivate::DeliverCookiesSet,
                   this, ctxt));
  }
}

void WebContextAdapterPrivate::CookieSetCallback(
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

void WebContextAdapterPrivate::DeliverCookiesSet(
    const scoped_refptr<SetCookiesContext>& ctxt) {
  DCHECK_EQ(ctxt->remaining, 0);

  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  adapter->CookiesSet(ctxt->id, ctxt->failed);
}

void WebContextAdapterPrivate::GetCookies(int request_id,
                                          const QUrl& url) {
  DCHECK_GT(request_id, -1);

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  scoped_refptr<net::CookieStore> store = context_->GetCookieStore();
  store->GetCookieMonster()->GetAllCookiesForURLAsync(
      GURL(url.toString().toStdString()),
      base::Bind(&WebContextAdapterPrivate::GotCookiesCallback,
                 this, request_id));
}

void WebContextAdapterPrivate::GetAllCookies(int request_id) {
  DCHECK_GT(request_id, -1);

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->GetAllCookiesAsync(
      base::Bind(&WebContextAdapterPrivate::GotCookiesCallback,
                 this, request_id));
}

void WebContextAdapterPrivate::GotCookiesCallback(
    int request_id,
    const net::CookieList& cookies) {
  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  if (handling_cookie_request_) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&WebContextAdapterPrivate::GotCookiesCallback,
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

void WebContextAdapterPrivate::DeleteAllCookies(int request_id) {
  DCHECK_GT(request_id, -1);

  base::AutoReset<bool> f(&handling_cookie_request_, true);

  context_->GetCookieStore()->GetCookieMonster()->DeleteAllAsync(
      base::Bind(&WebContextAdapterPrivate::DeletedCookiesCallback,
                 this, request_id));
}

void WebContextAdapterPrivate::DeletedCookiesCallback(int request_id,
                                                      int num_deleted) {
  WebContextAdapter* adapter = GetAdapter();
  if (!adapter) {
    return;
  }

  adapter->CookiesDeleted(request_id, num_deleted);
}

WebContextAdapter* WebContextAdapterPrivate::GetAdapter() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return adapter_;
}

oxide::BrowserContext* WebContextAdapterPrivate::GetContext() {
  if (context_.get()) {
    return context_.get();
  }

  DCHECK(construct_props_);

  oxide::BrowserContext::Params params(
      construct_props_->data_path,
      construct_props_->cache_path,
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

} // namespace qt
} // namespace oxide
