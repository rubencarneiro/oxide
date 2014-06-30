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

#include "oxideqdownloadrequest.h"
#include "oxideqdownloadrequest_p.h"

#include "net/http/http_request_headers.h"
#include "url/gurl.h"

namespace {
const QString kCookieListDelimiter = ";";
}

OxideQDownloadRequestPrivate::OxideQDownloadRequestPrivate(
    const QUrl& url,
    const QString& mimeType,
    const bool shouldPrompt,
    const QString& suggestedFilename,
    const QStringList& cookies,
    const QString& referrer)
  : url_(url),
    mime_type_(mimeType),
    should_prompt_(shouldPrompt),
    suggested_filename_(suggestedFilename),
    cookies_(cookies),
    referrer_(referrer) {}

OxideQDownloadRequestPrivate::~OxideQDownloadRequestPrivate() {}

OxideQDownloadRequest::OxideQDownloadRequest(
    const QUrl& url,
    const QString& mimeType,
    const bool shouldPrompt,
    const QString& suggestedFilename,
    const QString& cookies,
    const QString& referrer,
    QObject* parent) :
      QObject(parent),
      d_ptr(new OxideQDownloadRequestPrivate(url,
					     mimeType,
					     shouldPrompt,
					     suggestedFilename,
					     cookies.split(kCookieListDelimiter, QString::SkipEmptyParts),
					     referrer)) {
}

OxideQDownloadRequest::~OxideQDownloadRequest() {}

QUrl OxideQDownloadRequest::url() const {
  Q_D(const OxideQDownloadRequest);

  return d->url_;
}

QString OxideQDownloadRequest::mimeType() const {
  Q_D(const OxideQDownloadRequest);

  return d->mime_type_;
}

bool OxideQDownloadRequest::shouldPrompt() const {
  Q_D(const OxideQDownloadRequest);

  return d->should_prompt_;
}

QString OxideQDownloadRequest::suggestedFilename() const {
  Q_D(const OxideQDownloadRequest);

  return d->suggested_filename_;
}

QStringList OxideQDownloadRequest::cookies() const {
  Q_D(const OxideQDownloadRequest);

  return d->cookies_;
}

QString OxideQDownloadRequest::referrer() const {
  Q_D(const OxideQDownloadRequest);

  return d->referrer_;
}

