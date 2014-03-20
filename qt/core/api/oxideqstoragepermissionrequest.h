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

#ifndef OXIDE_Q_STORAGE_PERMISSION_REQUEST
#define OXIDE_Q_STORAGE_PERMISSION_REQUEST

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQStoragePermissionRequestPrivate;

class Q_DECL_EXPORT OxideQStoragePermissionRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(QUrl firstPartyUrl READ firstPartyUrl CONSTANT)
  Q_PROPERTY(bool write READ write CONSTANT)
  Q_PROPERTY(Type type READ type CONSTANT)

  Q_ENUMS(Type)

  Q_DECLARE_PRIVATE(OxideQStoragePermissionRequest)
  Q_DISABLE_COPY(OxideQStoragePermissionRequest)

 public:

  enum Type {
    TypeCookies,
    TypeAppCache,
    TypeLocalStorage,
    TypeSessionStorage,
    TypeIndexedDB,
    TypeWebDB
  };

  Q_DECL_HIDDEN OxideQStoragePermissionRequest(const QUrl& url,
                                               const QUrl& first_party_url,
                                               bool write,
                                               Type type);
  virtual ~OxideQStoragePermissionRequest();

  QUrl url() const;
  QUrl firstPartyUrl() const;
  bool write() const;
  Type type() const;

  Q_INVOKABLE void accept();
  Q_INVOKABLE void deny();

 private:
  QScopedPointer<OxideQStoragePermissionRequestPrivate> d_ptr;
};

#endif // OXIDE_Q_STORAGE_PERMISSION_REQUEST
