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

#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"

#include <QMutex>
#include <QMutexLocker>
#include <QQmlListProperty>
#include <QSharedPointer>
#include <QtDebug>
#include <QThread>
#include <QtQuickVersion>
#include <QWeakPointer>
#if defined(ENABLE_COMPOSITING)
#include <QtQuick/private/qsgcontext_p.h>
#endif

#include "qt/core/api/oxideqnetworkcallbackevents.h"
#include "qt/core/api/oxideqstoragepermissionrequest.h"

#include "oxideqquickglobals_p.h"
#include "oxideqquickglobals_p_p.h"
#include "oxideqquickuserscript_p.h"
#include "oxideqquickuserscript_p_p.h"
#include "oxideqquickwebcontextdelegateworker_p.h"
#include "oxideqquickwebcontextdelegateworker_p_p.h"
#include "oxidequseragentoverriderequest_p.h"
#include "oxidequseragentoverriderequest_p_p.h"

namespace {
QWeakPointer<OxideQQuickWebContext> g_default_context;
}

namespace oxide {
namespace qquick {

class WebContextIOThreadDelegate :
    public oxide::qt::WebContextAdapter::IOThreadDelegate {
 public:
  WebContextIOThreadDelegate() :
      network_request_delegate(NULL),
      storage_access_permission_delegate(NULL),
      user_agent_override_delegate(NULL) {}
  virtual ~WebContextIOThreadDelegate() {}

  virtual void OnBeforeURLRequest(OxideQBeforeURLRequestEvent* event) {
    QMutexLocker locker(&lock);
    if (!network_request_delegate) {
      delete event;
      return;
    }

    // FIXME(chrisccoulson): Should move |event| to the helper thread,
    //  where it will be consumed

    emit network_request_delegate->callEntryPointInWorker("onBeforeURLRequest",
                                                          event);
  }

  virtual void OnBeforeSendHeaders(OxideQBeforeSendHeadersEvent* event) {
    QMutexLocker locker(&lock);
    if (!network_request_delegate) {
      delete event;
      return;
    }

    // FIXME(chrisccoulson): Should move |event| to the helper thread,
    //  where it will be consumed

    emit network_request_delegate->callEntryPointInWorker("onBeforeSendHeaders",
                                                          event);
  }

  virtual void HandleStoragePermissionRequest(
      OxideQStoragePermissionRequest* req) {
    QMutexLocker locker(&lock);
    if (!storage_access_permission_delegate) {
      delete req;
      return;
    }

    // FIXME(chrisccoulson): Should move |req| to the helper thread,
    //  where it will be consumed

    emit storage_access_permission_delegate->callEntryPointInWorker(
        "onStoragePermissionRequest", req);
  }

  virtual bool GetUserAgentOverride(const QUrl& url, QString* user_agent) {
    bool did_override = false;

    QMutexLocker locker(&lock);
    if (!user_agent_override_delegate) {
      return did_override;
    }

    OxideQUserAgentOverrideRequest* req = new OxideQUserAgentOverrideRequest(url);

    OxideQUserAgentOverrideRequestPrivate* p =
        OxideQUserAgentOverrideRequestPrivate::get(req);
    p->did_override = &did_override;
    p->user_agent = user_agent;

    emit user_agent_override_delegate->callEntryPointInWorker(
        "onGetUserAgentOverride", req);

    return did_override;
  }

  QMutex lock;

  WebContextDelegateWorkerIOThreadController* network_request_delegate;
  WebContextDelegateWorkerIOThreadController* storage_access_permission_delegate;
  WebContextDelegateWorkerIOThreadController* user_agent_override_delegate;
};

} // namespace qquick
} // namespace oxide

OxideQQuickWebContextPrivate::OxideQQuickWebContextPrivate(
    OxideQQuickWebContext* q) :
    oxide::qt::WebContextAdapter(q, new oxide::qquick::WebContextIOThreadDelegate()),
    io_thread_delegate_(
        static_cast<oxide::qquick::WebContextIOThreadDelegate *>(getIOThreadDelegate())),
    network_request_delegate_(NULL),
    storage_access_permission_delegate_(NULL),
    user_agent_override_delegate_(NULL) {}

void OxideQQuickWebContextPrivate::userScriptUpdated() {
  updateUserScripts();
}

void OxideQQuickWebContextPrivate::userScriptWillBeDeleted() {
  Q_Q(OxideQQuickWebContext);

  OxideQQuickUserScriptPrivate* sender =
      qobject_cast<OxideQQuickUserScriptPrivate *>(q->sender());
  Q_ASSERT(sender);
  q->removeUserScript(adapterToQObject<OxideQQuickUserScript>(sender));  
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
  OxideQQuickWebContextPrivate* cd = OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext *>(prop->object));

  return cd->user_scripts().size();
}

OxideQQuickUserScript* OxideQQuickWebContextPrivate::userScript_at(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    int index) {
  OxideQQuickWebContextPrivate* cd = OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext *>(prop->object));

  if (index >= cd->user_scripts().size()) {
    return NULL;
  }

  return adapterToQObject<OxideQQuickUserScript>(cd->user_scripts().at(index));
}

void OxideQQuickWebContextPrivate::userScript_clear(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);
  OxideQQuickWebContextPrivate* cd = OxideQQuickWebContextPrivate::get(context);

  while (cd->user_scripts().size() > 0) {
    context->removeUserScript(
        adapterToQObject<OxideQQuickUserScript>(cd->user_scripts().at(0)));
  }
}

bool OxideQQuickWebContextPrivate::attachDelegateWorker(
    OxideQQuickWebContextDelegateWorker* worker,
    OxideQQuickWebContextDelegateWorker** ui_slot,
    oxide::qquick::WebContextDelegateWorkerIOThreadController** io_slot) {
  Q_Q(OxideQQuickWebContext);

  if (*ui_slot == worker) {
    return false;
  }

  oxide::qquick::WebContextDelegateWorkerIOThreadController* controller = NULL;

  if (worker) {
    OxideQQuickWebContext* parent =
        qobject_cast<OxideQQuickWebContext *>(worker->parent());
    if (parent && parent != q) {
      qWarning() << "Can't add WebContextDelegateWorker to more than one WebContext";
      return false;
    }

    worker->setParent(q);
    controller = OxideQQuickWebContextDelegateWorkerPrivate::get(
        worker)->io_thread_controller.data();
  }

  OxideQQuickWebContextDelegateWorker* old_worker = *ui_slot;
  *ui_slot = worker;

  {
    QMutexLocker lock(&io_thread_delegate_->lock);
    *io_slot = controller;
  }

  if (old_worker &&
      old_worker != q->networkRequestDelegate() &&
      old_worker != q->storageAccessPermissionDelegate() &&
      old_worker != q->userAgentOverrideDelegate()) {
    old_worker->setParent(NULL);
  }

  return true;
}

OxideQQuickWebContextPrivate::~OxideQQuickWebContextPrivate() {}

void OxideQQuickWebContextPrivate::delegateWorkerDestroyed(
    OxideQQuickWebContextDelegateWorker* worker) {
  Q_Q(OxideQQuickWebContext);

  if (worker == q->networkRequestDelegate()) {
    q->setNetworkRequestDelegate(NULL);
  }
  if (worker == q->storageAccessPermissionDelegate()) {
    q->setStorageAccessPermissionDelegate(NULL);
  }
  if (worker == q->userAgentOverrideDelegate()) {
    q->setUserAgentOverrideDelegate(NULL);
  }
}

OxideQQuickWebContextPrivate* OxideQQuickWebContextPrivate::get(
    OxideQQuickWebContext* context) {
  return context->d_func();
}

// static
void OxideQQuickWebContextPrivate::ensureChromiumStarted() {
  static bool started = false;
  if (started) {
    return;
  }
  started = true;
#if defined(ENABLE_COMPOSITING)
  oxide::qt::WebContextAdapter::setSharedGLContext(
      QSGContext::sharedOpenGLContext());
#endif
  oxide::qt::WebContextAdapter::ensureChromiumStarted();
}

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebContextPrivate(this)) {
  OxideQQuickWebContextPrivate::ensureChromiumStarted();
}

OxideQQuickWebContext::~OxideQQuickWebContext() {
  Q_D(OxideQQuickWebContext);

  emit d->willBeDestroyed();

  for (int i = 0; i < d->user_scripts().size(); ++i) {
    d->detachUserScriptSignals(
        adapterToQObject<OxideQQuickUserScript>(d->user_scripts().at(i)));
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

  d->init();
  emit d->initialized();
}

// static
QSharedPointer<OxideQQuickWebContext> OxideQQuickWebContext::defaultContext() {
  if (g_default_context) {
    return g_default_context;
  }

  QSharedPointer<OxideQQuickWebContext> new_context(
      new OxideQQuickWebContext());

  new_context->setProduct(OxideQQuickGlobals::instance()->product());
  new_context->setUserAgent(OxideQQuickGlobals::instance()->userAgent());
  new_context->setDataPath(OxideQQuickGlobals::instance()->dataPath());
  new_context->setCachePath(OxideQQuickGlobals::instance()->cachePath());
  new_context->setAcceptLangs(OxideQQuickGlobals::instance()->acceptLangs());

  new_context->componentComplete();
  g_default_context = new_context;

  OxideQQuickGlobalsPrivate::get(
      OxideQQuickGlobals::instance())->defaultContextCreated();

  return new_context;
}

QString OxideQQuickWebContext::product() const {
  Q_D(const OxideQQuickWebContext);

  return d->product();
}

void OxideQQuickWebContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebContext);

  if (d->product() == product) {
    return;
  }

  QString old_user_agent = userAgent();

  d->setProduct(product);
  emit productChanged();

  if (userAgent() != old_user_agent) {
    emit userAgentChanged();
  }
}

QString OxideQQuickWebContext::userAgent() const {
  Q_D(const OxideQQuickWebContext);

  return d->userAgent();
}

void OxideQQuickWebContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebContext);

  if (d->userAgent() == user_agent) {
    return;
  }

  d->setUserAgent(user_agent);
  emit userAgentChanged();
}

QUrl OxideQQuickWebContext::dataPath() const {
  Q_D(const OxideQQuickWebContext);

  return d->dataPath();
}

void OxideQQuickWebContext::setDataPath(const QUrl& data_url) {
  Q_D(OxideQQuickWebContext);

  if (d->isInitialized()) {
    qWarning() << "Can only set dataPath during construction";
    return;
  }

  if (d->dataPath() == data_url) {
    return;
  }

  d->setDataPath(data_url);
  emit dataPathChanged();
}

QUrl OxideQQuickWebContext::cachePath() const {
  Q_D(const OxideQQuickWebContext);

  return d->cachePath();
}

void OxideQQuickWebContext::setCachePath(const QUrl& cache_url) {
  Q_D(OxideQQuickWebContext);

  if (d->isInitialized()) {
    qWarning() << "Can only set cachePath during construction";
    return;
  }

  if (d->cachePath() == cache_url) {
    return;
  }

  d->setCachePath(cache_url);
  emit cachePathChanged();
}

QString OxideQQuickWebContext::acceptLangs() const {
  Q_D(const OxideQQuickWebContext);

  return d->acceptLangs();
}

void OxideQQuickWebContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebContext);

  if (d->acceptLangs() == accept_langs) {
    return;
  }

  d->setAcceptLangs(accept_langs);
  emit acceptLangsChanged();
}

QQmlListProperty<OxideQQuickUserScript>
OxideQQuickWebContext::userScripts() {
  return QQmlListProperty<OxideQQuickUserScript>(
      this, NULL,
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

  if (!d->user_scripts().contains(ud)) {
    connect(user_script, SIGNAL(scriptLoaded()),
            this, SLOT(userScriptUpdated()));
    connect(user_script, SIGNAL(scriptPropertyChanged()),
            this, SLOT(userScriptUpdated()));
    connect(ud, SIGNAL(willBeDeleted()),
            this, SLOT(userScriptWillBeDeleted()));
  } else {
    d->user_scripts().removeOne(ud);
  }

  if (!user_script->parent()) {
    user_script->setParent(this);
  }
  d->user_scripts().append(ud);

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

  if (!d->user_scripts().contains(ud)) {
    return;
  }

  d->detachUserScriptSignals(user_script);
  if (user_script->parent() == this) {
    user_script->setParent(NULL);
  }

  d->user_scripts().removeOne(ud);

  emit userScriptsChanged();
}

OxideQQuickWebContext::CookiePolicy OxideQQuickWebContext::cookiePolicy() const {
  Q_D(const OxideQQuickWebContext);

  Q_STATIC_ASSERT(
      CookiePolicyAllowAll ==
      static_cast<CookiePolicy>(
        oxide::qt::WebContextAdapter::CookiePolicyAllowAll));
  Q_STATIC_ASSERT(
      CookiePolicyBlockAll ==
      static_cast<CookiePolicy>(
        oxide::qt::WebContextAdapter::CookiePolicyBlockAll));
  Q_STATIC_ASSERT(
      CookiePolicyBlockThirdParty ==
      static_cast<CookiePolicy>(
        oxide::qt::WebContextAdapter::CookiePolicyBlockThirdParty));

  return static_cast<CookiePolicy>(d->cookiePolicy());
}

void OxideQQuickWebContext::setCookiePolicy(CookiePolicy policy) {
  Q_D(OxideQQuickWebContext);

  oxide::qt::WebContextAdapter::CookiePolicy p =
      static_cast<oxide::qt::WebContextAdapter::CookiePolicy>(policy);

  if (p == d->cookiePolicy()) {
    return;
  }

  d->setCookiePolicy(p);

  emit cookiePolicyChanged();
}

bool OxideQQuickWebContext::popupBlockerEnabled() const {
  Q_D(const OxideQQuickWebContext);

  return d->popupBlockerEnabled();
}

void OxideQQuickWebContext::setPopupBlockerEnabled(bool enabled) {
  Q_D(OxideQQuickWebContext);

  if (d->popupBlockerEnabled() == enabled) {
    return;
  }

  d->setPopupBlockerEnabled(enabled);

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

  if (d->attachDelegateWorker(
      delegate,
      &d->network_request_delegate_,
      &d->io_thread_delegate_->network_request_delegate)) {
    emit networkRequestDelegateChanged();
  }
}

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::storageAccessPermissionDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->storage_access_permission_delegate_;
}

void OxideQQuickWebContext::setStorageAccessPermissionDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->attachDelegateWorker(
      delegate,
      &d->storage_access_permission_delegate_,
      &d->io_thread_delegate_->storage_access_permission_delegate)) {
    emit storageAccessPermissionDelegateChanged();
  }
}

OxideQQuickWebContextDelegateWorker*
OxideQQuickWebContext::userAgentOverrideDelegate() const {
  Q_D(const OxideQQuickWebContext);

  return d->user_agent_override_delegate_;
}

void OxideQQuickWebContext::setUserAgentOverrideDelegate(
    OxideQQuickWebContextDelegateWorker* delegate) {
  Q_D(OxideQQuickWebContext);

  if (d->attachDelegateWorker(
      delegate,
      &d->user_agent_override_delegate_,
      &d->io_thread_delegate_->user_agent_override_delegate)) {
    emit userAgentOverrideDelegateChanged();
  }
}

#include "moc_oxideqquickwebcontext_p.cpp"
