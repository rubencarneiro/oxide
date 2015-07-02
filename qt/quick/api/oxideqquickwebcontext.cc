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

#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"

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

#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"
#include "qt/quick/oxide_qquick_init.h"

#include "oxideqquickcookiemanager_p.h"
#include "oxideqquickuserscript_p.h"
#include "oxideqquickuserscript_p_p.h"
#include "oxideqquickwebcontextdelegateworker_p.h"
#include "oxideqquickwebcontextdelegateworker_p_p.h"
#include "oxidequseragentoverriderequest_p.h"
#include "oxidequseragentoverriderequest_p_p.h"

using oxide::qt::WebContextProxy;

namespace {

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
  void HandleStoragePermissionRequest(
      OxideQStoragePermissionRequest* req) override;
  QString GetUserAgentOverride(const QUrl& url) override;

  QMutex lock;

  QWeakPointer<IOThreadController> network_request_delegate;
  QWeakPointer<IOThreadController> storage_access_permission_delegate;
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

void WebContextIODelegate::HandleStoragePermissionRequest(
    OxideQStoragePermissionRequest* req) {
  QSharedPointer<IOThreadController> delegate;
  {
    QMutexLocker locker(&lock);
    delegate = storage_access_permission_delegate.toStrongRef();
  }
  if (!delegate) {
    return;
  }

  delegate->CallEntryPointInWorker("onStoragePermissionRequest", req);
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

OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(OxideQQuickWebContext,
                                    oxide::qt::WebContextProxyHandle);

OxideQQuickWebContextPrivate::OxideQQuickWebContextPrivate(
    OxideQQuickWebContext* q)
    : oxide::qt::WebContextProxyHandle(WebContextProxy::create(this), q),
      constructed_(false),
      io_(new oxide::qquick::WebContextIODelegate()),
      network_request_delegate_(nullptr),
      storage_access_permission_delegate_(nullptr),
      user_agent_override_delegate_(nullptr),
      cookie_manager_(nullptr) {}

void OxideQQuickWebContextPrivate::userScriptUpdated() {
  proxy()->updateUserScripts();
}

void OxideQQuickWebContextPrivate::userScriptWillBeDeleted() {
  Q_Q(OxideQQuickWebContext);

  OxideQQuickUserScriptPrivate* sender =
      qobject_cast<OxideQQuickUserScriptPrivate *>(q->sender());
  Q_ASSERT(sender);
  q->removeUserScript(OxideQQuickUserScriptPrivate::fromProxyHandle(sender));
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
        static_cast<OxideQQuickWebContext*>(prop->object))->proxy();

  return p->userScripts().size();
}

OxideQQuickUserScript* OxideQQuickWebContextPrivate::userScript_at(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    int index) {
  WebContextProxy* p =
      OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext*>(prop->object))->proxy();

  if (index >= p->userScripts().size()) {
    return nullptr;
  }

  return OxideQQuickUserScriptPrivate::fromProxyHandle(
      p->userScripts().at(index));
}

void OxideQQuickWebContextPrivate::userScript_clear(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);
  WebContextProxy* p =
      OxideQQuickWebContextPrivate::get(context)->proxy();

  while (p->userScripts().size() > 0) {
    context->removeUserScript(
        OxideQQuickUserScriptPrivate::fromProxyHandle(p->userScripts().at(0)));
  }
}

bool OxideQQuickWebContextPrivate::prepareToAttachDelegateWorker(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_Q(OxideQQuickWebContext);

  OxideQQuickWebContext* parent =
      qobject_cast<OxideQQuickWebContext *>(delegate->parent());
  if (parent && parent != q) {
    qWarning() << "Can't add WebContextDelegateWorker to more than one WebContext";
    return false;
  }

  delegate->setParent(q);

  OxideQQuickWebContextDelegateWorkerPrivate* p =
      OxideQQuickWebContextDelegateWorkerPrivate::get(delegate);
  p->incAttachedCount();

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
  if (!p->decAttachedCount()) {
    return;
  }

  // The delegate is not attached to any more slots on this context.
  // If it's not already being deleted and we own it, delete it now
  if (!p->in_destruction() && delegate->parent() == q) {
    delete delegate;
  }
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

void OxideQQuickWebContextPrivate::DestroyDefault() {
  Q_Q(OxideQQuickWebContext);

  Q_ASSERT(q == OxideQQuickWebContext::defaultContext(false));
  delete q;
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
  proxy()->clearTemporarySavedPermissionStatuses();
}

bool OxideQQuickWebContextPrivate::isInitialized() const {
  return proxy()->isInitialized();
}

int OxideQQuickWebContextPrivate::setCookies(
    const QUrl& url,
    const QList<QNetworkCookie>& cookies) {
  return proxy()->setCookies(url, cookies);
}

int OxideQQuickWebContextPrivate::getCookies(const QUrl& url) {
  return proxy()->getCookies(url);
}

int OxideQQuickWebContextPrivate::getAllCookies() {
  return proxy()->getAllCookies();
}

int OxideQQuickWebContextPrivate::deleteAllCookies() {
  return proxy()->deleteAllCookies();
}

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent) :
    QObject(parent) {
  oxide::qquick::EnsureChromiumStarted();
  d_ptr.reset(new OxideQQuickWebContextPrivate(this));

  Q_D(OxideQQuickWebContext);

  QSharedPointer<oxide::qt::WebContextProxyClient::IOClient> io =
      qSharedPointerCast<oxide::qt::WebContextProxyClient::IOClient>(d->io_);
  d->proxy()->init(io.toWeakRef());
}

OxideQQuickWebContext::~OxideQQuickWebContext() {
  Q_D(OxideQQuickWebContext);

  for (int i = 0; i < d->proxy()->userScripts().size(); ++i) {
    d->detachUserScriptSignals(
        OxideQQuickUserScriptPrivate::fromProxyHandle(
          d->proxy()->userScripts().at(i)));
  }

  // These call back in to us when destroyed, so delete them now in order
  // to avoid a reentrancy crash
  delete d->network_request_delegate_;
  delete d->storage_access_permission_delegate_;
  delete d->user_agent_override_delegate_;
}

void OxideQQuickWebContext::classBegin() {}

void OxideQQuickWebContext::componentComplete() {
  Q_D(OxideQQuickWebContext);

  d->constructed_ = true;
  emit d->constructed();
}

// static
OxideQQuickWebContext* OxideQQuickWebContext::defaultContext(bool create) {
  oxide::qt::WebContextProxyHandle* h = WebContextProxy::defaultContext();
  if (h) {
    return OxideQQuickWebContextPrivate::fromProxyHandle(h);
  }

  if (!create) {
    return nullptr;
  }

  OxideQQuickWebContext* c = new OxideQQuickWebContext();
  c->componentComplete();
  QQmlEngine::setObjectOwnership(c, QQmlEngine::CppOwnership);

  OxideQQuickWebContextPrivate::get(c)->proxy()->makeDefault();
  Q_ASSERT(WebContextProxy::defaultContext());

  return c;
}

QString OxideQQuickWebContext::product() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->product();
}

void OxideQQuickWebContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->product() == product) {
    return;
  }

  QString old_user_agent = userAgent();

  d->proxy()->setProduct(product);
  emit productChanged();

  if (userAgent() != old_user_agent) {
    emit userAgentChanged();
  }
}

QString OxideQQuickWebContext::userAgent() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->userAgent();
}

void OxideQQuickWebContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->userAgent() == user_agent) {
    return;
  }

  d->proxy()->setUserAgent(user_agent);
  emit userAgentChanged();
}

QUrl OxideQQuickWebContext::dataPath() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->dataPath();
}

void OxideQQuickWebContext::setDataPath(const QUrl& data_url) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set dataPath once the context is in use";
    return;
  }

  if (dataPath() == data_url) {
    return;
  }

  if (!data_url.isLocalFile() && !data_url.isEmpty()) {
    qWarning() << "OxideQQuickWebContext: dataPath only supports local files";
    return;
  }

  d->proxy()->setDataPath(data_url);
  emit dataPathChanged();
}

QUrl OxideQQuickWebContext::cachePath() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->cachePath();
}

void OxideQQuickWebContext::setCachePath(const QUrl& cache_url) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->isInitialized()) {
    qWarning() << "OxideQQuickWebContext:: Cannot set cachePath once the context is in use";
    return;
  }

  if (cachePath() == cache_url) {
    return;
  }

  if (!cache_url.isLocalFile() && !cache_url.isEmpty()) {
    qWarning() << "OxideQQuickWebContext: cachePath only supports local files";
    return;
  }

  d->proxy()->setCachePath(cache_url);
  emit cachePathChanged();
}

QString OxideQQuickWebContext::acceptLangs() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->acceptLangs();
}

void OxideQQuickWebContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebContext);

  if (acceptLangs() == accept_langs) {
    return;
  }

  d->proxy()->setAcceptLangs(accept_langs);
  emit acceptLangsChanged();
}

QQmlListProperty<OxideQQuickUserScript>
OxideQQuickWebContext::userScripts() {
  return QQmlListProperty<OxideQQuickUserScript>(
      this, nullptr,
      OxideQQuickWebContextPrivate::userScript_append,
      OxideQQuickWebContextPrivate::userScript_count,
      OxideQQuickWebContextPrivate::userScript_at,
      OxideQQuickWebContextPrivate::userScript_clear);
}

void OxideQQuickWebContext::addUserScript(OxideQQuickUserScript* user_script) {
  Q_D(OxideQQuickWebContext);

  if (!user_script) {
    qWarning() << "Must specify a user script";
    return;
  }

  OxideQQuickUserScriptPrivate* ud =
      OxideQQuickUserScriptPrivate::get(user_script);

  if (!d->proxy()->userScripts().contains(ud)) {
    connect(user_script, SIGNAL(scriptLoaded()),
            this, SLOT(userScriptUpdated()));
    connect(user_script, SIGNAL(scriptPropertyChanged()),
            this, SLOT(userScriptUpdated()));
    connect(ud, SIGNAL(willBeDeleted()),
            this, SLOT(userScriptWillBeDeleted()));
  } else {
    d->proxy()->userScripts().removeOne(ud);
  }

  if (!user_script->parent()) {
    user_script->setParent(this);
  }
  d->proxy()->userScripts().append(ud);

  emit userScriptsChanged();
}

void OxideQQuickWebContext::removeUserScript(
    OxideQQuickUserScript* user_script) {
  Q_D(OxideQQuickWebContext);

  if (!user_script) {
    qWarning() << "Must specify a user script";
    return;
  }

  OxideQQuickUserScriptPrivate* ud =
      OxideQQuickUserScriptPrivate::get(user_script);

  if (!d->proxy()->userScripts().contains(ud)) {
    return;
  }

  d->detachUserScriptSignals(user_script);
  if (user_script->parent() == this) {
    user_script->setParent(nullptr);
  }

  d->proxy()->userScripts().removeOne(ud);

  emit userScriptsChanged();
}

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

  return static_cast<CookiePolicy>(d->proxy()->cookiePolicy());
}

void OxideQQuickWebContext::setCookiePolicy(CookiePolicy policy) {
  Q_D(OxideQQuickWebContext);

  WebContextProxy::CookiePolicy p =
      static_cast<WebContextProxy::CookiePolicy>(policy);

  if (policy == cookiePolicy()) {
    return;
  }

  d->proxy()->setCookiePolicy(p);

  emit cookiePolicyChanged();
}

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
  return static_cast<SessionCookieMode>(d->proxy()->sessionCookieMode());
}

void OxideQQuickWebContext::setSessionCookieMode(SessionCookieMode mode) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->isInitialized()) {
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

  d->proxy()->setSessionCookieMode(m);

  emit sessionCookieModeChanged();
}

bool OxideQQuickWebContext::popupBlockerEnabled() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->popupBlockerEnabled();
}

void OxideQQuickWebContext::setPopupBlockerEnabled(bool enabled) {
  Q_D(OxideQQuickWebContext);

  if (popupBlockerEnabled() == enabled) {
    return;
  }

  d->proxy()->setPopupBlockerEnabled(enabled);

  emit popupBlockerEnabledChanged();
}

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

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::storageAccessPermissionDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->storage_access_permission_delegate_;
}

void OxideQQuickWebContext::setStorageAccessPermissionDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->storage_access_permission_delegate_ == delegate) {
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

  OxideQQuickWebContextDelegateWorker* old = d->storage_access_permission_delegate_;
  d->storage_access_permission_delegate_ = delegate;
  {
    QMutexLocker lock(&d->io_->lock);
    d->io_->storage_access_permission_delegate = io_delegate.toWeakRef();
  }

  d->detachedDelegateWorker(old);

  emit storageAccessPermissionDelegateChanged();
}

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

  if (delegate) {
    qWarning() <<
        "OxideQQuickWebContext: userAgentOverrideDelegate is deprecated. "
        "Please consider switching to userAgentOverrides instead";
  }

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

  d->proxy()->setLegacyUserAgentOverrideEnabled(!!delegate);

  emit userAgentOverrideDelegateChanged();
}

bool OxideQQuickWebContext::devtoolsEnabled() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->devtoolsEnabled();
}

void OxideQQuickWebContext::setDevtoolsEnabled(bool enabled) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() == enabled) {
    return;
  }

  d->proxy()->setDevtoolsEnabled(enabled);

  emit devtoolsEnabledChanged();
}

int OxideQQuickWebContext::devtoolsPort() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->devtoolsPort();
}

void OxideQQuickWebContext::setDevtoolsPort(int port) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() && d->proxy()->isInitialized()) {
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

  d->proxy()->setDevtoolsPort(port);

  emit devtoolsPortChanged();
}

QString OxideQQuickWebContext::devtoolsBindIp() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->devtoolsBindIp();
}

void OxideQQuickWebContext::setDevtoolsBindIp(const QString& bindIp) {
  Q_D(OxideQQuickWebContext);

  if (devtoolsEnabled() && d->proxy()->isInitialized()) {
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

  d->proxy()->setDevtoolsBindIp(bindIp);

  emit devtoolsBindIpChanged();
}

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

QStringList OxideQQuickWebContext::hostMappingRules() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->hostMappingRules();
}

void OxideQQuickWebContext::setHostMappingRules(const QStringList& rules) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->isInitialized()) {
    qWarning() <<
        "OxideQQuickWebContext: Cannot set hostMappingRules once the context "
        "is in use";
    return; 
  }

  if (rules == hostMappingRules()) {
    return;
  }

  d->proxy()->setHostMappingRules(rules);

  emit hostMappingRulesChanged();
}

QStringList OxideQQuickWebContext::allowedExtraUrlSchemes() const {
  Q_D(const OxideQQuickWebContext);

  return d->allowed_extra_url_schemes_;
}

void OxideQQuickWebContext::setAllowedExtraUrlSchemes(
    const QStringList& schemes) {
  Q_D(OxideQQuickWebContext);

  d->allowed_extra_url_schemes_ = schemes;
  d->proxy()->setAllowedExtraUrlSchemes(schemes);

  emit allowedExtraUrlSchemesChanged();
}

int OxideQQuickWebContext::maxCacheSizeHint() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->maxCacheSizeHint();
}

void OxideQQuickWebContext::setMaxCacheSizeHint(int size) {
  Q_D(OxideQQuickWebContext);

  if (d->proxy()->isInitialized()) {
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

  d->proxy()->setMaxCacheSizeHint(size);
  emit maxCacheSizeHintChanged();
}

QString OxideQQuickWebContext::defaultAudioCaptureDeviceId() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->defaultAudioCaptureDeviceId();
}

void OxideQQuickWebContext::setDefaultAudioCaptureDeviceId(const QString& id) {
  Q_D(OxideQQuickWebContext);

  if (defaultAudioCaptureDeviceId() == id) {
    return;
  }

  if (!d->proxy()->setDefaultAudioCaptureDeviceId(id)) {
    qWarning() <<
        "OxideQQuickWebContext: Invalid defaultAudioCaptureDeviceId \"" <<
        id << "\"";
  }

  // Oxide loops back in to us to emit the signal, as the default will clear
  // if the actual device is removed
}

QString OxideQQuickWebContext::defaultVideoCaptureDeviceId() const {
  Q_D(const OxideQQuickWebContext);

  return d->proxy()->defaultVideoCaptureDeviceId();
}

void OxideQQuickWebContext::setDefaultVideoCaptureDeviceId(const QString& id) {
  Q_D(OxideQQuickWebContext);

  if (defaultVideoCaptureDeviceId() == id) {
    return;
  }

  if (!d->proxy()->setDefaultVideoCaptureDeviceId(id)) {
    qWarning() <<
        "OxideQQuickWebContext: Invalid defaultVideoCaptureDeviceId \"" <<
        id << "\"";
  }

  // Oxide loops back in to us to emit the signal, as the default will clear
  // if the actual device is removed
}

QVariantList OxideQQuickWebContext::userAgentOverrides() const {
  Q_D(const OxideQQuickWebContext);

  QVariantList rv;
  QList<WebContextProxy::UserAgentOverride> overrides =
      d->proxy()->userAgentOverrides();

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

  d->proxy()->setUserAgentOverrides(entries);

  emit userAgentOverridesChanged();
}

#include "moc_oxideqquickwebcontext_p.cpp"
