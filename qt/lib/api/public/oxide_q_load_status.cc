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

#include "oxide_q_load_status.h"

#include "qt/lib/api/oxide_q_load_status_p.h"

OxideQLoadStatus::~OxideQLoadStatus() {}

QUrl OxideQLoadStatus::url() const {
  Q_D(const oxide::qt::QLoadStatus);

  return d->url();
}

OxideQLoadStatus::LoadStatus OxideQLoadStatus::status() const {
  Q_D(const oxide::qt::QLoadStatus);

  return d->status();
}

OxideQLoadStatus::ErrorCode OxideQLoadStatus::error() const {
  Q_D(const oxide::qt::QLoadStatus);

  return d->error();
}

QString OxideQLoadStatus::errorString() const {
  Q_D(const oxide::qt::QLoadStatus);

  return d->errorString();
}

OxideQLoadStatus::OxideQLoadStatus(const QUrl& url,
                                   LoadStatus status,
                                   int error_code,
                                   const QString& error_description) :
    QObject(),
    d_ptr(oxide::qt::QLoadStatusPrivate::Create(
        url, status, error_code, error_description)) {}
