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

#ifndef _OXIDE_QMLPLUGIN_DOWNLOAD_REQUEST_H_
#define _OXIDE_QMLPLUGIN_DOWNLOAD_REQUEST_H_

#include <QtGlobal>
#include <QtQml/private/qqmlvaluetype_p.h>

#include "qt/core/api/oxideqdownloadrequest.h"

namespace oxide {
namespace qmlplugin {

class DownloadRequest : public QQmlValueTypeBase<OxideQDownloadRequest> {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(QString mimeType READ mimeType CONSTANT)
  Q_PROPERTY(bool shouldPrompt READ shouldPrompt CONSTANT)
  Q_PROPERTY(QString suggestedFilename READ suggestedFilename CONSTANT)
  Q_PROPERTY(QStringList cookies READ cookies CONSTANT)
  Q_PROPERTY(QString referrer READ referrer CONSTANT)
  Q_PROPERTY(QString userAgent READ userAgent CONSTANT)

  Q_DISABLE_COPY(DownloadRequest)

 public:
  DownloadRequest(QObject* parent = nullptr);
  ~DownloadRequest() override;

  QUrl url() const;
  QString mimeType() const;
  bool shouldPrompt() const;
  QString suggestedFilename() const;
  QStringList cookies() const;
  QString referrer() const;
  QString userAgent() const;

  // QQmlValueType implementation
  QString toString() const override;
  bool isEqual(const QVariant& other) const override;
};

} // namespace qmlplugin
} // namespace oxide

#endif // _OXIDE_QMLPLUGIN_DOWNLOAD_REQUEST_H_
