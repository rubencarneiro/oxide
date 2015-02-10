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

#include "oxideqloadevent.h"

#include "base/logging.h"

class OxideQLoadEventPrivate {
 public:
  ~OxideQLoadEventPrivate() {}
  OxideQLoadEventPrivate()
      : type(OxideQLoadEvent::TypeStarted),
        error_domain(OxideQLoadEvent::ErrorDomainNone),
        error_code(0),
        is_error(false) {}

  QUrl url;
  OxideQLoadEvent::Type type;
  OxideQLoadEvent::ErrorDomain error_domain;
  QString error_string;
  int error_code;
  QUrl original_url;
  bool is_error;
};

OxideQLoadEvent::OxideQLoadEvent(const QUrl& url,
                                 Type type,
                                 bool is_error)
    : d_ptr(new OxideQLoadEventPrivate()) {
  Q_D(OxideQLoadEvent);

  DCHECK(type != OxideQLoadEvent::TypeFailed &&
         type != OxideQLoadEvent::TypeRedirected);
  DCHECK(type == OxideQLoadEvent::TypeCommitted || !is_error);

  d->url = url;
  d->type = type;
  d->is_error = is_error;
}

OxideQLoadEvent::OxideQLoadEvent(const QUrl& url,
                                 ErrorDomain error_domain,
                                 const QString& error_string,
                                 int error_code)
    : d_ptr(new OxideQLoadEventPrivate()) {
  Q_D(OxideQLoadEvent);

  d->url = url;
  d->type = OxideQLoadEvent::TypeFailed;
  d->error_domain = error_domain;
  d->error_string = error_string;
  d->error_code = error_code;
}

OxideQLoadEvent::OxideQLoadEvent(const QUrl& url,
                                 const QUrl& original_url)
    : d_ptr(new OxideQLoadEventPrivate()) {
  Q_D(OxideQLoadEvent);

  d->url = url;
  d->type = OxideQLoadEvent::TypeRedirected;
  d->original_url = original_url;
}

OxideQLoadEvent::~OxideQLoadEvent() {}

QUrl OxideQLoadEvent::url() const {
  Q_D(const OxideQLoadEvent);

  return d->url;
}

OxideQLoadEvent::Type OxideQLoadEvent::type() const {
  Q_D(const OxideQLoadEvent);

  return d->type;
}

OxideQLoadEvent::ErrorDomain OxideQLoadEvent::errorDomain() const {
  Q_D(const OxideQLoadEvent);

  return d->error_domain;
}

QString OxideQLoadEvent::errorString() const {
  Q_D(const OxideQLoadEvent);

  return d->error_string;
}

int OxideQLoadEvent::errorCode() const {
  Q_D(const OxideQLoadEvent);

  return d->error_code;
}

QUrl OxideQLoadEvent::originalUrl() const {
  Q_D(const OxideQLoadEvent);

  return d->original_url;
}

bool OxideQLoadEvent::isError() const {
  Q_D(const OxideQLoadEvent);

  return d->is_error;
}
