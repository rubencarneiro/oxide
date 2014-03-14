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
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

#include "qt/core/gl/oxide_qt_shared_gl_context.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_form_factor.h"
#include "shared/browser/oxide_user_script_master.h"

#include "oxide_qt_user_script_adapter_p.h"

namespace oxide {
namespace qt {

namespace {
QOpenGLContext* g_shared_gl_context;

int GetProcessFlags() {
  switch (oxide::GetFormFactorHint()) {
    case FORM_FACTOR_TABLET:
    case FORM_FACTOR_PHONE:
      return oxide::BrowserProcessMain::ENABLE_VIEWPORT |
             oxide::BrowserProcessMain::ENABLE_OVERLAY_SCROLLBARS;
    default:
      return 0;
  }
}

}

struct ConstructProperties {
  ConstructProperties() :
    inject_oxide_api_in_main_world(false) {}
  std::string product;
  std::string user_agent;
  base::FilePath data_path;
  base::FilePath cache_path;
  std::string accept_langs;
  bool inject_oxide_api_in_main_world;
};

WebContextAdapterPrivate::WebContextAdapterPrivate() :
    construct_props_(new ConstructProperties()) {}

void WebContextAdapterPrivate::Init() {
  DCHECK(!context_);

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

  context()->SetShouldInjectOxideApiInMainWorld(
      construct_props()->inject_oxide_api_in_main_world);

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

bool WebContextAdapter::injectOxideApiInMainWorld() const {
  if (priv->context()) {
    return priv->context()->ShouldInjectOxideApiInMainWorld();
  }

  return priv->construct_props()->inject_oxide_api_in_main_world;
}

void WebContextAdapter::setInjectOxideApiInMainWorld(
      bool inject_oxide_api_in_main_world) {
  if (priv->context()) {
    priv->context()->SetShouldInjectOxideApiInMainWorld(
      inject_oxide_api_in_main_world);
  } else {
    priv->construct_props()->inject_oxide_api_in_main_world =
      inject_oxide_api_in_main_world;
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

  for (int i = 0; i < user_scripts_.size(); ++i) {
    UserScriptAdapterPrivate* script =
        UserScriptAdapterPrivate::get(user_scripts_.at(i));
    if (script->state == UserScriptAdapterPrivate::Loading ||
        script->state == UserScriptAdapterPrivate::Constructing) {
      return;
    } else if (script->state == UserScriptAdapterPrivate::Loaded) {
      scripts.push_back(&script->user_script);
    }
  }

  priv->context()->UserScriptManager().SerializeUserScriptsAndSendUpdates(
      scripts);
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

/* static */
void WebContextAdapter::ensureChromiumStarted() {
  if (!oxide::BrowserProcessMain::Exists()) {
    scoped_refptr<SharedGLContext> shared_gl_context(SharedGLContext::Create());
    oxide::BrowserProcessMain::StartIfNotRunning(
        GetProcessFlags(),
        shared_gl_context);
  }
}

WebContextAdapter::WebContextAdapter() :
    priv(new WebContextAdapterPrivate()) {
  static bool run_once = false;
  if (!run_once) {
    run_once = true;
    qAddPostRoutine(oxide::BrowserProcessMain::ShutdownIfRunning);
  }
}

} // namespace qt
} // namespace oxide
