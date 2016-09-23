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

/*!
\class OxideQDownloadRequest
\inheaderfile oxideqdownloadrequest.h
\inmodule OxideQtCore

\brief Request to download a resource

OxideQDownloadRequest represents a request to download a remote resource.
\l{url} specifies the location of the remote resource to download.

The application is responsible for performing the actual download.

\note This API does not make it possible to download URLs that point to
resources inside the web engine (eg, blob: or filesystem URLs).
*/

/*!
\internal
*/

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

/*!
\internal
*/

OxideQDownloadRequest::OxideQDownloadRequest()
    : d(new OxideQDownloadRequestData()) {}

/*!
Destroys the download request.
*/

OxideQDownloadRequest::~OxideQDownloadRequest() {}

/*!
Copy constructs a download request from \a other.
*/

OxideQDownloadRequest::OxideQDownloadRequest(
    const OxideQDownloadRequest& other)
    : d(other.d) {}

/*!
Assigns \a other to this download request.
*/

OxideQDownloadRequest OxideQDownloadRequest::operator=(
    const OxideQDownloadRequest& other) {
  d = other.d;
  return *this;
}

/*!
Returns true if this download request equals \a other. A download request will
only be equal to one that it was copied from.
*/

bool OxideQDownloadRequest::operator==(
    const OxideQDownloadRequest& other) const {
  return d == other.d;
}

/*!
Returns true if this download request not equal \a other.
*/

bool OxideQDownloadRequest::operator!=(
    const OxideQDownloadRequest& other) const {
  return !(*this == other);
}

/*!
Returns the url of the resource to download.
*/

QUrl OxideQDownloadRequest::url() const {
  return d->url;
}

/*!
\deprecated
Returns a hint of the remote resource's mime-type.

\note There are no scenarios in which this is useful to applications. In the
case where a download request is triggered by clicking on an anchor element
with a download attribute, the mime-type is guessed from the extension of
the filename returned by suggestedFilename. The mime-type might be accurate in
the case where a download request is triggered by the value of the
\e{Content-Disposition} header in a response, but this response is discarded
and the application might receive a different response when it tries to
download the resource itself. Applications should not use this to make any
decisions at all.
*/

QString OxideQDownloadRequest::mimeType() const {
  return d->mime_type;
}

/*!
\deprecated

This always returns false.
*/

bool OxideQDownloadRequest::shouldPrompt() const {
  return d->should_prompt;
}

/*!
The suggested name of the destination file. This corresponds to the value of the
\e{download} attribute if this download request is triggered from a HTML anchor
element, or the value of the \e{filename} attribute if the download request is
triggered by a response with a \e{Content-Disposition: attachment} header.
*/

QString OxideQDownloadRequest::suggestedFilename() const {
  return d->suggested_filename;
}

/*!
Returns a list of cookies that the application should use when downloading the
specified resource. The format of each cookie is a string that can be added to
the \e{Cookie} header in a HTTP request.

*/
QStringList OxideQDownloadRequest::cookies() const {
  return d->cookies;
}

/*!
Returns the spec of the referrer URL that the application should specify when
downloading the resource.
*/

QString OxideQDownloadRequest::referrer() const {
  return d->referrer;
}

/*!
Returns the user agent string that the application should use when downloading
the specified resource.
*/

QString OxideQDownloadRequest::userAgent() const {
  return d->user_agent;
}

