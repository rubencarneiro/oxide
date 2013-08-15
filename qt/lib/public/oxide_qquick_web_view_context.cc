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
#include "shared/browser/oxide_user_script_master.h"

#include "oxide_qquick_user_script.h"
#include "oxide_qquick_user_script_p.h"
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
    OxideQQuickWebViewContext* q) :
    q_ptr(q), lazy_init_props_(new LazyInitProperties()) {}

OxideQQuickWebViewContextPrivate::~OxideQQuickWebViewContextPrivate() {}

oxide::BrowserContext* OxideQQuickWebViewContextPrivate::GetContext() {
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

OxideQQuickWebViewContextPrivate* OxideQQuickWebViewContextPrivate::get(
    OxideQQuickWebViewContext* context) {
  return context->d_func();
}

void OxideQQuickWebViewContextPrivate::userScript_append(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    OxideQQuickUserScript* user_script) {
  if (!user_script) {
    return;
  }

  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(context);

  if (!p->user_scripts_.isEmpty() && p->user_scripts_.contains(user_script)) {
    p->user_scripts_.removeOne(user_script);
    p->user_scripts_.append(user_script);
  } else {
    QObject::connect(user_script, SIGNAL(scriptLoaded()),
                     context, SLOT(scriptUpdated()));
    QObject::connect(user_script, SIGNAL(scriptPropertyChanged()),
                     context, SLOT(scriptUpdated()));
    p->user_scripts_.append(user_script);
  }

  p->updateUserScripts();
}

int OxideQQuickWebViewContextPrivate::userScript_count(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  return p->user_scripts_.size();
}

OxideQQuickUserScript* OxideQQuickWebViewContextPrivate::userScript_at(
    QQmlListProperty<OxideQQuickUserScript>* prop,
    int index) {
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  if (index >= p->user_scripts_.size()) {
    return NULL;
  }

  return p->user_scripts_.at(index);
}

void OxideQQuickWebViewContextPrivate::userScript_clear(
    QQmlListProperty<OxideQQuickUserScript>* prop) {
  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(context);

  while (p->user_scripts_.size() > 0) {
    OxideQQuickUserScript* script = p->user_scripts_.first();
    p->user_scripts_.removeFirst();
    QObject::disconnect(script, SIGNAL(scriptLoaded()),
                        context, SLOT(scriptUpdated()));
    QObject::disconnect(script, SIGNAL(scriptPropertyChanged()),
                        context, SLOT(scriptUpdated()));
    delete script;
  }

  p->updateUserScripts();
}

void OxideQQuickWebViewContextPrivate::updateUserScripts() {
  if (!context_) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    OxideQQuickUserScript* qscript = user_scripts_.at(i);
    if (qscript->state() == OxideQQuickUserScript::Loading) {
      return;
    }

    if (qscript->state() != OxideQQuickUserScript::Ready) {
      continue;
    }

    scripts.push_back(
        OxideQQuickUserScriptPrivate::get(qscript)->user_script());
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

void OxideQQuickWebViewContext::scriptUpdated() {
  Q_D(OxideQQuickWebViewContext);

  d->updateUserScripts();
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickWebViewContextPrivate(this)) {}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(bool is_default) :
    QObject(),
    d_ptr(new OxideQQuickWebViewContextPrivate(this)) {
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

  return new OxideQQuickWebViewContext(true);
}

QString OxideQQuickWebViewContext::product() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetProduct());
  }

  return QString::fromStdString(d->lazy_init_props()->product);
}

void OxideQQuickWebViewContext::setProduct(const QString& product) {
  Q_D(OxideQQuickWebViewContext);

  if (d->context()) {
    d->context()->SetProduct(product.toStdString());
  } else {
    d->lazy_init_props()->product = product.toStdString();
  }

  emit productChanged();
}

QString OxideQQuickWebViewContext::userAgent() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetUserAgent());
  }

  return QString::fromStdString(d->lazy_init_props()->user_agent);
}

void OxideQQuickWebViewContext::setUserAgent(const QString& user_agent) {
  Q_D(OxideQQuickWebViewContext);

  if (d->context()) {
    d->context()->SetUserAgent(user_agent.toStdString());
  } else {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  }

  emit userAgentChanged();
}

QUrl OxideQQuickWebViewContext::dataPath() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetPath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->data_path.value()));
}

void OxideQQuickWebViewContext::setDataPath(const QUrl& data_url) {
  Q_D(OxideQQuickWebViewContext);

  if (!d->context()) {
    if (!data_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->data_path =
        base::FilePath(data_url.toLocalFile().toStdString());

    emit dataPathChanged();
  } 
}

QUrl OxideQQuickWebViewContext::cachePath() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetCachePath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->cache_path.value()));
}

void OxideQQuickWebViewContext::setCachePath(const QUrl& cache_url) {
  Q_D(OxideQQuickWebViewContext);

  if (!d->context()) {
    if (!cache_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->cache_path =
        base::FilePath(cache_url.toLocalFile().toStdString());

    emit cachePathChanged();
  }
}

QString OxideQQuickWebViewContext::acceptLangs() const {
  Q_D(const OxideQQuickWebViewContext);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetAcceptLangs());
  }

  return QString::fromStdString(d->lazy_init_props()->accept_langs);
}

void OxideQQuickWebViewContext::setAcceptLangs(const QString& accept_langs) {
  Q_D(OxideQQuickWebViewContext);

  if (d->context()) {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  } else {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  }

  emit acceptLangsChanged();
}

QQmlListProperty<OxideQQuickUserScript>
OxideQQuickWebViewContext::userScripts() {
  return QQmlListProperty<OxideQQuickUserScript>(
      this, NULL,
      OxideQQuickWebViewContextPrivate::userScript_append,
      OxideQQuickWebViewContextPrivate::userScript_count,
      OxideQQuickWebViewContextPrivate::userScript_at,
      OxideQQuickWebViewContextPrivate::userScript_clear);
}

