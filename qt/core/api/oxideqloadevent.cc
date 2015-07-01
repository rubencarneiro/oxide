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

OxideQLoadEvent::OxideQLoadEvent(
    const QSharedDataPointer<OxideQLoadEventData>& dd)
    : d(dd) {}

OxideQLoadEvent::OxideQLoadEvent()
    : d(new OxideQLoadEventData()) {}

OxideQLoadEvent::OxideQLoadEvent(const OxideQLoadEvent& other)
    : d(other.d) {}

OxideQLoadEvent::~OxideQLoadEvent() {}

OxideQLoadEvent OxideQLoadEvent::operator=(const OxideQLoadEvent& other) {
  d = other.d;
  return *this;
}

bool OxideQLoadEvent::operator==(const OxideQLoadEvent& other) const {
  return d == other.d;
}

bool OxideQLoadEvent::operator!=(const OxideQLoadEvent& other) const {
  return !(*this == other);
}

QUrl OxideQLoadEvent::url() const {
  return d->url;
}

OxideQLoadEvent::Type OxideQLoadEvent::type() const {
  return d->type;
}

OxideQLoadEvent::ErrorDomain OxideQLoadEvent::errorDomain() const {
  return d->error_domain;
}

QString OxideQLoadEvent::errorString() const {
  return d->error_string;
}

int OxideQLoadEvent::errorCode() const {
  return d->error_code;
}

int OxideQLoadEvent::httpStatusCode() const {
  return d->http_status_code;
}

QUrl OxideQLoadEvent::originalUrl() const {
  return d->original_url;
}

bool OxideQLoadEvent::isError() const {
  return d->is_error;
}
