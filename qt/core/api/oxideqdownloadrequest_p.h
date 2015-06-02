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

#ifndef _OXIDE_QT_CORE_API_DOWNLOAD_REQUEST_P_H_
#define _OXIDE_QT_CORE_API_DOWNLOAD_REQUEST_P_H_

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QUrl>

class OxideQDownloadRequest;

class OxideQDownloadRequestPrivate {
 public:
  OxideQDownloadRequestPrivate(
      const QUrl& url,
      const QString& mimeType,
      const bool shouldPrompt,
      const QString& suggestedFilename,
      const QStringList& cookies,
      const QString& referrer,
      const QString& userAgent);
  virtual ~OxideQDownloadRequestPrivate();

 private:
  friend class OxideQDownloadRequest;

  QUrl url_;
  QString mime_type_;
  bool should_prompt_;
  QString suggested_filename_;
  QStringList cookies_;
  QString referrer_;
  QString user_agent_;
};

#endif // _OXIDE_QT_CORE_API_DOWNLOAD_REQUEST_P_H_

