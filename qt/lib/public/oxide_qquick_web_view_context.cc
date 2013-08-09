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

#include <QCoreApplication>
#include <string>

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"

#include "oxide_qquick_web_view_context_p.h"

QT_USE_NAMESPACE

namespace {
OxideQQuickWebViewContext* g_default_context;
}

struct LazyInitProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

OxideQQuickWebViewContextPrivate::OxideQQuickWebViewContextPrivate(
    OxideQQuickWebViewContext* q,
    bool is_default) :
    q_ptr(q), is_default_(is_default),
    lazy_init_props_(new LazyInitProperties()) {}

OxideQQuickWebViewContextPrivate::~OxideQQuickWebViewContextPrivate() {}

oxide::BrowserContext* OxideQQuickWebViewContextPrivate::GetContext() {
  if (weak_context_) {
    return weak_context_.get();
  }

  Q_ASSERT(lazy_init_props_);

  if (is_default_) {
    weak_context_ = oxide::BrowserContext::CreateDefault(
        lazy_init_props_->data_path,
        lazy_init_props_->cache_path);
  } else {
    context_.reset(oxide::BrowserContext::Create(
        lazy_init_props_->data_path,
        lazy_init_props_->cache_path));
    weak_context_ = context_->GetWeakPtr();
  }

  if (!lazy_init_props_->product.empty()) {
    weak_context_->SetProduct(lazy_init_props_->product);
  }
  if (!lazy_init_props_->user_agent.empty()) {
    weak_context_->SetUserAgent(lazy_init_props_->user_agent);
  }
  if (!lazy_init_props_->accept_langs.empty()) {
    weak_context_->SetAcceptLangs(lazy_init_props_->accept_langs);
  }

  lazy_init_props_.reset();

  return weak_context_.get();
}

OxideQQuickWebViewContextPrivate* OxideQQuickWebViewContextPrivate::get(
    OxideQQuickWebViewContext* context) {
  return context->d_func();
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebViewContextPrivate(this, false)) {}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(bool is_default,
                                                     QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebViewContextPrivate(this, is_default)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebViewContext::~OxideQQuickWebViewContext() {
  if (g_default_context == this) {
    g_default_context = NULL;
  }
}

// static
OxideQQuickWebViewContext* OxideQQuickWebViewContext::defaultContext() {
  if (g_default_context) {
    return g_default_context;
  }

  return new OxideQQuickWebViewContext(true, QCoreApplication::instance());
}

QString OxideQQuickWebViewContext::product() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    return QString::fromStdString(d->lazy_init_props()->product);
  }

  return QString::fromStdString(d->context()->GetProduct());
}

void OxideQQuickWebViewContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    d->lazy_init_props()->product = product.toStdString();
  } else {
    d->context()->SetProduct(product.toStdString());
  }

  emit productChanged();
}

QString OxideQQuickWebViewContext::userAgent() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    return QString::fromStdString(d->lazy_init_props()->user_agent);
  }

  return QString::fromStdString(d->context()->GetUserAgent());
}

void OxideQQuickWebViewContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  } else {
    d->context()->SetUserAgent(user_agent.toStdString());
  }

  emit userAgentChanged();
}

QString OxideQQuickWebViewContext::dataPath() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    return QString::fromStdString(d->lazy_init_props()->data_path.value());
  }

  return QString::fromStdString(d->context()->GetPath().value());
}

void OxideQQuickWebViewContext::setDataPath(const QString& data_path) {
  Q_D(OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    d->lazy_init_props()->data_path = base::FilePath(data_path.toStdString());
    emit dataPathChanged();
  }
}

QString OxideQQuickWebViewContext::cachePath() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    return QString::fromStdString(d->lazy_init_props()->cache_path.value());
  }

  return QString::fromStdString(d->context()->GetCachePath().value());
}

void OxideQQuickWebViewContext::setCachePath(const QString& cache_path) {
  Q_D(OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    d->lazy_init_props()->cache_path = base::FilePath(cache_path.toStdString());
    emit cachePathChanged();
  }
}

QString OxideQQuickWebViewContext::acceptLangs() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    return QString::fromStdString(d->lazy_init_props()->accept_langs);
  }

  return QString::fromStdString(d->context()->GetAcceptLangs());
}

void OxideQQuickWebViewContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebViewContext);

  if (d->lazy_init_props()) {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  } else {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  }

  emit acceptLangsChanged();
}
