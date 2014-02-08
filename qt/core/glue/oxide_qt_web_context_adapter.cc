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

#include "oxide_qt_web_context_adapter.h"
#include "oxide_qt_web_context_adapter_p.h"

#include <string>
#include <vector>

#include <QCoreApplication>
#include <QObject>
#include <QtDebug>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "qt/core/glue/private/oxide_qt_user_script_adapter_p.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_user_script_master.h"

namespace oxide {
namespace qt {

namespace {
QOpenGLContext* g_shared_gl_context;
}

struct ConstructProperties {
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
};

WebContextAdapterPrivate::WebContextAdapterPrivate() :
    construct_props_(new ConstructProperties()) {}

void WebContextAdapterPrivate::Init() {
  DCHECK(!context_);

  // We do this here rather than in the constructor because the first
  // browser context needs to set the shared GL context before anything
  // starts up, in order for compositing to work
  // FIXME: What if this fails?
  oxide::BrowserProcessMain::Run(0);

  context_.reset(oxide::BrowserContext::Create(
      construct_props()->data_path,
      construct_props()->cache_path));

  if (!construct_props()->product.empty()) {
    context()->SetProduct(construct_props()->product);
  }
  if (!construct_props()->user_agent.empty()) {
    context()->SetUserAgent(construct_props()->user_agent);
  }
  if (!construct_props()->accept_langs.empty()) {
    context()->SetAcceptLangs(construct_props()->accept_langs);
  }

  construct_props_.reset();
}

// static
WebContextAdapterPrivate* WebContextAdapterPrivate::get(
    WebContextAdapter* adapter) {
  return adapter->priv.data();
}

WebContextAdapter::~WebContextAdapter() {}

QString WebContextAdapter::product() const {
  if (priv->context()) {
    return QString::fromStdString(priv->context()->GetProduct());
  }

  return QString::fromStdString(priv->construct_props()->product);
}

void WebContextAdapter::setProduct(const QString& product) {
  if (priv->context()) {
    priv->context()->SetProduct(product.toStdString());
  } else {
    priv->construct_props()->product = product.toStdString();
  }
}

QString WebContextAdapter::userAgent() const {
  if (priv->context()) {
    return QString::fromStdString(priv->context()->GetUserAgent());
  }

  return QString::fromStdString(priv->construct_props()->user_agent);
}

void WebContextAdapter::setUserAgent(const QString& user_agent) {
  if (priv->context()) {
    priv->context()->SetUserAgent(user_agent.toStdString());
  } else {
    priv->construct_props()->user_agent = user_agent.toStdString();
  }
}

QUrl WebContextAdapter::dataPath() const {
  base::FilePath path;
  if (priv->context()) {
    path = priv->context()->GetPath();
  } else {
    path = priv->construct_props()->data_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContextAdapter::setDataPath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "dataPath only supports local files";
    return;
  }

  DCHECK(!priv->context());
  priv->construct_props()->data_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QUrl WebContextAdapter::cachePath() const {
  base::FilePath path;
  if (priv->context()) {
    path = priv->context()->GetCachePath();
  } else {
    path = priv->construct_props()->cache_path;
  }

  if (path.empty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(QString::fromStdString(path.value()));
}

void WebContextAdapter::setCachePath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "cachePath only supports local files";
    return;
  }

  DCHECK(!priv->context());
  priv->construct_props()->cache_path =
      base::FilePath(url.toLocalFile().toStdString());
}

QString WebContextAdapter::acceptLangs() const {
  if (priv->context()) {
    return QString::fromStdString(priv->context()->GetAcceptLangs());
  }

  return QString::fromStdString(priv->construct_props()->accept_langs);
}

void WebContextAdapter::setAcceptLangs(const QString& langs) {
  if (priv->context()) {
    priv->context()->SetAcceptLangs(langs.toStdString());
  } else {
    priv->construct_props()->accept_langs = langs.toStdString();
  }
}

QList<UserScriptAdapter *>& WebContextAdapter::user_scripts() {
  return user_scripts_;
}

void WebContextAdapter::updateUserScripts() {
  if (!priv->context()) {
    return;
  }

  std::vector<oxide::UserScript *> scripts;
  bool wait = false;

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScriptAdapterPrivate* script =
        UserScriptAdapterPrivate::get(user_scripts_.at(i));
    if (script->state() == UserScriptAdapter::Loading) {
      wait = true;
    } else if (script->state() == UserScriptAdapter::Deferred) {
      script->StartLoading();
      wait = true;
    } else if (script->state() == UserScriptAdapter::Ready) {
      scripts.push_back(&script->user_script());
    }
  }

  if (!wait) {
    priv->context()->UserScriptManager().SerializeUserScriptsAndSendUpdates(
        scripts);
  }
}

bool WebContextAdapter::constructed() const {
  return priv->context() != NULL;
}

void WebContextAdapter::completeConstruction() {
  priv->Init();
  updateUserScripts();
}

/* static */
QOpenGLContext* WebContextAdapter::sharedGLContext() {
  return g_shared_gl_context;
}

/* static */
void WebContextAdapter::setSharedGLContext(QOpenGLContext* context) {
  CHECK(!oxide::BrowserProcessMain::Exists()) <<
      "WebContextAdapter::setSharedGLContext must be called before the "
      "browser components are started!";

  g_shared_gl_context = context;
}

WebContextAdapter::WebContextAdapter() :
    priv(new WebContextAdapterPrivate()) {
  static bool run_once = false;
  if (!run_once) {
    run_once = true;
    // XXX: This seems to fire before all webviews and contexts disappear
    //QObject::connect(QCoreApplication::instance(),
    //                 &QCoreApplication::aboutToQuit,
    //                 oxide::BrowserProcessMain::Quit);

    // XXX: We add this for quicktest, which doesn't use QCoreApplication::exec,
    //      and so doesn't emit aboutToQuit()
    qAddPostRoutine(oxide::BrowserProcessMain::Quit);
  }
}

} // namespace qt
} // namespace oxide
