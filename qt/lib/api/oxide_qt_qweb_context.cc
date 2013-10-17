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

#include "oxide_q_web_context_base.h"
#include "oxide_qquick_web_context_p.h"

#include <string>

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"

#include "qt/lib/api/private/oxide_qt_qweb_context_p.h"

namespace {
OxideQQuickWebContext* g_default_context;
}

void OxideQWebContextBase::scriptUpdated() {
  Q_D(oxide::qt::QWebContextBase);

  d->updateUserScripts();
}

OxideQWebContextBase::OxideQWebContextBase(
    oxide::qt::QWebContextBasePrivate& dd,
    QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

OxideQWebContextBase::~OxideQWebContextBase() {}

QString OxideQWebContextBase::product() const {
  Q_D(const oxide::qt::QWebContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetProduct());
  }

  return QString::fromStdString(d->lazy_init_props()->product);
}

void OxideQWebContextBase::setProduct(const QString& product) {
  Q_D(oxide::qt::QWebContextBase);

  if (d->context()) {
    d->context()->SetProduct(product.toStdString());
  } else {
    d->lazy_init_props()->product = product.toStdString();
  }

  emit productChanged();
}

QString OxideQWebContextBase::userAgent() const {
  Q_D(const oxide::qt::QWebContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetUserAgent());
  }

  return QString::fromStdString(d->lazy_init_props()->user_agent);
}

void OxideQWebContextBase::setUserAgent(const QString& user_agent) {
  Q_D(oxide::qt::QWebContextBase);

  if (d->context()) {
    d->context()->SetUserAgent(user_agent.toStdString());
  } else {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  }

  emit userAgentChanged();
}

QUrl OxideQWebContextBase::dataPath() const {
  Q_D(const oxide::qt::QWebContextBase);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetPath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->data_path.value()));
}

void OxideQWebContextBase::setDataPath(const QUrl& data_url) {
  Q_D(oxide::qt::QWebContextBase);

  if (!d->context()) {
    if (!data_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->data_path =
        base::FilePath(data_url.toLocalFile().toStdString());

    emit dataPathChanged();
  } 
}

QUrl OxideQWebContextBase::cachePath() const {
  Q_D(const oxide::qt::QWebContextBase);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetCachePath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->cache_path.value()));
}

void OxideQWebContextBase::setCachePath(const QUrl& cache_url) {
  Q_D(oxide::qt::QWebContextBase);

  if (!d->context()) {
    if (!cache_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->cache_path =
        base::FilePath(cache_url.toLocalFile().toStdString());

    emit cachePathChanged();
  }
}

QString OxideQWebContextBase::acceptLangs() const {
  Q_D(const oxide::qt::QWebContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetAcceptLangs());
  }

  return QString::fromStdString(d->lazy_init_props()->accept_langs);
}

void OxideQWebContextBase::setAcceptLangs(const QString& accept_langs) {
  Q_D(oxide::qt::QWebContextBase);

  if (d->context()) {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  } else {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  }

  emit acceptLangsChanged();
}

OxideQQuickWebContext::OxideQQuickWebContext(bool is_default) :
    OxideQWebContextBase(
      *oxide::qt::QQuickWebContextPrivate::Create(this)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent) :
    OxideQWebContextBase(
      *oxide::qt::QQuickWebContextPrivate::Create(this),
      parent) {}

OxideQQuickWebContext::~OxideQQuickWebContext() {
  if (g_default_context == this) {
    g_default_context = NULL;
  }
}

// static
OxideQQuickWebContext* OxideQQuickWebContext::defaultContext() {
  if (g_default_context) {
    return g_default_context;
  }

  return new OxideQQuickWebContext(true);
}

QQmlListProperty<OxideQUserScript>
OxideQQuickWebContext::userScripts() {
  return QQmlListProperty<OxideQUserScript>(
      this, NULL,
      oxide::qt::QQuickWebContextPrivate::userScript_append,
      oxide::qt::QQuickWebContextPrivate::userScript_count,
      oxide::qt::QQuickWebContextPrivate::userScript_at,
      oxide::qt::QQuickWebContextPrivate::userScript_clear);
}

