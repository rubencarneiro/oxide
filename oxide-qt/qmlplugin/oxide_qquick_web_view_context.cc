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

#include "oxide/browser/oxide_global_settings.h"

QT_USE_NAMESPACE

OxideQQuickWebViewContext::OxideQQuickWebViewContext(
    QObject* parent) :
    QObject(parent) {}

OxideQQuickWebViewContext::~OxideQQuickWebViewContext() {}

QString OxideQQuickWebViewContext::product() const {
  return QString::fromStdString(oxide::GlobalSettings::GetProduct());
}

void OxideQQuickWebViewContext::setProduct(const QString& product) {
  oxide::GlobalSettings::SetProduct(product.toStdString());
  emit productChanged();
}

QString OxideQQuickWebViewContext::userAgent() const {
  return QString::fromStdString(oxide::GlobalSettings::GetUserAgent());
}

void OxideQQuickWebViewContext::setUserAgent(const QString& user_agent) {
  oxide::GlobalSettings::SetUserAgent(user_agent.toStdString());
  emit userAgentChanged();
}

QString OxideQQuickWebViewContext::dataPath() const {
  return QString::fromStdString(oxide::GlobalSettings::GetDataPath());
}

void OxideQQuickWebViewContext::setDataPath(const QString& data_path) {
  if (oxide::GlobalSettings::SetDataPath(data_path.toStdString())) {
    emit dataPathChanged();
  }
}

QString OxideQQuickWebViewContext::cachePath() const {
  return QString::fromStdString(oxide::GlobalSettings::GetCachePath());
}

void OxideQQuickWebViewContext::setCachePath(const QString& cache_path) {
  if (oxide::GlobalSettings::SetCachePath(cache_path.toStdString())) {
    emit cachePathChanged();
  }
}

QString OxideQQuickWebViewContext::acceptLangs() const {
  return QString::fromStdString(oxide::GlobalSettings::GetAcceptLangs());
}

void OxideQQuickWebViewContext::setAcceptLangs(const QString& accept_langs) {
  oxide::GlobalSettings::SetAcceptLangs(accept_langs.toStdString());
  emit acceptLangsChanged();
}
