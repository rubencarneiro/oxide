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

#include "oxideqloadevent_p.h"

#include "net/base/net_errors.h"

namespace oxide {
namespace qt {

QLoadEventPrivate::QLoadEventPrivate(const QUrl& url,
                                     OxideQLoadEvent::Type type,
                                     OxideQLoadEvent::ErrorCode error,
                                     const QString& error_description) :
    url_(url),
    type_(type),
    error_(error),
    error_string_(error_description) {}

// static
OxideQLoadEvent::ErrorCode
QLoadEventPrivate::ChromeErrorCodeToOxideErrorCode(int error_code) {
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

// static
QLoadEventPrivate* QLoadEventPrivate::Create(
    const QUrl& url,
    OxideQLoadEvent::Type type,
    int error_code,
    const QString& error_description) {
  return new QLoadEventPrivate(url, type,
                               ChromeErrorCodeToOxideErrorCode(error_code),
                               error_description);
}

QUrl QLoadEventPrivate::url() const {
  return url_;
}

OxideQLoadEvent::Type QLoadEventPrivate::type() const {
  return type_;
}

OxideQLoadEvent::ErrorCode QLoadEventPrivate::error() const {
  return error_;
}

QString QLoadEventPrivate::errorString() const {
  return error_string_;
}

} // namespace qt
} // namespace oxide
