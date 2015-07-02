// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

#include "oxide_qml_download_request.h"

#include <QMetaType>

namespace oxide {
namespace qmlplugin {

DownloadRequest::DownloadRequest(QObject* parent)
    : QQmlValueTypeBase<OxideQDownloadRequest>(
          qMetaTypeId<OxideQDownloadRequest>(),
          parent) {}

DownloadRequest::~DownloadRequest() {}

QUrl DownloadRequest::url() const {
  return v.url();
}

QString DownloadRequest::mimeType() const {
  return v.mimeType();
}

bool DownloadRequest::shouldPrompt() const {
  return v.shouldPrompt();
}

QString DownloadRequest::suggestedFilename() const {
  return v.suggestedFilename();
}

QStringList DownloadRequest::cookies() const {
  return v.cookies();
}

QString DownloadRequest::referrer() const {
  return v.referrer();
}

QString DownloadRequest::userAgent() const {
  return v.userAgent();
}

QString DownloadRequest::toString() const {
  return QString();
}

bool DownloadRequest::isEqual(const QVariant& other) const {
  if (other.userType() != qMetaTypeId<OxideQDownloadRequest>()) {
    return false;
  }

  return v == other.value<OxideQDownloadRequest>();
}

} // namespace qmlplugin
} // namespace oxide
