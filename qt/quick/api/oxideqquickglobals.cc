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

#include "oxideqquickglobals_p.h"
#include "oxideqquickglobals_p_p.h"

#include <QCoreApplication>
#include <QSharedPointer>

#include "oxideqquickwebcontext_p.h"

namespace {
class OxideQQuickGlobals* g_instance;
}

OxideQQuickGlobalsPrivate::OxideQQuickGlobalsPrivate(
    OxideQQuickGlobals* q) :
    q_ptr(q),
    has_default_context_(false) {
  Q_ASSERT(!g_instance);
}

void OxideQQuickGlobalsPrivate::defaultContextCreated() {
  Q_Q(OxideQQuickGlobals);

  has_default_context_ = true;

  QSharedPointer<OxideQQuickWebContext> context(
      OxideQQuickWebContext::defaultContext());

  QObject::connect(context.data(), SIGNAL(destroyed()),
                   q, SLOT(defaultContextDestroyed()));

  QObject::connect(context.data(), SIGNAL(productChanged()),
                   q, SLOT(defaultContextProductChanged()));
  QObject::connect(context.data(), SIGNAL(userAgentChanged()),
                   q, SLOT(defaultContextUserAgentChanged()));
  QObject::connect(context.data(), SIGNAL(dataPathChanged()),
                   q, SLOT(defaultContextDataPathChanged()));
  QObject::connect(context.data(), SIGNAL(cachePathChanged()),
                   q, SLOT(defaultContextCachePathChanged()));
  QObject::connect(context.data(), SIGNAL(acceptLangsChanged()),
                   q, SLOT(defaultContextAcceptLangsChanged()));
}

void OxideQQuickGlobalsPrivate::defaultContextDestroyed() {
  has_default_context_ = false;
}

void OxideQQuickGlobalsPrivate::defaultContextProductChanged() {
  Q_Q(OxideQQuickGlobals);

  product = OxideQQuickWebContext::defaultContext()->product();
  Q_EMIT q->productChanged();
}

void OxideQQuickGlobalsPrivate::defaultContextUserAgentChanged() {
  Q_Q(OxideQQuickGlobals);

  user_agent = OxideQQuickWebContext::defaultContext()->userAgent();
  Q_EMIT q->userAgentChanged();
}

void OxideQQuickGlobalsPrivate::defaultContextDataPathChanged() {
  Q_Q(OxideQQuickGlobals);

  data_path = OxideQQuickWebContext::defaultContext()->dataPath();
  Q_EMIT q->dataPathChanged();
}

void OxideQQuickGlobalsPrivate::defaultContextCachePathChanged() {
  Q_Q(OxideQQuickGlobals);

  cache_path = OxideQQuickWebContext::defaultContext()->cachePath();
  Q_EMIT q->cachePathChanged();
}

void OxideQQuickGlobalsPrivate::defaultContextAcceptLangsChanged() {
  Q_Q(OxideQQuickGlobals);

  accept_langs = OxideQQuickWebContext::defaultContext()->acceptLangs();
  Q_EMIT q->acceptLangsChanged();
}

// static
OxideQQuickGlobalsPrivate* OxideQQuickGlobalsPrivate::get(
    OxideQQuickGlobals* q) {
  return q->d_func();
}

OxideQQuickGlobals::OxideQQuickGlobals() :
    QObject(QCoreApplication::instance()),
    d_ptr(new OxideQQuickGlobalsPrivate(this)) {}

// static
OxideQQuickGlobals* OxideQQuickGlobals::instance() {
  if (!g_instance) {
    g_instance = new OxideQQuickGlobals();
  }

  Q_ASSERT(g_instance);

  return g_instance;
}

OxideQQuickGlobals::~OxideQQuickGlobals() {
  Q_ASSERT(this == g_instance);
  g_instance = NULL;
}

QString OxideQQuickGlobals::product() const {
  Q_D(const OxideQQuickGlobals);

  return d->product;
}

void OxideQQuickGlobals::setProduct(const QString& product) {
  Q_D(OxideQQuickGlobals);

  if (d->has_default_context()) {
    OxideQQuickWebContext::defaultContext()->setProduct(product);
  } else {
    d->product = product;
    Q_EMIT productChanged();
  }
}

QString OxideQQuickGlobals::userAgent() const {
  Q_D(const OxideQQuickGlobals);

  return d->user_agent;
}

void OxideQQuickGlobals::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickGlobals);

  if (d->has_default_context()) {
    OxideQQuickWebContext::defaultContext()->setUserAgent(user_agent);
  } else {
    d->user_agent = user_agent;
    Q_EMIT userAgentChanged();
  }
}

QUrl OxideQQuickGlobals::dataPath() const {
  Q_D(const OxideQQuickGlobals);

  return d->data_path;
}

void OxideQQuickGlobals::setDataPath(const QUrl& data_path) {
  Q_D(OxideQQuickGlobals);

  if (d->has_default_context()) {
    OxideQQuickWebContext::defaultContext()->setDataPath(data_path);
  } else {
    d->data_path = data_path;
    Q_EMIT dataPathChanged();
  }
}

QUrl OxideQQuickGlobals::cachePath() const {
  Q_D(const OxideQQuickGlobals);

  return d->cache_path;
}

void OxideQQuickGlobals::setCachePath(const QUrl& cache_path) {
  Q_D(OxideQQuickGlobals);

  if (d->has_default_context()) {
    OxideQQuickWebContext::defaultContext()->setCachePath(cache_path);
  } else {
    d->cache_path = cache_path;
    Q_EMIT cachePathChanged();
  }
}

QString OxideQQuickGlobals::acceptLangs() const {
  Q_D(const OxideQQuickGlobals);

  return d->accept_langs;
}

void OxideQQuickGlobals::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickGlobals);

  if (d->has_default_context()) {
    OxideQQuickWebContext::defaultContext()->setAcceptLangs(accept_langs);
  } else {
    d->accept_langs = accept_langs;
    Q_EMIT acceptLangsChanged();
  }
}

OxideQQuickWebContext* OxideQQuickGlobals::_defaultWebContext() {
  Q_D(OxideQQuickGlobals);
  static bool warn_once = false;
  if (!warn_once) {
    qWarning("_defaultWebContext() is not intended for public use. "
             "The Oxide global object provides a means to configure "
             "some paramters for the default WebContext. Please "
             "use this instead.");
    warn_once = true;
  }

  if (!d->has_default_context()) {
    return NULL;
  }

  return OxideQQuickWebContext::defaultContext().data();
}

#include "moc_oxideqquickglobals_p.cpp"
