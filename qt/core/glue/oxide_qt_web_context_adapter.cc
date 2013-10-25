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

#include <QtDebug>

#include "base/files/file_path.h"
#include "url/gurl.h"

#include "qt/core/glue/private/oxide_qt_web_context_adapter_p.h"

namespace oxide {
namespace qt {

WebContextAdapter::~WebContextAdapter() {}

QString WebContextAdapter::product() const {
  return QString::fromStdString(priv_->GetProduct());
}

void WebContextAdapter::setProduct(const QString& product) {
  priv_->SetProduct(product.toStdString());
}

QString WebContextAdapter::userAgent() const {
  return QString::fromStdString(priv_->GetUserAgent());
}

void WebContextAdapter::setUserAgent(const QString& user_agent) {
  priv_->SetUserAgent(user_agent.toStdString());
}

QUrl WebContextAdapter::dataPath() const {
  QString path(QString::fromStdString(priv_->GetDataPath().value()));
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

  priv_->SetDataPath(base::FilePath(url.toLocalFile().toStdString()));
}

QUrl WebContextAdapter::cachePath() const {
  QString path(QString::fromStdString(priv_->GetCachePath().value()));
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

  priv_->SetCachePath(base::FilePath(url.toLocalFile().toStdString()));
}

QString WebContextAdapter::acceptLangs() const {
  return QString::fromStdString(priv_->GetAcceptLangs());
}

void WebContextAdapter::setAcceptLangs(const QString& langs) {
  priv_->SetAcceptLangs(langs.toStdString());
}

QList<UserScriptAdapter *>& WebContextAdapter::user_scripts() {
  return priv_->user_scripts();
}

void WebContextAdapter::updateUserScripts() {
  priv_->UpdateUserScripts();
}

bool WebContextAdapter::inUse() const {
  return priv_->InUse();
}

WebContextAdapter::WebContextAdapter() :
    priv_(WebContextAdapterPrivate::Create()) {}

} // namespace qt
} // namespace oxide
