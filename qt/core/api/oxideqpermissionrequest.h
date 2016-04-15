// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef OXIDE_QTCORE_PERMISSION_REQUEST
#define OXIDE_QTCORE_PERMISSION_REQUEST

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>

#include <OxideQtCore/oxideqglobal.h>

class OxideQGeolocationPermissionRequestPrivate;
class OxideQMediaAccessPermissionRequestPrivate;
class OxideQPermissionRequestPrivate;

class OXIDE_QTCORE_EXPORT OxideQPermissionRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl origin READ origin CONSTANT)
  Q_PROPERTY(QUrl embedder READ embedder CONSTANT)
  Q_PROPERTY(bool isCancelled READ isCancelled NOTIFY cancelled)

  // Same as origin - here for legacy purposes
  Q_PROPERTY(QUrl url READ url CONSTANT)

  Q_DECLARE_PRIVATE(OxideQPermissionRequest)
  Q_DISABLE_COPY(OxideQPermissionRequest)

 public:
  ~OxideQPermissionRequest() Q_DECL_OVERRIDE;

  QUrl origin() const;
  QUrl embedder() const;

  QUrl url() const;

  bool isCancelled() const;

 public Q_SLOTS:
  void allow();
  void deny();

 Q_SIGNALS:
  void cancelled();

 protected:
  OxideQPermissionRequest(OxideQPermissionRequestPrivate& dd);

  QScopedPointer<OxideQPermissionRequestPrivate> d_ptr;
};

class OXIDE_QTCORE_EXPORT OxideQGeolocationPermissionRequest
    : public OxideQPermissionRequest {
  Q_OBJECT

  Q_DECLARE_PRIVATE(OxideQGeolocationPermissionRequest)
  Q_DISABLE_COPY(OxideQGeolocationPermissionRequest)

 public:
  ~OxideQGeolocationPermissionRequest() Q_DECL_OVERRIDE;

 public Q_SLOTS:
  // This has been replaced by allow(). With hindsight, allow/deny always made
  // more sense
  void accept();

 private:
  OxideQGeolocationPermissionRequest(
      OxideQGeolocationPermissionRequestPrivate& dd);
};

class OXIDE_QTCORE_EXPORT OxideQMediaAccessPermissionRequest
    : public OxideQPermissionRequest {
  Q_OBJECT

  Q_PROPERTY(bool isForAudio READ isForAudio CONSTANT)
  Q_PROPERTY(bool isForVideo READ isForVideo CONSTANT)

  Q_PROPERTY(QString requestedAudioDeviceId READ requestedAudioDeviceId CONSTANT REVISION 1)
  Q_PROPERTY(QString requestedVideoDeviceId READ requestedVideoDeviceId CONSTANT REVISION 1)

  Q_DECLARE_PRIVATE(OxideQMediaAccessPermissionRequest)
  Q_DISABLE_COPY(OxideQMediaAccessPermissionRequest)

 public:
  ~OxideQMediaAccessPermissionRequest() Q_DECL_OVERRIDE;

  bool isForAudio() const;
  bool isForVideo() const;
  QString requestedAudioDeviceId() const;
  QString requestedVideoDeviceId() const;

 private:
  OxideQMediaAccessPermissionRequest(
      OxideQMediaAccessPermissionRequestPrivate& dd);
};

#endif // OXIDE_QTCORE_PERMISSION_REQUEST
