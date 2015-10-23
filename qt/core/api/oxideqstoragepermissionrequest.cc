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

#include "oxideqstoragepermissionrequest.h"
#include "oxideqstoragepermissionrequest_p.h"

#include "base/macros.h"

OxideQStoragePermissionRequestPrivate::OxideQStoragePermissionRequestPrivate(
    const QUrl& url,
    const QUrl& first_party_url,
    bool write,
    OxideQStoragePermissionRequest::Type type) :
    permission(oxide::STORAGE_PERMISSION_UNDEFINED),
    url_(url),
    first_party_url_(first_party_url),
    write_(write),
    type_(type) {}

OxideQStoragePermissionRequestPrivate::~OxideQStoragePermissionRequestPrivate() {}

// static
OxideQStoragePermissionRequestPrivate*
OxideQStoragePermissionRequestPrivate::get(OxideQStoragePermissionRequest* q) {
  return q->d_func();
}

OxideQStoragePermissionRequest::OxideQStoragePermissionRequest(
    const QUrl& url,
    const QUrl& first_party_url,
    bool write,
    Type type) :
    d_ptr(new OxideQStoragePermissionRequestPrivate(url, first_party_url,
                                                    write, type)) {
  static_assert(
      TypeCookies == static_cast<Type>(oxide::STORAGE_TYPE_COOKIES),
      "Type and oxide::StorageType enums don't match: TypeCookies");
  static_assert(
      TypeAppCache == static_cast<Type>(oxide::STORAGE_TYPE_APPCACHE),
      "Type and oxide::StorageType enums don't match: TypeAppCache");
  static_assert(
      TypeLocalStorage == static_cast<Type>(oxide::STORAGE_TYPE_LOCAL_STORAGE),
      "Type and oxide::StorageType enums don't match: TypeLocalStorage");
  static_assert(
      TypeSessionStorage == static_cast<Type>(oxide::STORAGE_TYPE_SESSION_STORAGE),
      "Type and oxide::StorageType enums don't match: TypeSessionStorage");
  static_assert(
      TypeIndexedDB == static_cast<Type>(oxide::STORAGE_TYPE_INDEXEDDB),
      "Type and oxide::StorageType enums don't match: TypeIndexedDB");
  static_assert(
      TypeWebDB == static_cast<Type>(oxide::STORAGE_TYPE_WEBDB),
      "Type and oxide::StorageType enums don't match: TypeWebDB");
}

OxideQStoragePermissionRequest::~OxideQStoragePermissionRequest() {}

QUrl OxideQStoragePermissionRequest::url() const {
  Q_D(const OxideQStoragePermissionRequest);

  return d->url_;
}

QUrl OxideQStoragePermissionRequest::firstPartyUrl() const {
  Q_D(const OxideQStoragePermissionRequest);

  return d->first_party_url_;
}

bool OxideQStoragePermissionRequest::write() const {
  Q_D(const OxideQStoragePermissionRequest);

  return d->write_;
}

OxideQStoragePermissionRequest::Type
OxideQStoragePermissionRequest::type() const {
  Q_D(const OxideQStoragePermissionRequest);

  return d->type_;
}

void OxideQStoragePermissionRequest::accept() {
  Q_D(OxideQStoragePermissionRequest);

  d->permission = oxide::STORAGE_PERMISSION_ALLOW;
}

void OxideQStoragePermissionRequest::deny() {
  Q_D(OxideQStoragePermissionRequest);

  d->permission = oxide::STORAGE_PERMISSION_DENY;
} 
