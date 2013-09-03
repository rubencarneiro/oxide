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

#include "oxide_q_web_view_context_base.h"
#include "oxide_q_web_view_context_base_p.h"
#include "oxide_qquick_web_view_context_p.h"

#include <string>

#include "base/files/file_path.h"

#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_user_script_master.h"

#include "oxide_q_user_script.h"
#include "oxide_q_user_script_p.h"

namespace {
OxideQQuickWebViewContext* g_default_context;
}

namespace oxide {
namespace qt {

struct LazyInitProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

QWebViewContextBasePrivate::QWebViewContextBasePrivate(
    OxideQWebViewContextBase* q) :
    q_ptr(q),
    lazy_init_props_(new LazyInitProperties()) {}

QWebViewContextBasePrivate::~QWebViewContextBasePrivate() {}

oxide::BrowserContext* QWebViewContextBasePrivate::GetContext() {
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
QWebViewContextBasePrivate* QWebViewContextBasePrivate::get(
    OxideQWebViewContextBase* context) {
  return context->d_func();
}

void QWebViewContextBasePrivate::updateUserScripts() {
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
        oxide::qt::QUserScriptPrivate::get(qscript)->user_script());
  }

  context_->UserScriptManager().SerializeUserScriptsAndSendUpdates(scripts);
}

class QQuickWebViewContextPrivate FINAL : public QWebViewContextBasePrivate {
  Q_DECLARE_PUBLIC(OxideQQuickWebViewContext)

 public:
  QQuickWebViewContextPrivate(OxideQQuickWebViewContext* q);
  ~QQuickWebViewContextPrivate();

  static QQuickWebViewContextPrivate* get(
      OxideQQuickWebViewContext* context);

  static void userScript_append(QQmlListProperty<OxideQUserScript>* prop,
                                OxideQUserScript* user_script);
  static int userScript_count(QQmlListProperty<OxideQUserScript>* prop);
  static OxideQUserScript* userScript_at(
      QQmlListProperty<OxideQUserScript>* prop,
      int index);
  static void userScript_clear(QQmlListProperty<OxideQUserScript>* prop);

 private:
  DISALLOW_COPY_AND_ASSIGN(QQuickWebViewContextPrivate);
};

QQuickWebViewContextPrivate::QQuickWebViewContextPrivate(
    OxideQQuickWebViewContext* q) :
    QWebViewContextBasePrivate(q) {}

QQuickWebViewContextPrivate::~QQuickWebViewContextPrivate() {}

QQuickWebViewContextPrivate* QQuickWebViewContextPrivate::get(
    OxideQQuickWebViewContext* context) {
  return context->d_func();
}

void QQuickWebViewContextPrivate::userScript_append(
    QQmlListProperty<OxideQUserScript>* prop,
    OxideQUserScript* user_script) {
  if (!user_script) {
    return;
  }

  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  QQuickWebViewContextPrivate* p = QQuickWebViewContextPrivate::get(context);

  if (!p->user_scripts().isEmpty() &&
      p->user_scripts().contains(user_script)) {
    p->user_scripts().removeOne(user_script);
    p->user_scripts().append(user_script);
  } else {
    QObject::connect(user_script, SIGNAL(scriptLoaded()),
                     context, SLOT(scriptUpdated()));
    QObject::connect(user_script, SIGNAL(scriptPropertyChanged()),
                     context, SLOT(scriptUpdated()));
    p->user_scripts().append(user_script);
  }

  p->updateUserScripts();
}

int QQuickWebViewContextPrivate::userScript_count(
    QQmlListProperty<OxideQUserScript>* prop) {
  QQuickWebViewContextPrivate* p = QQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  return p->user_scripts().size();
}

OxideQUserScript* QQuickWebViewContextPrivate::userScript_at(
    QQmlListProperty<OxideQUserScript>* prop,
    int index) {
  QQuickWebViewContextPrivate* p = QQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  if (index >= p->user_scripts().size()) {
    return NULL;
  }

  return qobject_cast<OxideQUserScript *>(p->user_scripts().at(index));
}

void QQuickWebViewContextPrivate::userScript_clear(
    QQmlListProperty<OxideQUserScript>* prop) {
  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  QQuickWebViewContextPrivate* p = QQuickWebViewContextPrivate::get(context);

  while (p->user_scripts().size() > 0) {
    OxideQUserScript* script = p->user_scripts().first();
    p->user_scripts().removeFirst();
    QObject::disconnect(script, SIGNAL(scriptLoaded()),
                        context, SLOT(scriptUpdated()));
    QObject::disconnect(script, SIGNAL(scriptPropertyChanged()),
                        context, SLOT(scriptUpdated()));
    delete script;
  }

  p->updateUserScripts();
}

} // namespace qt
} // namespace oxide

void OxideQWebViewContextBase::scriptUpdated() {
  Q_D(oxide::qt::QWebViewContextBase);

  d->updateUserScripts();
}

OxideQWebViewContextBase::OxideQWebViewContextBase(
    oxide::qt::QWebViewContextBasePrivate& dd,
    QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

OxideQWebViewContextBase::~OxideQWebViewContextBase() {}

QString OxideQWebViewContextBase::product() const {
  Q_D(const oxide::qt::QWebViewContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetProduct());
  }

  return QString::fromStdString(d->lazy_init_props()->product);
}

void OxideQWebViewContextBase::setProduct(const QString& product) {
  Q_D(oxide::qt::QWebViewContextBase);

  if (d->context()) {
    d->context()->SetProduct(product.toStdString());
  } else {
    d->lazy_init_props()->product = product.toStdString();
  }

  emit productChanged();
}

QString OxideQWebViewContextBase::userAgent() const {
  Q_D(const oxide::qt::QWebViewContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetUserAgent());
  }

  return QString::fromStdString(d->lazy_init_props()->user_agent);
}

void OxideQWebViewContextBase::setUserAgent(const QString& user_agent) {
  Q_D(oxide::qt::QWebViewContextBase);

  if (d->context()) {
    d->context()->SetUserAgent(user_agent.toStdString());
  } else {
    d->lazy_init_props()->user_agent = user_agent.toStdString();
  }

  emit userAgentChanged();
}

QUrl OxideQWebViewContextBase::dataPath() const {
  Q_D(const oxide::qt::QWebViewContextBase);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetPath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->data_path.value()));
}

void OxideQWebViewContextBase::setDataPath(const QUrl& data_url) {
  Q_D(oxide::qt::QWebViewContextBase);

  if (!d->context()) {
    if (!data_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->data_path =
        base::FilePath(data_url.toLocalFile().toStdString());

    emit dataPathChanged();
  } 
}

QUrl OxideQWebViewContextBase::cachePath() const {
  Q_D(const oxide::qt::QWebViewContextBase);

  if (d->context()) {
    return QUrl::fromLocalFile(
        QString::fromStdString(d->context()->GetCachePath().value()));
  }

  return QUrl::fromLocalFile(
      QString::fromStdString(d->lazy_init_props()->cache_path.value()));
}

void OxideQWebViewContextBase::setCachePath(const QUrl& cache_url) {
  Q_D(oxide::qt::QWebViewContextBase);

  if (!d->context()) {
    if (!cache_url.isLocalFile()) {
      return;
    }
    d->lazy_init_props()->cache_path =
        base::FilePath(cache_url.toLocalFile().toStdString());

    emit cachePathChanged();
  }
}

QString OxideQWebViewContextBase::acceptLangs() const {
  Q_D(const oxide::qt::QWebViewContextBase);

  if (d->context()) {
    return QString::fromStdString(d->context()->GetAcceptLangs());
  }

  return QString::fromStdString(d->lazy_init_props()->accept_langs);
}

void OxideQWebViewContextBase::setAcceptLangs(const QString& accept_langs) {
  Q_D(oxide::qt::QWebViewContextBase);

  if (d->context()) {
    d->context()->SetAcceptLangs(accept_langs.toStdString());
  } else {
    d->lazy_init_props()->accept_langs = accept_langs.toStdString();
  }

  emit acceptLangsChanged();
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(bool is_default) :
    OxideQWebViewContextBase(*new oxide::qt::QQuickWebViewContextPrivate(this)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(QObject* parent) :
    OxideQWebViewContextBase(*new oxide::qt::QQuickWebViewContextPrivate(this),
                             parent) {}

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

QQmlListProperty<OxideQUserScript>
OxideQQuickWebViewContext::userScripts() {
  return QQmlListProperty<OxideQUserScript>(
      this, NULL,
      oxide::qt::QQuickWebViewContextPrivate::userScript_append,
      oxide::qt::QQuickWebViewContextPrivate::userScript_count,
      oxide::qt::QQuickWebViewContextPrivate::userScript_at,
      oxide::qt::QQuickWebViewContextPrivate::userScript_clear);
}

