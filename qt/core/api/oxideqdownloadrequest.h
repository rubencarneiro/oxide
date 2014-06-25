// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef OXIDE_Q_DOWNLOAD_REQUEST
#define OXIDE_Q_DOWNLOAD_REQUEST

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

class OxideQDownloadRequestPrivate;

class Q_DECL_EXPORT OxideQDownloadRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(QString mimeType READ mimeType CONSTANT)
  Q_PROPERTY(bool shouldPrompt READ shouldPrompt CONSTANT)
  Q_PROPERTY(QString suggestedFilename READ suggestedFilename CONSTANT)
  Q_PROPERTY(QStringList cookies READ cookies CONSTANT)
  Q_PROPERTY(QString referrer READ referrer CONSTANT)

  Q_DECLARE_PRIVATE(OxideQDownloadRequest)
  Q_DISABLE_COPY(OxideQDownloadRequest)

 public:
  OxideQDownloadRequest(
      const QUrl& url,
      const QString& mimeType,
      const bool shouldPrompt,
      const QString& suggestedFilename,
      const QString& cookies,
      const QString& referrer,
      QObject* parent = 0);
  virtual ~OxideQDownloadRequest();

  QUrl url() const;
  QString mimeType() const;
  bool shouldPrompt() const;
  QString suggestedFilename() const;
  QStringList cookies() const;
  QString referrer() const;

 private:

  QScopedPointer<OxideQDownloadRequestPrivate> d_ptr;
};

#endif // OXIDE_Q_DOWNLOAD_REQUEST
