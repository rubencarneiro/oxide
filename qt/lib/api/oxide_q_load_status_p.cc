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

#include "oxide_q_load_status_p.h"

#include "net/base/net_errors.h"

namespace oxide {
namespace qt {

QLoadStatusPrivate::QLoadStatusPrivate(const QUrl& url,
                                       OxideQLoadStatus::LoadStatus status,
                                       OxideQLoadStatus::ErrorCode error,
                                       const QString& error_description) :
    url_(url),
    status_(status),
    error_(error),
    error_string_(error_description) {}

// static
OxideQLoadStatus::ErrorCode
QLoadStatusPrivate::ChromeErrorCodeToOxideErrorCode(int error_code) {
  switch (error_code) {
    case 0:
      return OxideQLoadStatus::ErrorNone;
    case net::ERR_UNEXPECTED:
      return OxideQLoadStatus::ErrorUnexpected;
    case net::ERR_NAME_NOT_RESOLVED:
      return OxideQLoadStatus::ErrorNameNotResolved;
    default:
      return OxideQLoadStatus::ErrorFailed;
  }
}

// static
QLoadStatusPrivate* QLoadStatusPrivate::Create(
    const QUrl& url,
    OxideQLoadStatus::LoadStatus status,
    int error_code,
    const QString& error_description) {
  return new QLoadStatusPrivate(url, status,
                                ChromeErrorCodeToOxideErrorCode(error_code),
                                error_description);
}

QUrl QLoadStatusPrivate::url() const {
  return url_;
}

OxideQLoadStatus::LoadStatus QLoadStatusPrivate::status() const {
  return status_;
}

OxideQLoadStatus::ErrorCode QLoadStatusPrivate::error() const {
  return error_;
}

QString QLoadStatusPrivate::errorString() const {
  return error_string_;
}

} // namespace qt
} // namespace oxide
