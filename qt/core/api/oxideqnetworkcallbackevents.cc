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

#include "oxideqnetworkcallbackevents.h"
#include "oxideqnetworkcallbackevents_p.h"

#include "net/http/http_request_headers.h"
#include "url/gurl.h"

OxideQNetworkCallbackEventPrivate::OxideQNetworkCallbackEventPrivate(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame) :
    request_cancelled(NULL),
    url_(url),
    method_(method),
    referrer_(referrer),
    is_main_frame_(isMainFrame) {}

OxideQNetworkCallbackEventPrivate::~OxideQNetworkCallbackEventPrivate() {}

OxideQBeforeURLRequestEventPrivate::OxideQBeforeURLRequestEventPrivate(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame) :
    OxideQNetworkCallbackEventPrivate(url, method, referrer, isMainFrame),
    new_url(NULL) {}

OxideQBeforeURLRequestEventPrivate::~OxideQBeforeURLRequestEventPrivate() {}

// static
OxideQBeforeURLRequestEventPrivate* OxideQBeforeURLRequestEventPrivate::get(
    OxideQBeforeURLRequestEvent* q) {
  return q->d_func();
}

OxideQBeforeSendHeadersEventPrivate::OxideQBeforeSendHeadersEventPrivate(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame) :
    OxideQNetworkCallbackEventPrivate(url, method, referrer, isMainFrame),
    headers(NULL) {}

OxideQBeforeSendHeadersEventPrivate::~OxideQBeforeSendHeadersEventPrivate() {}

// static
OxideQBeforeSendHeadersEventPrivate* OxideQBeforeSendHeadersEventPrivate::get(
    OxideQBeforeSendHeadersEvent* q) {
  return q->d_func();
}

OxideQBeforeRedirectEventPrivate::OxideQBeforeRedirectEventPrivate(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame,
    const QUrl& redirectionChainPreviousUrlEntry) :
    OxideQNetworkCallbackEventPrivate(url, method, referrer, isMainFrame),
    previous_url_(redirectionChainPreviousUrlEntry) {}

OxideQBeforeRedirectEventPrivate::~OxideQBeforeRedirectEventPrivate() {}

// static
OxideQBeforeRedirectEventPrivate* OxideQBeforeRedirectEventPrivate::get(
    OxideQBeforeRedirectEvent* q) {
  return q->d_func();
}

OxideQNetworkCallbackEvent::OxideQNetworkCallbackEvent(
    OxideQNetworkCallbackEventPrivate& dd) :
    d_ptr(&dd) {}

OxideQNetworkCallbackEvent::~OxideQNetworkCallbackEvent() {}

QUrl OxideQNetworkCallbackEvent::url() const {
  Q_D(const OxideQNetworkCallbackEvent);

  return d->url_;
}

QString OxideQNetworkCallbackEvent::method() const {
  Q_D(const OxideQNetworkCallbackEvent);

  return d->method_;
}

bool OxideQNetworkCallbackEvent::isMainFrame() const {
  Q_D(const OxideQNetworkCallbackEvent);

  return d->is_main_frame_;
}

QString OxideQNetworkCallbackEvent::referrer() const {
  Q_D(const OxideQNetworkCallbackEvent);

  return d->referrer_;
}

bool OxideQNetworkCallbackEvent::requestCancelled() const {
  Q_D(const OxideQNetworkCallbackEvent);

  if (!d->request_cancelled) {
    return false;
  }

  return *(d->request_cancelled);
}

void OxideQNetworkCallbackEvent::cancelRequest() {
  Q_D(OxideQNetworkCallbackEvent);

  if (!d->request_cancelled) {
    return;
  }

  *(d->request_cancelled) = true;
}

OxideQBeforeURLRequestEvent::OxideQBeforeURLRequestEvent(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame) :
    OxideQNetworkCallbackEvent(
        *new OxideQBeforeURLRequestEventPrivate(
            url, method, referrer, isMainFrame)) {}

OxideQBeforeURLRequestEvent::~OxideQBeforeURLRequestEvent() {}

QUrl OxideQBeforeURLRequestEvent::redirectUrl() const {
  Q_D(const OxideQBeforeURLRequestEvent);

  if (!d->new_url) {
    return QUrl();
  }

  return QUrl(QString::fromStdString(d->new_url->spec()));
}

void OxideQBeforeURLRequestEvent::setRedirectUrl(const QUrl& url) {
  Q_D(OxideQBeforeURLRequestEvent);

  if (!d->new_url) {
    return;
  }

  *(d->new_url) = GURL(url.toString().toStdString());
}

OxideQBeforeSendHeadersEvent::OxideQBeforeSendHeadersEvent(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame) :
    OxideQNetworkCallbackEvent(
	*new OxideQBeforeSendHeadersEventPrivate(
            url, method, referrer, isMainFrame)) {}

OxideQBeforeSendHeadersEvent::~OxideQBeforeSendHeadersEvent() {}

bool OxideQBeforeSendHeadersEvent::hasHeader(const QString& header) const {
  Q_D(const OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return false;
  }

  return d->headers->HasHeader(header.toStdString());
}

QString OxideQBeforeSendHeadersEvent::getHeader(const QString& header) const {
  Q_D(const OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return QString();
  }

  std::string value;
  d->headers->GetHeader(header.toStdString(), &value);
  return QString::fromStdString(value);
}

void OxideQBeforeSendHeadersEvent::setHeader(const QString& header,
                                             const QString& value) {
  Q_D(OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return;
  }

  d->headers->SetHeader(header.toStdString(), value.toStdString());
}

void OxideQBeforeSendHeadersEvent::setHeaderIfMissing(
    const QString& header,
    const QString& value) {
  Q_D(OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return;
  }

  d->headers->SetHeaderIfMissing(header.toStdString(), value.toStdString());
}

void OxideQBeforeSendHeadersEvent::clearHeaders() {
  Q_D(OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return;
  }

  d->headers->Clear();
}

void OxideQBeforeSendHeadersEvent::removeHeader(const QString& header) {
  Q_D(OxideQBeforeSendHeadersEvent);

  if (!d->headers) {
    return;
  }

  d->headers->RemoveHeader(header.toStdString());
}

OxideQBeforeRedirectEvent::OxideQBeforeRedirectEvent(
    const QUrl& url,
    const QString& method,
    const QString& referrer,
    bool isMainFrame,
    const QUrl& redirectionChainPreviousUrlEntry) :
    OxideQNetworkCallbackEvent(
       *new OxideQBeforeRedirectEventPrivate(
           url, method, referrer,
           isMainFrame, redirectionChainPreviousUrlEntry)) {}

OxideQBeforeRedirectEvent::~OxideQBeforeRedirectEvent() {}

QUrl OxideQBeforeRedirectEvent::redirectionChainPreviousUrlEntry() const {
  Q_D(const OxideQBeforeRedirectEvent);

  return d->previous_url_;
}

