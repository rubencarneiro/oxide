// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef OXIDE_Q_PERMISSION_REQUEST
#define OXIDE_Q_PERMISSION_REQUEST

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

class OxideQGeolocationPermissionRequestPrivate;
class OxideQPermissionRequestPrivate;

class Q_DECL_EXPORT OxideQPermissionRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl origin READ origin CONSTANT)
  Q_PROPERTY(QUrl embedder READ embedder CONSTANT)
  Q_PROPERTY(bool isCancelled READ isCancelled NOTIFY cancelled)

  Q_DECLARE_PRIVATE(OxideQPermissionRequest)
  Q_DISABLE_COPY(OxideQPermissionRequest)

 public:
  virtual ~OxideQPermissionRequest();

  QUrl origin() const;
  QUrl embedder() const;

  bool isCancelled() const;

 Q_SIGNALS:
  void cancelled();

 protected:
  Q_DECL_HIDDEN OxideQPermissionRequest(OxideQPermissionRequestPrivate& dd);

  QScopedPointer<OxideQPermissionRequestPrivate> d_ptr;
};

class Q_DECL_EXPORT OxideQGeolocationPermissionRequest :
    public OxideQPermissionRequest {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQGeolocationPermissionRequest)
  Q_DISABLE_COPY(OxideQGeolocationPermissionRequest)

 public:
  Q_DECL_HIDDEN OxideQGeolocationPermissionRequest();
  ~OxideQGeolocationPermissionRequest();

 public Q_SLOTS:
  void accept();
  void deny();
};

#endif // OXIDE_Q_PERMISSION_REQUEST
