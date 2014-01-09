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
#include "oxideqloadevent_p.h"

#include "net/base/net_errors.h"

OxideQLoadEventPrivate::OxideQLoadEventPrivate(
    QUrl url,
    OxideQLoadEvent::Type type,
    OxideQLoadEvent::ErrorCode error,
    QString error_string) :
    url(url), type(type), error(error), error_string(error_string) {}

// static
OxideQLoadEvent::ErrorCode
OxideQLoadEventPrivate::ChromeErrorCodeToOxideErrorCode(int error_code) {
  switch (error_code) {
    case 0:
      return OxideQLoadEvent::ErrorNone;
    case net::ERR_UNEXPECTED:
      return OxideQLoadEvent::ErrorUnexpected;
    case net::ERR_NAME_NOT_RESOLVED:
      return OxideQLoadEvent::ErrorNameNotResolved;
    default:
      return OxideQLoadEvent::ErrorFailed;
  }
}

OxideQLoadEvent::OxideQLoadEvent(const QUrl& url,
                                 Type type,
                                 ErrorCode error_code,
                                 const QString& error_description) :
    d_ptr(new OxideQLoadEventPrivate(
        url, type, error_code, error_description)) {}

OxideQLoadEvent::~OxideQLoadEvent() {}

QUrl OxideQLoadEvent::url() const {
  Q_D(const OxideQLoadEvent);

  return d->url;
}

OxideQLoadEvent::Type OxideQLoadEvent::type() const {
  Q_D(const OxideQLoadEvent);

  return d->type;
}

OxideQLoadEvent::ErrorCode OxideQLoadEvent::error() const {
  Q_D(const OxideQLoadEvent);

  return d->error;
}

QString OxideQLoadEvent::errorString() const {
  Q_D(const OxideQLoadEvent);

  return d->error_string;
}
