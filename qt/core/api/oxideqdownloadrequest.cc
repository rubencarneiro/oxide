// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxideqdownloadrequest.h"

#include "net/http/http_request_headers.h"
#include "url/gurl.h"

namespace {
const QString kCookieListDelimiter = ";";
}

class OxideQDownloadRequestData : public QSharedData {
 public:
  OxideQDownloadRequestData(const QUrl& url,
                            const QString& mime_type,
                            bool should_prompt,
                            const QString& suggested_filename,
                            const QStringList& cookies,
                            const QString& referrer,
                            const QString& user_agent)
      : url(url),
        mime_type(mime_type),
        should_prompt(should_prompt),
        suggested_filename(suggested_filename),
        cookies(cookies),
        referrer(referrer),
        user_agent(user_agent) {}

  OxideQDownloadRequestData()
      : should_prompt(false) {}

  QUrl url;
  QString mime_type;
  bool should_prompt;
  QString suggested_filename;
  QStringList cookies;
  QString referrer;
  QString user_agent;
};

OxideQDownloadRequest::OxideQDownloadRequest(
    const QUrl& url,
    const QString& mimeType,
    const bool shouldPrompt,
    const QString& suggestedFilename,
    const QString& cookies,
    const QString& referrer,
    const QString& userAgent)
    : d(new OxideQDownloadRequestData(
          url,
          mimeType,
          shouldPrompt,
          suggestedFilename,
          cookies.split(kCookieListDelimiter, QString::SkipEmptyParts),
          referrer,
          userAgent)) {}

OxideQDownloadRequest::OxideQDownloadRequest()
    : d(new OxideQDownloadRequestData()) {}

OxideQDownloadRequest::~OxideQDownloadRequest() {}

OxideQDownloadRequest::OxideQDownloadRequest(
    const OxideQDownloadRequest& other)
    : d(other.d) {}

OxideQDownloadRequest OxideQDownloadRequest::operator=(
    const OxideQDownloadRequest& other) {
  d = other.d;
  return *this;
}

bool OxideQDownloadRequest::operator==(
    const OxideQDownloadRequest& other) const {
  return d == other.d;
}

bool OxideQDownloadRequest::operator!=(
    const OxideQDownloadRequest& other) const {
  return !(*this == other);
}

QUrl OxideQDownloadRequest::url() const {
  return d->url;
}

QString OxideQDownloadRequest::mimeType() const {
  return d->mime_type;
}

bool OxideQDownloadRequest::shouldPrompt() const {
  return d->should_prompt;
}

QString OxideQDownloadRequest::suggestedFilename() const {
  return d->suggested_filename;
}

QStringList OxideQDownloadRequest::cookies() const {
  return d->cookies;
}

QString OxideQDownloadRequest::referrer() const {
  return d->referrer;
}

QString OxideQDownloadRequest::userAgent() const {
  return d->user_agent;
}

