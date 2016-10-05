// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxideqquickwebcontext.h"
#include "oxideqquickwebcontext_p.h"

#include <limits>

#include <QMutex>
#include <QMutexLocker>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QSharedPointer>
#include <QtDebug>
#include <QThread>
#include <QtQuickVersion>
#include <QWeakPointer>

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/api/oxideqnetworkcallbackevents_p.h"
#include "qt/core/glue/oxide_qt_web_context_proxy.h"
#include "qt/quick/oxide_qquick_init.h"

#include "oxideqquickcookiemanager_p.h"
#include "oxideqquickuserscript.h"
#include "oxideqquickuserscript_p.h"
#include "oxideqquickwebcontextdelegateworker_p.h"
#include "oxideqquickwebcontextdelegateworker_p_p.h"
#include "oxidequseragentoverriderequest_p.h"
#include "oxidequseragentoverriderequest_p_p.h"

using oxide::qt::WebContextProxy;

namespace {

bool g_default_context_initialized = false;
OxideQQuickWebContext* g_default_context;

void CleanupDefaultContext() {
  delete g_default_context;
  Q_ASSERT(!g_default_context);
}

QVariant networkCookiesToVariant(const QList<QNetworkCookie>& cookies) {
  QList<QVariant> list;
  Q_FOREACH(QNetworkCookie cookie, cookies) {
    QVariantMap c;
    c.insert("name", QVariant(QString(cookie.name())));
    c.insert("value", QVariant(QString(cookie.value())));
    c.insert("domain", QVariant(cookie.domain()));
    c.insert("path", QVariant(cookie.path()));
    c.insert("httponly", QVariant(cookie.isHttpOnly()));
    c.insert("issecure", QVariant(cookie.isSecure()));
    c.insert("issessioncookie", QVariant(cookie.isSessionCookie()));
    if (cookie.expirationDate().isValid()) {
      c.insert("expirationdate", QVariant(cookie.expirationDate()));
    } else {
      c.insert("expirationdate", QVariant());
    }

    list.append(c);
  }

  return QVariant(list);
}

WebContextProxy::UserAgentOverride ToUserAgentOverride(
    const QVariant& v,
    bool* valid) {
  *valid = false;

  QVariantList entry = v.toList();
  if (entry.size() != 2) {
    return WebContextProxy::UserAgentOverride();
  }

  WebContextProxy::UserAgentOverride rv;

  QRegExp re;
  if (entry[0].canConvert<QRegExp>()) {
    re = entry[0].toRegExp();
  } else {
    re = QRegExp(entry[0].toString());
  }

  if (!re.isValid()) {
    return WebContextProxy::UserAgentOverride();
  }

  *valid = true;

  rv.first = re.pattern();
  rv.second = entry[1].toString();

  return rv;
}

QVariant FromUserAgentOverride(
    const WebContextProxy::UserAgentOverride& o) {
  QVariantList rv;
  rv.append(o.first);
  rv.append(o.second);

  return rv;
}

}

namespace oxide {
namespace qquick {

using namespace webcontextdelegateworker;

class WebContextIODelegate : public oxide::qt::WebContextProxyClient::IOClient {
 public:
  WebContextIODelegate() {}
  virtual ~WebContextIODelegate() {}

  void OnBeforeURLRequest(OxideQBeforeURLRequestEvent* event) override;
  void OnBeforeRedirect(OxideQBeforeRedirectEvent* event) override;
  void OnBeforeSendHeaders(OxideQBeforeSendHeadersEvent* event) override;
  QString GetUserAgentOverride(const QUrl& url) override;

  QMutex lock;

  QWeakPointer<IOThreadController> network_request_delegate;
  QWeakPointer<IOThreadController> user_agent_override_delegate;
};

void WebContextIODelegate::OnBeforeURLRequest(
    OxideQBeforeURLRequestEvent* event) {
  QSharedPointer<IOThreadController> delegate;
  {
    QMutexLocker locker(&lock);
    delegate = network_request_delegate.toStrongRef();
  }
  if (!delegate) {
    return;
  }

  delegate->CallEntryPointInWorker("onBeforeURLRequest", event);
}

void WebContextIODelegate::OnBeforeSendHeaders(
    OxideQBeforeSendHeadersEvent* event) {
  QSharedPointer<IOThreadController> delegate;
  {
    QMutexLocker locker(&lock);
    delegate = network_request_delegate.toStrongRef();
  }
  if (!delegate) {
    return;
  }

  delegate->CallEntryPointInWorker("onBeforeSendHeaders", event);
}

void WebContextIODelegate::OnBeforeRedirect(
    OxideQBeforeRedirectEvent* event) {
  QSharedPointer<IOThreadController> delegate;
  {
    QMutexLocker locker(&lock);
    delegate = network_request_delegate.toStrongRef();
  }
  if (!delegate) {
    return;
  }

  emit delegate->CallEntryPointInWorker("onBeforeRedirect", event);
}

QString WebContextIODelegate::GetUserAgentOverride(const QUrl& url) {
  QSharedPointer<IOThreadController> delegate;
  {
    QMutexLocker locker(&lock);
    delegate = user_agent_override_delegate.toStrongRef();
  }
  if (!delegate) {
    return QString();
  }

  OxideQUserAgentOverrideRequest req(url);
  delegate->CallEntryPointInWorker("onGetUserAgentOverride", &req);

  OxideQUserAgentOverrideRequestPrivate* p =
      OxideQUserAgentOverrideRequestPrivate::get(&req);

  return p->user_agent;
}

} // namespace qquick
} // namespace oxide

using namespace oxide::qquick;

OxideQQuickWebContextPrivate::OxideQQuickWebContextPrivate(
    OxideQQuickWebContext* q)
    : q_ptr(q),
      proxy_(WebContextProxy::create(this, q)),
      constructed_(false),
      io_(new oxide::qquick::WebContextIODelegate()),
      cookie_manager_(nullptr) {}

void OxideQQuickWebContextPrivate::userScriptUpdated() {
  proxy_->updateUserScripts();
}

void OxideQQuickWebContextPrivate::userScriptWillBeDeleted() {
  Q_Q(OxideQQuickWebContext);

  OxideQQuickUserScriptPrivate* sender =
      qobject_cast<OxideQQuickUserScriptPrivate *>(q->sender());
  Q_ASSERT(sender);
  q->removeUserScript(sender->q());
}

void OxideQQuickWebContextPrivate::detachUserScriptSignals(
    OxideQQuickUserScript* user_script) {
  Q_Q(OxideQQuickWebContext);

  QObject::disconnect(user_script, SIGNAL(scriptLoaded()),
                      q, SLOT(userScriptUpdated()));
  QObject::disconnect(user_script, SIGNAL(scriptPropertyChanged()),
                      q, SLOT(userScriptUpdated()));
  QObject::disconnect(OxideQQuickUserScriptPrivate::get(user_script),
                      SIGNAL(willBeDeleted()),
                      q, SLOT(userScriptWillBeDeleted()));
}

void OxideQQuickWebContextPrivate::userScript_append(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    OxideQQuickUserScript* user_script) {
  if (!user_script) {
    return;
  }

  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);

  context->addUserScript(user_script);
}

int OxideQQuickWebContextPrivate::userScript_count(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  WebContextProxy* p =
      OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext*>(prop->object))->proxy_.data();

  return p->userScripts().size();
}

OxideQQuickUserScript* OxideQQuickWebContextPrivate::userScript_at(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    int index) {
  WebContextProxy* p =
      OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext*>(prop->object))->proxy_.data();

  if (index >= p->userScripts().size()) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickUserScript*>(p->userScripts().at(index));
}

void OxideQQuickWebContextPrivate::userScript_clear(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);
  WebContextProxy* p =
      OxideQQuickWebContextPrivate::get(context)->proxy_.data();

  while (p->userScripts().size() > 0) {
    context->removeUserScript(
        qobject_cast<OxideQQuickUserScript*>(p->userScripts().at(0)));
  }
}

bool OxideQQuickWebContextPrivate::prepareToAttachDelegateWorker(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_Q(OxideQQuickWebContext);

  OxideQQuickWebContextDelegateWorkerPrivate* p =
      OxideQQuickWebContextDelegateWorkerPrivate::get(delegate);
  Q_ASSERT((!p->context && p->attached_count == 0) ||
           (p->context && p->attached_count > 0));

  if (p->context && p->context != q) {
    qWarning() << "Can't add WebContextDelegateWorker to more than one WebContext";
    return false;
  }

  if (!p->context) {
    p->context = q;
    if (!delegate->parent()) {
      delegate->setParent(q);
      p->owned_by_context = true;
    }
  }

  ++p->attached_count;

  Q_ASSERT(p->io_thread_controller().data());

  return true;
}

void OxideQQuickWebContextPrivate::detachedDelegateWorker(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_Q(OxideQQuickWebContext);

  if (!delegate) {
    return;
  }

  OxideQQuickWebContextDelegateWorkerPrivate* p =
      OxideQQuickWebContextDelegateWorkerPrivate::get(delegate);
  Q_ASSERT(p->context == q);
  Q_ASSERT(p->attached_count > 0);

  if (--p->attached_count > 0) {
    return;
  }

  p->context = nullptr;

  if (p->in_destruction() || delegate->parent() != q || !p->owned_by_context) {
    p->owned_by_context = false;
    return;
  }

  delete delegate;
}

void OxideQQuickWebContextPrivate::CookiesSet(
    int request_id,
    const QList<QNetworkCookie>& failed_cookies) {
  emit cookie_manager_->setCookiesResponse(
      request_id,
      networkCookiesToVariant(failed_cookies));
}

void OxideQQuickWebContextPrivate::CookiesRetrieved(
      int request_id,
      const QList<QNetworkCookie>& cookies) {
  emit cookie_manager_->getCookiesResponse(
      request_id,
      networkCookiesToVariant(cookies));
}

void OxideQQuickWebContextPrivate::CookiesDeleted(
    int request_id, int num_deleted) {
  emit cookie_manager_->deleteCookiesResponse(request_id, num_deleted);
}

QNetworkAccessManager*
OxideQQuickWebContextPrivate::GetCustomNetworkAccessManager() {
  Q_Q(OxideQQuickWebContext);

  QQmlEngine* engine = qmlEngine(q);
  if (!engine) {
    return nullptr;
  }

  return engine->networkAccessManager();
}

void OxideQQuickWebContextPrivate::DefaultAudioCaptureDeviceChanged() {
  Q_Q(OxideQQuickWebContext);

  emit q->defaultAudioCaptureDeviceIdChanged();
}

void OxideQQuickWebContextPrivate::DefaultVideoCaptureDeviceChanged() {
  Q_Q(OxideQQuickWebContext);

  emit q->defaultVideoCaptureDeviceIdChanged();
}

OxideQQuickWebContextPrivate::~OxideQQuickWebContextPrivate() {}

void OxideQQuickWebContextPrivate::delegateWorkerDestroyed(
    OxideQQuickWebContextDelegateWorker* worker) {
  Q_Q(OxideQQuickWebContext);

  if (worker == q->networkRequestDelegate()) {
    q->setNetworkRequestDelegate(nullptr);
  }
  if (worker == q->storageAccessPermissionDelegate()) {
    q->setStorageAccessPermissionDelegate(nullptr);
  }
  if (worker == q->userAgentOverrideDelegate()) {
    q->setUserAgentOverrideDelegate(nullptr);
  }
}

OxideQQuickWebContextPrivate* OxideQQuickWebContextPrivate::get(
    OxideQQuickWebContext* context) {
  return context->d_func();
}

void OxideQQuickWebContextPrivate::clearTemporarySavedPermissionStatuses() {
  proxy_->clearTemporarySavedPermissionStatuses();
}

bool OxideQQuickWebContextPrivate::isInitialized() const {
  return proxy_->isInitialized();
}

int OxideQQuickWebContextPrivate::setCookies(
    const QUrl& url,
    const QList<QNetworkCookie>& cookies) {
  return proxy_->setCookies(url, cookies);
}

int OxideQQuickWebContextPrivate::getCookies(const QUrl& url) {
  return proxy_->getCookies(url);
}

int OxideQQuickWebContextPrivate::getAllCookies() {
  return proxy_->getAllCookies();
}

int OxideQQuickWebContextPrivate::deleteAllCookies() {
  return proxy_->deleteAllCookies();
}

/*!
\class OxideQQuickWebContext
\inmodule OxideQtQuick
\inheaderfile oxideqquickwebcontext.h

\brief Manages state shared between web views
*/

/*!
\qmltype WebContext
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickWebContext

\brief Manages state shared between web views

WebContext manages state shared between web views - this includes things like
the cookie database, network cache and local storage. Applications can set
dataPath and cachePath during construction to specify storage locations.

Applications can loosely control the maximum size of the network cache by
setting maxCacheSizeHint during construction.

The user agent string used by web views can be customized using the userAgent
and \l{product} properties. These control the \e{User-Agent} HTTP header and the
value of \e{navigator.userAgent}. Per-URL user-agent string overrides can also
be provided by using userAgentOverrides.

WebContext also provides a mechanism to inject scripts in to web pages via
userScripts, addUserScript and removeUserScript.

WebContext allows some cookie behaviour to be configured, using
sessionCookieMode and cookiePolicy.

If Oxide::processModel is \e{Oxide.ProcessModelSingleProcess}, the first created
WebContext will become the application default WebContext (the one provided by
Oxide::defaultWebContext) if that hasn't already been created.
*/

void OxideQQuickWebContext::classBegin() {}

void OxideQQuickWebContext::componentComplete() {
  Q_D(OxideQQuickWebContext);

  d->constructed_ = true;
  emit d->constructed();
}

/*!
\internal
*/

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent)
    : QObject(parent) {
  oxide::qquick::EnsureChromiumStarted();
  d_ptr.reset(new OxideQQuickWebContextPrivate(this));

  Q_D(OxideQQuickWebContext);

  QSharedPointer<oxide::qt::WebContextProxyClient::IOClient> io =
      qSharedPointerCast<oxide::qt::WebContextProxyClient::IOClient>(d->io_);
  d->proxy_->init(io.toWeakRef());

  if (oxideGetProcessModel() == OxideProcessModelSingleProcess &&
      !g_default_context_initialized) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
    g_default_context_initialized = true;
  }
}

/*!
Destroy this web context.
*/

OxideQQuickWebContext::~OxideQQuickWebContext() {
  Q_D(OxideQQuickWebContext);

  if (g_default_context == this) {
    g_default_context = nullptr;
  }

  for (int i = 0; i < d->proxy_->userScripts().size(); ++i) {
    d->detachUserScriptSignals(
        qobject_cast<OxideQQuickUserScript*>(d->proxy_->userScripts().at(i)));
  }

  // These call back in to us when destroyed, so delete them now if we own them
  // in order to avoid a reentrancy crash
  setNetworkRequestDelegate(nullptr);
  setStorageAccessPermissionDelegate(nullptr);
  setUserAgentOverrideDelegate(nullptr);
}

// static
OxideQQuickWebContext* OxideQQuickWebContext::defaultContext(bool create) {
  if (g_default_context) {
    return g_default_context;
  }

  if (!create) {
    return nullptr;
  }

  if (g_default_context_initialized) {
    qFatal("OxideQQuickWebContext::defaultContext: The default context has "
           "been deleted. This can happen in single process mode when the "
           "application creates and manages the default context");
    return nullptr;
  }

  g_default_context_initialized = true;

  g_default_context = new OxideQQuickWebContext();
  g_default_context->componentComplete();
  QQmlEngine::setObjectOwnership(g_default_context, QQmlEngine::CppOwnership);

  oxideAddShutdownCallback(CleanupDefaultContext);

  return g_default_context;
}

/*!
\qmlproperty string WebContext::product

The product name used to build the default user agent string. Setting this will
cause userAgent to change if it is set to the default.

The default value is "Chrome/X.X.X.X", where "X.X.X.X" is the Chromium version
that this Oxide build is based on.

Setting this to an empty string will restore the default value.

\sa userAgent
*/

QString OxideQQuickWebContext::product() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->product();
}

void OxideQQuickWebContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->product() == product) {
    return;
  }

  QString old_user_agent = userAgent();

  d->proxy_->setProduct(product);
  emit productChanged();

  if (userAgent() != old_user_agent) {
    emit userAgentChanged();
  }
}

/*!
\qmlproperty string WebContext::userAgent

The default user agent string used for the \e{User-Agent} header in HTTP
requests and for \e{navigator.userAgent}. By default, this is the Chrome
user-agent string, with the product name based on the value of \l{product}.

Setting this to an empty string will restore the default value.

\sa product
*/

QString OxideQQuickWebContext::userAgent() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->userAgent();
}

void OxideQQuickWebContext::setUserAgent(const QString& userAgent) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->userAgent() == userAgent) {
    return;
  }

  d->proxy_->setUserAgent(userAgent);
  emit userAgentChanged();
}

/*!
\qmlproperty url WebContext::dataPath

The location of non-cache related persistent data files, such as the cookie
database or local storage.

This can be set during construction of the WebContext. Attempts to change it
afterwards will be ignored.

To define the location of non-cache related persistent data files, this must be
set to a local (file:) URL. Non-local schemes are not supported, and will be
ignored.
*/

QUrl OxideQQuickWebContext::dataPath() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->dataPath();
}

/*!
\internal
*/

void OxideQQuickWebContext::setDataPath(const QUrl& dataUrl) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set dataPath once the context is in use";
    return;
  }

  if (dataPath() == dataUrl) {
    return;
  }

  if (!dataUrl.isLocalFile() && !dataUrl.isEmpty()) {
    qWarning() << "OxideQQuickWebContext: dataPath only supports local files";
    return;
  }

  d->proxy_->setDataPath(dataUrl);
  emit dataPathChanged();
}

/*!
\qmlproperty url WebContext::cachePath

The location of cache related persistent data files, such as the network cache.

This can be set during construction of the WebContext. Attempts to change it
afterwards will be ignored.

To define the location of cache related persistent data files, this must be
set to a local (file:) URL. Non-local schemes are not supported, and will be
ignored.

If this is not set, Oxide will fall back to using dataPath if that is set.
*/

QUrl OxideQQuickWebContext::cachePath() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->cachePath();
}

/*!
\internal
*/

void OxideQQuickWebContext::setCachePath(const QUrl& cacheUrl) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->isInitialized()) {
    qWarning() << "OxideQQuickWebContext:: Cannot set cachePath once the context is in use";
    return;
  }

  if (cachePath() == cacheUrl) {
    return;
  }

  if (!cacheUrl.isLocalFile() && !cacheUrl.isEmpty()) {
    qWarning() << "OxideQQuickWebContext: cachePath only supports local files";
    return;
  }

  d->proxy_->setCachePath(cacheUrl);
  emit cachePathChanged();
}

/*!
\qmlproperty string WebContext::acceptLangs

The value used to determine the contents of \e{navigator.languages} and of the
HTTP \e{Accept-Language} header.

Applications can specify this by setting it to a comma delimited list of
language codes in order of preference (starting with the most-preferred). Oxide
will automatically convert this in to the format used by the HTTP
\e{Accept-Language} header, including quality values.
*/

QString OxideQQuickWebContext::acceptLangs() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->acceptLangs();
}

void OxideQQuickWebContext::setAcceptLangs(const QString& acceptLangs) {
  Q_D(OxideQQuickWebContext);

  if (this->acceptLangs() == acceptLangs) {
    return;
  }

  d->proxy_->setAcceptLangs(acceptLangs);
  emit acceptLangsChanged();
}

/*!
\qmlproperty list<UserScript> WebContext::userScripts

This property holds the list of user scripts that will be injected in to web
pages.
*/

QQmlListProperty<OxideQQuickUserScript>
OxideQQuickWebContext::userScripts() {
  return QQmlListProperty<OxideQQuickUserScript>(
      this, nullptr,
      OxideQQuickWebContextPrivate::userScript_append,
      OxideQQuickWebContextPrivate::userScript_count,
      OxideQQuickWebContextPrivate::userScript_at,
      OxideQQuickWebContextPrivate::userScript_clear);
}

/*!
\qmlmethod void WebContext::addUserScript(UserScript script)

Append \a{script} to the list of user scripts.

If \a{script} doesn't have a parent, then this WebContext will set itself as
the parent and assume ownership of it.

If \a{script} already exists in the list of user scripts, then it will be moved
to the end of the list.

If \a{script} is deleted after being added to the list of scripts, then it will
be removed from the list automatically.
*/

void OxideQQuickWebContext::addUserScript(OxideQQuickUserScript* script) {
  Q_D(OxideQQuickWebContext);

  if (!script) {
    qWarning() << "Must specify a user script";
    return;
  }

  OxideQQuickUserScriptPrivate* ud =
      OxideQQuickUserScriptPrivate::get(script);

  if (!d->proxy_->userScripts().contains(script)) {
    connect(script, SIGNAL(scriptLoaded()),
            this, SLOT(userScriptUpdated()));
    connect(script, SIGNAL(scriptPropertyChanged()),
            this, SLOT(userScriptUpdated()));
    connect(ud, SIGNAL(willBeDeleted()),
            this, SLOT(userScriptWillBeDeleted()));
  } else {
    d->proxy_->userScripts().removeOne(script);
  }

  if (!script->parent()) {
    script->setParent(this);
  }
  d->proxy_->userScripts().append(script);

  emit userScriptsChanged();
}

/*!
\qmlmethod void WebContext::removeUserScript(UserScript script)

Remove \a{script} from the list of user scripts.

If \a{script} is owned by this WebContext, then removing it will cause its
parent to be cleared. This means that \a{script} will become unowned.
*/

void OxideQQuickWebContext::removeUserScript(OxideQQuickUserScript* script) {
  Q_D(OxideQQuickWebContext);

  if (!script) {
    qWarning() << "Must specify a user script";
    return;
  }

  if (!d->proxy_->userScripts().contains(script)) {
    return;
  }

  d->detachUserScriptSignals(script);
  if (script->parent() == this) {
    script->setParent(nullptr);
  }

  d->proxy_->userScripts().removeOne(script);

  emit userScriptsChanged();
}

/*!
\qmlproperty enumeration WebContext::cookiePolicy

The cookie policy. This can be set to one of the following values:

\value WebContext.CookiePolicyAllowAll
No cookie blocking is performed. This doesn't affect cookie blocking features
for individual cookies (eg, Secure, HttpOnly, SameSite).

\value WebContext.CookiePolicyBlockAll
Block all cookies from being set or read.

\value WebContext.CookiePolicyBlockThirdParty
Prevent third-party cookies from being set or read. No cookie blocking is
performed for first-party cookies.

The default value is WebContext.CookiePolicyAllowAll
*/

OxideQQuickWebContext::CookiePolicy OxideQQuickWebContext::cookiePolicy() const {
  Q_D(const OxideQQuickWebContext);

  Q_STATIC_ASSERT(
      CookiePolicyAllowAll ==
      static_cast<CookiePolicy>(
        WebContextProxy::CookiePolicyAllowAll));
  Q_STATIC_ASSERT(
      CookiePolicyBlockAll ==
      static_cast<CookiePolicy>(
        WebContextProxy::CookiePolicyBlockAll));
  Q_STATIC_ASSERT(
      CookiePolicyBlockThirdParty ==
      static_cast<CookiePolicy>(
        WebContextProxy::CookiePolicyBlockThirdParty));

  return static_cast<CookiePolicy>(d->proxy_->cookiePolicy());
}

void OxideQQuickWebContext::setCookiePolicy(CookiePolicy policy) {
  Q_D(OxideQQuickWebContext);

  WebContextProxy::CookiePolicy p =
      static_cast<WebContextProxy::CookiePolicy>(policy);

  if (policy == cookiePolicy()) {
    return;
  }

  d->proxy_->setCookiePolicy(p);

  emit cookiePolicyChanged();
}

/*!
\qmlproperty enumeration WebContext::sessionCookieMode

Determines the restoration behaviour of session cookies. This can be set during
construction only, and may be set to one of the following values:

\value WebContext.SessionCookieModeEphemeral
Session cookies that exist in the cookie database from the previous session are
discarded, and no session cookies created during this session are persisted to
the cookie database in a way that allows them to be restored.

\value WebContext.SessionCookieModePersistent
Session cookies that exist in the cookie database from the previous session are
discarded. Session cookies created during this session are persisted to the
cookie database in a way that allows them to be restored.

\value WebContext.SessionCookieModeRestored
Session cookies that exist in the cookie database from the previous session are
restored, and session cookies created during this session are persisted to the
cookie database in a way that allows them to be restored. This is intended to be
used by applications to recover from an unclean shutdown or for a web browser to
provide session restore functionality.

Attempts to change this value after WebContext is constructed will have no
effect.

The default value is WebContext.SessionCookieModePersistent.
*/

OxideQQuickWebContext::SessionCookieMode
OxideQQuickWebContext::sessionCookieMode() const {
  Q_D(const OxideQQuickWebContext);

  Q_STATIC_ASSERT(
      SessionCookieModeEphemeral ==
      static_cast<SessionCookieMode>(
        WebContextProxy::SessionCookieModeEphemeral));
  Q_STATIC_ASSERT(
      SessionCookieModePersistent ==
      static_cast<SessionCookieMode>(
        WebContextProxy::SessionCookieModePersistent));
  Q_STATIC_ASSERT(
      SessionCookieModeRestored ==
      static_cast<SessionCookieMode>(
        WebContextProxy::SessionCookieModeRestored));
  return static_cast<SessionCookieMode>(d->proxy_->sessionCookieMode());
}

/*!
\internal
*/

void OxideQQuickWebContext::setSessionCookieMode(SessionCookieMode mode) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext::Cannot set sessionCookieMode once the context "
        "is in use";
    return;
  }

  WebContextProxy::SessionCookieMode m =
      static_cast<WebContextProxy::SessionCookieMode>(mode);

  if (mode == sessionCookieMode()) {
    return;
  }

  d->proxy_->setSessionCookieMode(m);

  emit sessionCookieModeChanged();
}

/*!
\qmlproperty bool WebContext::popupBlockerEnabled

Whether to prevent sites from creating new windows with \e{window.open()}
without being initiated by a user action. The default is true.
*/

bool OxideQQuickWebContext::popupBlockerEnabled() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->popupBlockerEnabled();
}

void OxideQQuickWebContext::setPopupBlockerEnabled(bool enabled) {
  Q_D(OxideQQuickWebContext);

  if (popupBlockerEnabled() == enabled) {
    return;
  }

  d->proxy_->setPopupBlockerEnabled(enabled);

  emit popupBlockerEnabledChanged();
}

/*!
\property OxideQQuickWebContext::networkRequestDelegate
\deprecated
*/

/*!
\qmlproperty WebContextDelegateWorker WebContext::networkRequestDelegate
\deprecated
*/

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::networkRequestDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->network_request_delegate_;
}

void OxideQQuickWebContext::setNetworkRequestDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->network_request_delegate_ == delegate) {
    return;
  }

  if (delegate && !d->prepareToAttachDelegateWorker(delegate)) {
    return;
  }

  QSharedPointer<webcontextdelegateworker::IOThreadController> io_delegate;
  if (delegate) {
    io_delegate = OxideQQuickWebContextDelegateWorkerPrivate::get(
        delegate)->io_thread_controller();
  }

  OxideQQuickWebContextDelegateWorker* old = d->network_request_delegate_;
  d->network_request_delegate_ = delegate;
  {
    QMutexLocker lock(&d->io_->lock);
    d->io_->network_request_delegate = io_delegate.toWeakRef();
  }

  d->detachedDelegateWorker(old);

  emit networkRequestDelegateChanged();
}

/*!
\property OxideQQuickWebContext::storageAccessPermissionDelegate
\deprecated
*/

/*!
\qmlproperty WebContextDelegateWorker WebContext::storageAccessPermissionDelegate
\deprecated
*/

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::storageAccessPermissionDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->unused_storage_access_permission_delegate_;
}

void OxideQQuickWebContext::setStorageAccessPermissionDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->unused_storage_access_permission_delegate_ == delegate) {
    return;
  }

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebContext: storageAccessPermissionDelegate is deprecated "
      "and no longer works";

  if (delegate && !d->prepareToAttachDelegateWorker(delegate)) {
    return;
  }

  OxideQQuickWebContextDelegateWorker* old =
      d->unused_storage_access_permission_delegate_;
  d->unused_storage_access_permission_delegate_ = delegate;

  d->detachedDelegateWorker(old);

  emit storageAccessPermissionDelegateChanged();
}

/*!
\property OxideQQuickWebContext::userAgentOverrideDelegate
\deprecated
*/

/*!
\qmlproperty WebContextDelegateWorker WebContext::userAgentOverrideDelegate
\deprecated
*/

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::userAgentOverrideDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->user_agent_override_delegate_;
}

void OxideQQuickWebContext::setUserAgentOverrideDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->user_agent_override_delegate_ == delegate) {
    return;
  }

  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickWebContext: userAgentOverrideDelegate is deprecated. "
      "Please use userAgentOverrides instead";

  if (delegate && !d->prepareToAttachDelegateWorker(delegate)) {
    return;
  }

  QSharedPointer<webcontextdelegateworker::IOThreadController> io_delegate;
  if (delegate) {
    io_delegate = OxideQQuickWebContextDelegateWorkerPrivate::get(
        delegate)->io_thread_controller();
  }

  OxideQQuickWebContextDelegateWorker* old = d->user_agent_override_delegate_;
  d->user_agent_override_delegate_ = delegate;
  {
    QMutexLocker lock(&d->io_->lock);
    d->io_->user_agent_override_delegate = io_delegate.toWeakRef();
  }

  d->detachedDelegateWorker(old);

  d->proxy_->setLegacyUserAgentOverrideEnabled(!!delegate);

  emit userAgentOverrideDelegateChanged();
}

/*!
\qmlproperty bool WebContext::devtoolsEnabled

Whether to enable remote debugging. The default is false.

\note Use this with caution, as this makes a remote debugging service available
on the specified network interface.
*/

bool OxideQQuickWebContext::devtoolsEnabled() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->devtoolsEnabled();
}

void OxideQQuickWebContext::setDevtoolsEnabled(bool enabled) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() == enabled) {
    return;
  }

  d->proxy_->setDevtoolsEnabled(enabled);

  emit devtoolsEnabledChanged();
}

/*!
\qmlproperty int WebContext::devtoolsPort

The port to run the remote debugging service on. This can be set to a value
between 1024 and 65535.

This can't be modified whilst devtoolsEnabled is true.

The default value is 8484.
*/

int OxideQQuickWebContext::devtoolsPort() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->devtoolsPort();
}

void OxideQQuickWebContext::setDevtoolsPort(int port) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() && d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set devtoolsPort whilst "
        "devtoolsEnabled is set to true";
    return;
  }

  if (devtoolsPort() == port) {
    return;
  }

  int min, max;
  WebContextProxy::getValidDevtoolsPorts(&min, &max);

  if (port < min || port > max) {
    qWarning() <<
        "OxideQQuickWebContext: devtoolsPort was set to an invalid value. "
        "It must be set between " << min << " and " << max;
    return;
  }

  d->proxy_->setDevtoolsPort(port);

  emit devtoolsPortChanged();
}

/*!
\qmlproperty string WebContext::devtoolsBindIp

The IP address of the network interface to run the remote debugging service on.
This must be set to a valid IPv4 or IPv6 string literal.

This can't be modified whilst devtoolsEnabled is true.

The default value is 127.0.0.1.
*/

QString OxideQQuickWebContext::devtoolsBindIp() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->devtoolsBindIp();
}

void OxideQQuickWebContext::setDevtoolsBindIp(const QString& bindIp) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() && d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set devtoolsBindIp whilst "
        "devtoolsEnabled is set to true";
    return;
  }

  if (devtoolsBindIp() == bindIp) {
    return;
  }

  if (!WebContextProxy::checkIPAddress(bindIp)) {
    qWarning() <<
        "OxideQQuickWebContext: devtoolsBindIp was set to an invalid value. "
        "It must be set to a valid IPv4 or IPv6 address";
    return;
  }

  d->proxy_->setDevtoolsBindIp(bindIp);

  emit devtoolsBindIpChanged();
}

/*!
\qmlproperty CookieManager WebContext::cookieManager
\since OxideQt 1.3
*/

OxideQQuickCookieManager*
OxideQQuickWebContext::cookieManager() const {
  Q_D(const OxideQQuickWebContext);

  if (!d->cookie_manager_) {
    OxideQQuickWebContext* web_context =
        const_cast<OxideQQuickWebContext*>(this);
    d->cookie_manager_ =
        new OxideQQuickCookieManager(web_context, web_context);
  }

  return d->cookie_manager_;
}

/*!
\qmlproperty list<string> WebContext::hostMappingRules
\since OxideQt 1.3

This provides a mechanism to allow applications to map host:port pairs to other
host:port pairs. Where a match exists, network connections to a host:port pair
will be automatically mapped to the replacement host:port pair.

This is mostly useful for automated testing.

Each entry in this list is a string with a format that can be one of:
\list
  \li MAP <host>[:<port>] <replacement_host>[:<replacement_port>]
  \li EXCLUDE <host>
\endlist

This can be set during construction only. Attempts to change this after
WebContext is constructed will have no effect.
*/

QStringList OxideQQuickWebContext::hostMappingRules() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->hostMappingRules();
}

/*!
\internal
*/

void OxideQQuickWebContext::setHostMappingRules(const QStringList& rules) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set hostMappingRules once the context "
        "is in use";
    return; 
  }

  if (rules == hostMappingRules()) {
    return;
  }

  d->proxy_->setHostMappingRules(rules);

  emit hostMappingRulesChanged();
}

/*!
\qmlproperty list<string> WebContext::allowedExtraUrlSchemes
\since OxideQt 1.3

Specifies a list of URL schemes for which requests will be delegated to Qt's
networking stack. Requests will be performed with the QNetworkAccessManager
provided by the QQmlEngine to which this WebContext belongs.

Some URL schemes will be ignored and never delegated to Qt's networking stack.
These are \e{about}, \e{blob}, \e{chrome}, \e{chrome-devtools}, \e{data},
\e{file}, \e{ftp}, \e{http}, \e{https}, \e{ws} and \e{wss}.
*/

QStringList OxideQQuickWebContext::allowedExtraUrlSchemes() const {
  Q_D(const OxideQQuickWebContext);

  return d->allowed_extra_url_schemes_;
}

void OxideQQuickWebContext::setAllowedExtraUrlSchemes(
    const QStringList& schemes) {
  Q_D(OxideQQuickWebContext);

  d->allowed_extra_url_schemes_ = schemes;
  d->proxy_->setAllowedExtraUrlSchemes(schemes);

  emit allowedExtraUrlSchemesChanged();
}

/*!
\qmlproperty int WebContext::maxCacheSizeHint
\since OxideQt 1.6

Specify a soft upper limit for the size of the network cache in MB. This can
only be set during construction. Attempts to change it after WebContext is
constructed will be ignored.

By default this is 0, which means that Oxide will determine an appropriate value
automatically.
*/

int OxideQQuickWebContext::maxCacheSizeHint() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->maxCacheSizeHint();
}

/*!
\internal
*/

void OxideQQuickWebContext::setMaxCacheSizeHint(int size) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy_->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set maxCacheSizeHint once the context "
        "is in use";
    return;
  }

  if (size < 0) {
    qWarning() <<
        "OxideQQuickWebContext: maxCacheSizeHint cannot have a negative value";
    return;
  }

  static int upper_limit = std::numeric_limits<int>::max() / (1024 * 1024);
  if (size > upper_limit) {
    // To avoid integer overflow.
    qWarning() << "OxideQQuickWebContext: maxCacheSizeHint cannot exceed "
               << upper_limit << "MB";
    return;
  }

  if (maxCacheSizeHint() == size) {
    return;
  }

  d->proxy_->setMaxCacheSizeHint(size);
  emit maxCacheSizeHintChanged();
}

/*!
\qmlproperty string WebContext::defaultAudioCaptureDeviceId
\since OxideQt 1.9

Allow the application to specify the default audio capture device used when
handling calls to \e{MediaDevices.getUserMedia()}. This must be set to the ID of
a currently detected audio capture device.

By default this is empty, which means that Oxide will use the first audio
capture device returned by Oxide::availableAudioCaptureDevices.
*/

QString OxideQQuickWebContext::defaultAudioCaptureDeviceId() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->defaultAudioCaptureDeviceId();
}

void OxideQQuickWebContext::setDefaultAudioCaptureDeviceId(const QString& id) {
  Q_D(OxideQQuickWebContext);

  if (defaultAudioCaptureDeviceId() == id) {
    return;
  }

  if (!d->proxy_->setDefaultAudioCaptureDeviceId(id)) {
    qWarning() <<
        "OxideQQuickWebContext: Invalid defaultAudioCaptureDeviceId \"" <<
        id << "\"";
  }

  // Oxide loops back in to us to emit the signal, as the default will clear
  // if the actual device is removed
}

/*!
\qmlproperty string WebContext::defaultVideoCaptureDeviceId
\since OxideQt 1.9

Allow the application to specify the default video capture device used when
handling calls to \e{MediaDevices.getUserMedia()}. This must be set to the ID of
a currently detected video capture device.

By default this is empty, which means that Oxide will use the first front-facing
video capture device returned by Oxide::availableAudioCaptureDevices (or the
first device if no front-facing device is available).
*/

QString OxideQQuickWebContext::defaultVideoCaptureDeviceId() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->defaultVideoCaptureDeviceId();
}

void OxideQQuickWebContext::setDefaultVideoCaptureDeviceId(const QString& id) {
  Q_D(OxideQQuickWebContext);

  if (defaultVideoCaptureDeviceId() == id) {
    return;
  }

  if (!d->proxy_->setDefaultVideoCaptureDeviceId(id)) {
    qWarning() <<
        "OxideQQuickWebContext: Invalid defaultVideoCaptureDeviceId \"" <<
        id << "\"";
  }

  // Oxide loops back in to us to emit the signal, as the default will clear
  // if the actual device is removed
}

/*!
\qmlproperty list<variant> WebContext::userAgentOverrides
\since OxideQt 1.9

Allows the application to specify a list of per-URL overrides for the user-agent
string.

The format is a list of variants. Each variant is a list of 2 strings. The
first string is a regular expression used to match URLs. The second string is
the override user-agent string to use in the case that the regular expression
is a match for a URL.

Behaviour is unspecified for URLs that match more than one entry.
*/

QVariantList OxideQQuickWebContext::userAgentOverrides() const {
  Q_D(const OxideQQuickWebContext);

  QVariantList rv;
  QList<WebContextProxy::UserAgentOverride> overrides =
      d->proxy_->userAgentOverrides();

  for (auto it = overrides.begin(); it != overrides.end(); ++it) {
    rv.append(FromUserAgentOverride(*it));
  }

  return rv;
}

void OxideQQuickWebContext::setUserAgentOverrides(
    const QVariantList& overrides) {
  Q_D(OxideQQuickWebContext);

  QList<WebContextProxy::UserAgentOverride> entries;

  for (auto it = overrides.begin(); it != overrides.end(); ++it) {
    bool valid = false;
    WebContextProxy::UserAgentOverride entry = ToUserAgentOverride(*it, &valid);

    if (!valid) {
      qWarning() <<
          "OxideQQuickWebContext::userAgentOverride: Each entry must be a "
          "list of size 2, with the first item being a valid regular "
          "expression for URL matching and the second item being the user "
          "agent string";
      return;
    }

    entries.append(entry);
  }

  d->proxy_->setUserAgentOverrides(entries);

  emit userAgentOverridesChanged();
}

/*!
\qmlproperty bool WebContext::doNotTrack
\since OxideQt 1.9

Whether to enable do-not-track. The default is false.

Setting this to true will result in the \e{DNT} header being added and set to
\e{1} in outgoing HTTP requests. It will also result in \e{navigator.doNotTrack}
indicating that do-not-track is enabled.

When set to false, Oxide behaves as if no preference has been specified (the
\e{DNT} header is omitted from HTTP requests and \e{navigator.doNotTrack} is an
undefined value.
*/

bool OxideQQuickWebContext::doNotTrack() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy_->doNotTrack();
}

void OxideQQuickWebContext::setDoNotTrack(bool dnt) {
  Q_D(OxideQQuickWebContext);

  if (doNotTrack() == dnt) {
    return;
  }

  d->proxy_->setDoNotTrack(dnt);

  emit doNotTrackEnabledChanged();
}

#include "moc_oxideqquickwebcontext.cpp"
