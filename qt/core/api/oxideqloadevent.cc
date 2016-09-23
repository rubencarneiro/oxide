// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxideqloadevent.h"

class OxideQLoadEventData : public QSharedData {
 public:
  OxideQLoadEventData()
      : type(OxideQLoadEvent::TypeStarted),
        error_domain(OxideQLoadEvent::ErrorDomainNone),
        error_code(0),
        http_status_code(0),
        is_error(false) {}

  QUrl url;
  OxideQLoadEvent::Type type;
  OxideQLoadEvent::ErrorDomain error_domain;
  QString error_string;
  int error_code;
  int http_status_code;
  QUrl original_url;
  bool is_error;
};

/*!
\class OxideQLoadEvent
\inheaderfile oxideqloadevent.h
\inmodule OxideQtCore

\brief Represents a load event

OxideQLoadEvent represents a load event. Load events are associated with a
\l{url}, and have a \l{type}.

For load events where the \l{type} is TypeFailed, the event will provide
details of the error via errorDomain, errorString and errorCode.

For load events where the \l{type} is TypeCommitted, TypeSucceeded or
TypeRedirected, and \l{url} is a http: or https: URL, httpStatusCode will
indicate the HTTP status code returned from the server. Load events where
\l{type} is TypeFailed may also provide the HTTP status code if the failure
is post-commit.

Load events where \l{type} is TypeRedirected will provide the original URL
via originalUrl.
*/

/*!
\enum OxideQLoadEvent::Type

This enum represents the type of a load event

\value TypeStarted
A started event is the first event in a load event sequence. It occurs before
any request is sent over the network. At this point, the previous document still
exists.

\value TypeStopped
A stopped event occurs when the load is stopped by the application, a
non-overridable certificate error occurs in the main frame, a certificate
error is denied by the application, or the load is cancelled by some other
mechanism.

\value TypeSucceeded
A succeeded event occurs when a load completes successfully.

\value TypeFailed
A failed event occurs when a load fails.

\value TypeCommitted
A committed event occurs when a response is received from the remote server.
At this point, the new document has replaced the previous document.

\value TypeRedirected
A redirected event occurs when the server responds with a 3xx response.
*/

/*!
\enum OxideQLoadEvent::ErrorDomain

This enum represents the error category for a load event

\value ErrorDomainNone
No error.

\value ErrorDomainInternal
An internal error occurred.

\value ErrorDomainConnection
A connection error occurred, such as a SSL error, TCP protocol error or name
resolution failure

\value ErrorDomainCertificate
A certificate error occurred, such as the server responding with a certificate
whose common name is invalid, is expired or signed by an authority that isn't
trusted. This error won't be seen for main frame loads.

\value ErrorDomainHTTP
A HTTP error occurred, such as a redirect loop or other invalid HTTP response.

\value ErrorDomainCache
A cache error occurred

\value ErrorDomainFTP
A FTP error occurred

\value ErrorDomainDNS
An error occurred during name resolution, such as a timeout or invalid response
*/

OxideQLoadEvent::OxideQLoadEvent(
    const QSharedDataPointer<OxideQLoadEventData>& dd)
    : d(dd) {}

// static
OxideQLoadEvent OxideQLoadEvent::createStarted(const QUrl& url) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeStarted;

  return OxideQLoadEvent(data);
}

// static
OxideQLoadEvent OxideQLoadEvent::createStopped(const QUrl& url) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeStopped;

  return OxideQLoadEvent(data);
}

// static
OxideQLoadEvent OxideQLoadEvent::createSucceeded(const QUrl& url,
                                                 int http_status_code) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeSucceeded;
  data->http_status_code = http_status_code;

  return OxideQLoadEvent(data);
}

// static
OxideQLoadEvent OxideQLoadEvent::createFailed(const QUrl& url,
                                              ErrorDomain error_domain,
                                              const QString& error_string,
                                              int error_code,
                                              int http_status_code) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeFailed;
  data->error_domain = error_domain;
  data->error_string = error_string;
  data->error_code = error_code;
  data->http_status_code = http_status_code;

  return OxideQLoadEvent(data);
}

// static
OxideQLoadEvent OxideQLoadEvent::createCommitted(const QUrl& url,
                                                 bool is_error,
                                                 int http_status_code) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeCommitted;
  data->http_status_code = http_status_code;
  data->is_error = is_error;

  return OxideQLoadEvent(data);
}

// static
OxideQLoadEvent OxideQLoadEvent::createRedirected(const QUrl& url,
                                                  const QUrl& original_url,
                                                  int http_status_code) {
  QSharedDataPointer<OxideQLoadEventData> data(new OxideQLoadEventData());

  data->url = url;
  data->type = TypeRedirected;
  data->http_status_code = http_status_code;
  data->original_url = original_url;

  return OxideQLoadEvent(data);
}

/*!
\internal
*/

OxideQLoadEvent::OxideQLoadEvent()
    : d(new OxideQLoadEventData()) {}

/*!
Copy constructs a load event from \a other.
*/

OxideQLoadEvent::OxideQLoadEvent(const OxideQLoadEvent& other)
    : d(other.d) {}

/*!
Destroys the load event.
*/

OxideQLoadEvent::~OxideQLoadEvent() {}

/*!
Assigns \a other to this load event.
*/

OxideQLoadEvent OxideQLoadEvent::operator=(const OxideQLoadEvent& other) {
  d = other.d;
  return *this;
}

/*!
Returns true if this load event equals \a other. A load event will only be equal
to one that it was copied from.
*/

bool OxideQLoadEvent::operator==(const OxideQLoadEvent& other) const {
  return d == other.d;
}


/*!
Returns true if this load event does not equal \a other.
*/

bool OxideQLoadEvent::operator!=(const OxideQLoadEvent& other) const {
  return !(*this == other);
}

/*!
Returns the url associated with this load event.
*/

QUrl OxideQLoadEvent::url() const {
  return d->url;
}

/*!
Returns the type of this load event.
*/

OxideQLoadEvent::Type OxideQLoadEvent::type() const {
  return d->type;
}

/*!
Returns the error domain for this load event. If \l{type} is not TypeFailed,
this will return ErrorDomainNone.

\sa errorString, errorCode
*/

OxideQLoadEvent::ErrorDomain OxideQLoadEvent::errorDomain() const {
  return d->error_domain;
}

/*!
Returns a description of the error for this load event, suitable for display
in an application UI. If \l{type} is not TypeFailed, this will return an
empty string.

\sa errorDomain, errorCode
*/

QString OxideQLoadEvent::errorString() const {
  return d->error_string;
}

/*!
Returns the error code for this load event. The error code matches the internal
code provided by the Chromium networking stack.

Applications shouldn't make any assumption about the meaning of these error
codes, and shouldn't assume that specific errors will produce the same error
code in future releases of Oxide.

\sa errorDomain, errorString
*/

int OxideQLoadEvent::errorCode() const {
  return d->error_code;
}

/*!
Returns the HTTP status code returned from the remote server for this load.
This will return 0 for load events where \l{type} is TypeStarted or
TypeStopped, or \l{url} is not a http: or https: URL. It will also return 0
for load events where \l{type} is TypeFailed and the load hasn't committed
yet.
*/

int OxideQLoadEvent::httpStatusCode() const {
  return d->http_status_code;
}

/*!
Returns the original URL for load events where \l{type} is TypeRedirected.
For all other load events, this returns an empty URL.
*/

QUrl OxideQLoadEvent::originalUrl() const {
  return d->original_url;
}

/*!
Returns true if the load event is associated with an error page. If a load
fails, Chromium will proceed to load a (currently empty) error page.
*/

bool OxideQLoadEvent::isError() const {
  return d->is_error;
}
