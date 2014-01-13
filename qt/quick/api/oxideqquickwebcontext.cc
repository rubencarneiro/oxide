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

#include <QQmlListProperty>
#include <QtDebug>
#include <QtQuickVersion>
#if defined(ENABLE_COMPOSITING)
#include <QtQuick/private/qsgcontext_p.h>
#endif

#include "qt/core/glue/oxide_qt_shared_gl_context_factory.h"

#include "oxideqquickuserscript_p.h"
#include "oxideqquickuserscript_p_p.h"

namespace {
OxideQQuickWebContext* g_default_context;
unsigned int g_context_count = 0;

QOpenGLContext* OxideQQuickSharedGLContextFactory() {
#if defined(ENABLE_COMPOSITING)
  return QSGContext::sharedOpenGLContext();
#else
  return NULL;
#endif
}

}

OxideQQuickWebContextPrivate::OxideQQuickWebContextPrivate(
    OxideQQuickWebContext* q) :
    q_ptr(q) {
  if (g_context_count++ == 0) {
    oxide::qt::SetSharedGLContextFactory(OxideQQuickSharedGLContextFactory);
  }
}

OxideQQuickWebContextPrivate::~OxideQQuickWebContextPrivate() {
  Q_ASSERT(g_context_count > 0);
  --g_context_count;
}

OxideQQuickWebContextPrivate* OxideQQuickWebContextPrivate::get(
    OxideQQuickWebContext* context) {
  return context->d_func();
}

void OxideQQuickWebContextPrivate::userScript_append(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    OxideQQuickUserScript* user_script) {
  if (!user_script) {
    return;
  }

  OxideQQuickUserScriptPrivate* script_priv =
      OxideQQuickUserScriptPrivate::get(user_script);

  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);
  OxideQQuickWebContextPrivate* p = OxideQQuickWebContextPrivate::get(context);

  if (!p->user_scripts().isEmpty() &&
      p->user_scripts().contains(script_priv)) {
    p->user_scripts().removeOne(script_priv);
    p->user_scripts().append(script_priv);
  } else {
    QObject::connect(user_script, SIGNAL(scriptLoaded()),
                     context, SLOT(scriptUpdated()));
    QObject::connect(user_script, SIGNAL(scriptPropertyChanged()),
                     context, SLOT(scriptUpdated()));
    p->user_scripts().append(script_priv);
  }

  p->updateUserScripts();
}

int OxideQQuickWebContextPrivate::userScript_count(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebContextPrivate* p = OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext *>(prop->object));

  return p->user_scripts().size();
}

OxideQQuickUserScript* OxideQQuickWebContextPrivate::userScript_at(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    int index) {
  OxideQQuickWebContextPrivate* p = OxideQQuickWebContextPrivate::get(
        static_cast<OxideQQuickWebContext *>(prop->object));

  if (index >= p->user_scripts().size()) {
    return NULL;
  }

  // XXX: Should we have a better way to get from adapter to public object?
  return static_cast<OxideQQuickUserScriptPrivate *>(
      p->user_scripts().at(index))->q_func();
}

void OxideQQuickWebContextPrivate::userScript_clear(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebContext* context =
      static_cast<OxideQQuickWebContext *>(prop->object);
  OxideQQuickWebContextPrivate* p = OxideQQuickWebContextPrivate::get(context);

  while (p->user_scripts().size() > 0) {
    // XXX: Should we have a better way to get from adapter to public object?
    OxideQQuickUserScriptPrivate* script =
        static_cast<OxideQQuickUserScriptPrivate *>(p->user_scripts().first());
    p->user_scripts().removeFirst();
    QObject::disconnect(script->q_func(), SIGNAL(scriptLoaded()),
                        context, SLOT(scriptUpdated()));
    QObject::disconnect(script->q_func(), SIGNAL(scriptPropertyChanged()),
                        context, SLOT(scriptUpdated()));
    delete script->q_func();
  }

  p->updateUserScripts();
}

void OxideQQuickWebContext::scriptUpdated() {
  Q_D(OxideQQuickWebContext);

  d->updateUserScripts();
}

OxideQQuickWebContext::OxideQQuickWebContext(bool is_default) :
    d_ptr(new OxideQQuickWebContextPrivate(this)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebContextPrivate(this)) {}

OxideQQuickWebContext::~OxideQQuickWebContext() {
  if (g_default_context == this) {
    g_default_context = NULL;
  }
}

void OxideQQuickWebContext::classBegin() {}

void OxideQQuickWebContext::componentComplete() {
  Q_D(OxideQQuickWebContext);

  d->completeConstruction();
}

// static
OxideQQuickWebContext* OxideQQuickWebContext::defaultContext() {
  if (g_default_context) {
    return g_default_context;
  }

  new OxideQQuickWebContext(true);
  g_default_context->componentComplete();

  return g_default_context;
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

  d->setProduct(product);
  emit productChanged();
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

  if (d->constructed()) {
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

  if (d->constructed()) {
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

