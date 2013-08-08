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

#include "oxide_qquick_web_view_context.h"

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_handle.h"

#include "oxide_qquick_web_view_context_p.h"

QT_USE_NAMESPACE

OxideQQuickWebViewContextPrivate::OxideQQuickWebViewContextPrivate(
    OxideQQuickWebViewContext* q,
    oxide::BrowserContext* context,
    bool owns_context) :
    q_ptr(q), context_(context), owns_context_(owns_context) {}

OxideQQuickWebViewContextPrivate::~OxideQQuickWebViewContextPrivate() {
  if (context_ && owns_context_) {
    delete context_;
  }
}

OxideQQuickWebViewContextPrivate* OxideQQuickWebViewContextPrivate::get(
    OxideQQuickWebViewContext* context) {
  return context->d_func();
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(
    QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebViewContextPrivate(
              this, oxide::BrowserContext::Create(), true)) {}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(
    oxide::BrowserContext* context, QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebViewContextPrivate(this, context, false)) {}

OxideQQuickWebViewContext::~OxideQQuickWebViewContext() {}

// static
OxideQQuickWebViewContext* OxideQQuickWebViewContext::createForDefault() {
  return new OxideQQuickWebViewContext(oxide::BrowserContext::GetDefault());
}

QString OxideQQuickWebViewContext::product() const {
  Q_D(const OxideQQuickWebViewContext);

  return QString::fromStdString(d->context()->GetProduct());
}

void OxideQQuickWebViewContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebViewContext);

  d->context()->SetProduct(product.toStdString());
  emit productChanged();
}

QString OxideQQuickWebViewContext::userAgent() const {
  Q_D(const OxideQQuickWebViewContext);

  return QString::fromStdString(d->context()->GetUserAgent());
}

void OxideQQuickWebViewContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebViewContext);

  d->context()->SetUserAgent(user_agent.toStdString());
  emit userAgentChanged();
}

QString OxideQQuickWebViewContext::dataPath() const {
  Q_D(const OxideQQuickWebViewContext);

  return QString::fromStdString(d->context()->GetPath().value());
}

void OxideQQuickWebViewContext::setDataPath(const QString& data_path) {
  Q_D(OxideQQuickWebViewContext);

  if (d->context()->SetPath(base::FilePath(data_path.toStdString()))) {
    emit dataPathChanged();
  }
}

QString OxideQQuickWebViewContext::cachePath() const {
  Q_D(const OxideQQuickWebViewContext);

  return QString::fromStdString(d->context()->GetCachePath().value());
}

void OxideQQuickWebViewContext::setCachePath(const QString& cache_path) {
  Q_D(OxideQQuickWebViewContext);

  if (d->context()->SetCachePath(base::FilePath(cache_path.toStdString()))) {
    emit cachePathChanged();
  }
}

QString OxideQQuickWebViewContext::acceptLangs() const {
  Q_D(const OxideQQuickWebViewContext);

  return QString::fromStdString(d->context()->GetAcceptLangs());
}

void OxideQQuickWebViewContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebViewContext);

  d->context()->SetAcceptLangs(accept_langs.toStdString());
  emit acceptLangsChanged();
}
