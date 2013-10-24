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

#include <string>

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"

namespace {
OxideQQuickWebContext* g_default_context;
}

void OxideQQuickWebContext::scriptUpdated() {
  Q_D(OxideQQuickWebContext);

  d->updateUserScripts();
}

OxideQQuickWebContext::OxideQQuickWebContext(bool is_default) :
    d_ptr(OxideQQuickWebContextPrivate::Create(this)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebContext::OxideQQuickWebContext(QObject* parent) :
    QObject(parent),
    d_ptr(OxideQQuickWebContextPrivate::Create(this)) {}

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

QString OxideQQuickWebContext::product() const {
  Q_D(const OxideQQuickWebContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetProduct());
  }

  return QString::fromStdString(d->lazy_init_props()->product);
}

void OxideQQuickWebContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebContext);

  if (d->context()) {
    d->context()->SetProduct(product.toStdString());
  } else {
    d->lazy_init_props()->product = product.toStdString();
  }

  emit productChanged();
}

QString OxideQQuickWebContext::userAgent() const {
  Q_D(const OxideQQuickWebContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetUserAgent());
  }

  return QString::fromStdString(d->lazy_init_props()->user_agent);
}

void OxideQQuickWebContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebContext);

  if (d->context()) {
    d->context()->SetUserAgent(user_agent.toStdString());
  } else {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  }

  emit userAgentChanged();
}

QUrl OxideQQuickWebContext::dataPath() const {
  Q_D(const OxideQQuickWebContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetPath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->data_path.value()));
}

void OxideQQuickWebContext::setDataPath(const QUrl& data_url) {
  Q_D(OxideQQuickWebContext);

  if (!d->context()) {
    if (!data_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->data_path =
        base::FilePath(data_url.toLocalFile().toStdString());

    emit dataPathChanged();
  } 
}

QUrl OxideQQuickWebContext::cachePath() const {
  Q_D(const OxideQQuickWebContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetCachePath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->cache_path.value()));
}

void OxideQQuickWebContext::setCachePath(const QUrl& cache_url) {
  Q_D(OxideQQuickWebContext);

  if (!d->context()) {
    if (!cache_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->cache_path =
        base::FilePath(cache_url.toLocalFile().toStdString());

    emit cachePathChanged();
  }
}

QString OxideQQuickWebContext::acceptLangs() const {
  Q_D(const OxideQQuickWebContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetAcceptLangs());
  }

  return QString::fromStdString(d->lazy_init_props()->accept_langs);
}

void OxideQQuickWebContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebContext);

  if (d->context()) {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  } else {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  }

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

