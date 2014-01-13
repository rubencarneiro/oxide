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

#include <QCoreApplication>
#include <QObject>
#include <QtDebug>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "qt/core/glue/private/oxide_qt_web_context_adapter_p.h"
#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {
namespace qt {

namespace {
QOpenGLContext* g_shared_gl_context;
}

WebContextAdapter::~WebContextAdapter() {}

QString WebContextAdapter::product() const {
  return QString::fromStdString(priv->GetProduct());
}

void WebContextAdapter::setProduct(const QString& product) {
  priv->SetProduct(product.toStdString());
}

QString WebContextAdapter::userAgent() const {
  return QString::fromStdString(priv->GetUserAgent());
}

void WebContextAdapter::setUserAgent(const QString& user_agent) {
  priv->SetUserAgent(user_agent.toStdString());
}

QUrl WebContextAdapter::dataPath() const {
  QString path(QString::fromStdString(priv->GetDataPath().value()));
  if (path.isEmpty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(path);
}

void WebContextAdapter::setDataPath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "dataPath only supports local files";
    return;
  }

  priv->SetDataPath(base::FilePath(url.toLocalFile().toStdString()));
}

QUrl WebContextAdapter::cachePath() const {
  QString path(QString::fromStdString(priv->GetCachePath().value()));
  if (path.isEmpty()) {
    return QUrl();
  }

  return QUrl::fromLocalFile(path);
}

void WebContextAdapter::setCachePath(const QUrl& url) {
  if (!url.isLocalFile() && !url.isEmpty()) {
    qWarning() << "cachePath only supports local files";
    return;
  }

  priv->SetCachePath(base::FilePath(url.toLocalFile().toStdString()));
}

QString WebContextAdapter::acceptLangs() const {
  return QString::fromStdString(priv->GetAcceptLangs());
}

void WebContextAdapter::setAcceptLangs(const QString& langs) {
  priv->SetAcceptLangs(langs.toStdString());
}

QList<UserScriptAdapter *>& WebContextAdapter::user_scripts() {
  return priv->user_scripts();
}

void WebContextAdapter::updateUserScripts() {
  priv->UpdateUserScripts();
}

bool WebContextAdapter::constructed() const {
  return priv->context() != NULL;
}

void WebContextAdapter::completeConstruction() {
  priv->CompleteConstruction();
}

/* static */
QOpenGLContext* WebContextAdapter::sharedGLContext() {
  return g_shared_gl_context;
}

/* static */
void WebContextAdapter::setSharedGLContext(QOpenGLContext* context) {
  DCHECK(!oxide::BrowserProcessMain::IsRunning()) <<
      "WebContextAdapter::setSharedGLContext must be called before the "
      "browser components are started!";

  g_shared_gl_context = context;
}

WebContextAdapter::WebContextAdapter() :
    priv(WebContextAdapterPrivate::Create()) {
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
