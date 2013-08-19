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

#include "oxide_qt_web_view_context.h"

#include <string>

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_user_script_master.h"

#include "oxide_q_user_script.h"
#include "oxide_q_user_script_p.h"
#include "oxide_qt_web_view_context_p.h"

namespace oxide {
namespace qt {

struct LazyInitProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

WebViewContextPrivate::WebViewContextPrivate(WebViewContext* q) :
    q_ptr(q), lazy_init_props_(new LazyInitProperties()) {}

WebViewContextPrivate::~WebViewContextPrivate() {}

oxide::BrowserContext* WebViewContextPrivate::GetContext() {
  if (context_) {
    return context_.get();
  }

  context_.reset(oxide::BrowserContext::Create(
      lazy_init_props_->data_path,
      lazy_init_props_->cache_path));

  if (!lazy_init_props_->product.empty()) {
    context_->SetProduct(lazy_init_props_->product);
  }
  if (!lazy_init_props_->user_agent.empty()) {
    context_->SetUserAgent(lazy_init_props_->user_agent);
  }
  if (!lazy_init_props_->accept_langs.empty()) {
    context_->SetAcceptLangs(lazy_init_props_->accept_langs);
  }

  lazy_init_props_.reset();

  updateUserScripts();

  return context_.get();
}

// static
WebViewContextPrivate* WebViewContextPrivate::get(WebViewContext* context) {
  return context->d_func();
}

void WebViewContextPrivate::updateUserScripts() {
  if (!context_) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    OxideQUserScript* qscript = user_scripts_.at(i);
    if (qscript->state() == OxideQUserScript::Loading) {
      return;
    }

    if (qscript->state() != OxideQUserScript::Ready) {
      continue;
    }

    scripts.push_back(
        OxideQUserScriptPrivate::get(qscript)->user_script());
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

void WebViewContext::scriptUpdated() {
  Q_D(WebViewContext);

  d->updateUserScripts();
}

WebViewContext::WebViewContext(WebViewContextPrivate& dd,
                               QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

WebViewContext::~WebViewContext() {}

QString WebViewContext::product() const {
  Q_D(const WebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetProduct());
  }

  return QString::fromStdString(d->lazy_init_props()->product);
}

void WebViewContext::setProduct(const QString& product) {
  Q_D(WebViewContext);

  if (d->context()) {
    d->context()->SetProduct(product.toStdString());
  } else {
    d->lazy_init_props()->product = product.toStdString();
  }

  emit productChanged();
}

QString WebViewContext::userAgent() const {
  Q_D(const WebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetUserAgent());
  }

  return QString::fromStdString(d->lazy_init_props()->user_agent);
}

void WebViewContext::setUserAgent(const QString& user_agent) {
  Q_D(WebViewContext);

  if (d->context()) {
    d->context()->SetUserAgent(user_agent.toStdString());
  } else {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  }

  emit userAgentChanged();
}

QUrl WebViewContext::dataPath() const {
  Q_D(const WebViewContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetPath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->data_path.value()));
}

void WebViewContext::setDataPath(const QUrl& data_url) {
  Q_D(WebViewContext);

  if (!d->context()) {
    if (!data_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->data_path =
        base::FilePath(data_url.toLocalFile().toStdString());

    emit dataPathChanged();
  } 
}

QUrl WebViewContext::cachePath() const {
  Q_D(const WebViewContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetCachePath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->cache_path.value()));
}

void WebViewContext::setCachePath(const QUrl& cache_url) {
  Q_D(WebViewContext);

  if (!d->context()) {
    if (!cache_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->cache_path =
        base::FilePath(cache_url.toLocalFile().toStdString());

    emit cachePathChanged();
  }
}

QString WebViewContext::acceptLangs() const {
  Q_D(const WebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetAcceptLangs());
  }

  return QString::fromStdString(d->lazy_init_props()->accept_langs);
}

void WebViewContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(WebViewContext);

  if (d->context()) {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  } else {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  }

  emit acceptLangsChanged();
}

} // namespace qt
} // namespace oxide
