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
class OxideQSimplePermissionRequestPrivate;

class Q_DECL_EXPORT OxideQPermissionRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(QUrl embedder READ embedder CONSTANT)
  Q_PROPERTY(bool isCancelled READ isCancelled NOTIFY cancelled)

  Q_DECLARE_PRIVATE(OxideQPermissionRequest)
  Q_DISABLE_COPY(OxideQPermissionRequest)

 public:
  virtual ~OxideQPermissionRequest();

  QUrl url() const;
  QUrl embedder() const;

  bool isCancelled() const;

 Q_SIGNALS:
  void cancelled();

 protected:
  OxideQPermissionRequest(OxideQPermissionRequestPrivate& dd);

  QScopedPointer<OxideQPermissionRequestPrivate> d_ptr;
};

class Q_DECL_EXPORT OxideQSimplePermissionRequest :
    public OxideQPermissionRequest {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQSimplePermissionRequest)
  Q_DISABLE_COPY(OxideQSimplePermissionRequest)

 public:
  virtual ~OxideQSimplePermissionRequest();

 public Q_SLOTS:
  void allow();
  void deny();

 protected:
  OxideQSimplePermissionRequest(OxideQSimplePermissionRequestPrivate& dd);
};

class Q_DECL_EXPORT OxideQGeolocationPermissionRequest Q_DECL_FINAL :
    public OxideQSimplePermissionRequest {
  Q_OBJECT

  // This has been replaced by url. origin made sense for geolocation
  // because we only get an origin from Chromium, whereas we get a full URL
  // for other types of request
  Q_PROPERTY(QUrl origin READ origin CONSTANT)

  Q_DECLARE_PRIVATE(OxideQGeolocationPermissionRequest)
  Q_DISABLE_COPY(OxideQGeolocationPermissionRequest)

 public:
  ~OxideQGeolocationPermissionRequest();

  QUrl origin() const;

 public Q_SLOTS:
  // This has been replaced by allow(). With hindsight, allow/deny always made
  // more sense
  void accept();

 private:
  OxideQGeolocationPermissionRequest(
      OxideQGeolocationPermissionRequestPrivate& dd);
};

#endif // OXIDE_Q_PERMISSION_REQUEST
