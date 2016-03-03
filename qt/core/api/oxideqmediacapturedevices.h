// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef OXIDE_QTCORE_MEDIA_CAPTURE_DEVICES
#define OXIDE_QTCORE_MEDIA_CAPTURE_DEVICES

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include <OxideQtCore/oxideqglobal.h>

class OxideQAudioCaptureDeviceData;
class OxideQMediaCaptureDevices;
class OxideQMediaCaptureDevicesPrivate;
class OxideQVideoCaptureDeviceData;

class OXIDE_QTCORE_EXPORT OxideQAudioCaptureDevice {
 public:
  OxideQAudioCaptureDevice(const OxideQAudioCaptureDevice& other);
  ~OxideQAudioCaptureDevice();

  QString id() const;

  QString displayName() const;

 private:
  friend class OxideQMediaCaptureDevices;

  Q_DECL_HIDDEN OxideQAudioCaptureDevice(const QString& id,
                                         const QString& name);

  QSharedDataPointer<OxideQAudioCaptureDeviceData> d;
};

class OXIDE_QTCORE_EXPORT OxideQVideoCaptureDevice {
 public:
  enum Position {
    PositionUnspecified,
    PositionFrontFace,
    PositionBackFace
  };

  OxideQVideoCaptureDevice(const OxideQVideoCaptureDevice& other);
  ~OxideQVideoCaptureDevice();

  QString id() const;

  QString displayName() const;

  Position position() const;

 private:
  friend class OxideQMediaCaptureDevices;

  Q_DECL_HIDDEN OxideQVideoCaptureDevice(const QString& id,
                                         const QString& name,
                                         Position position);

  QSharedDataPointer<OxideQVideoCaptureDeviceData> d;
};

class OXIDE_QTCORE_EXPORT OxideQMediaCaptureDevices : public QObject {
  Q_OBJECT

  Q_DISABLE_COPY(OxideQMediaCaptureDevices)
  Q_DECLARE_PRIVATE(OxideQMediaCaptureDevices)

 public:
  static OxideQMediaCaptureDevices* instance();

  Q_DECL_HIDDEN OxideQMediaCaptureDevices();
  ~OxideQMediaCaptureDevices() Q_DECL_OVERRIDE;

  QList<OxideQAudioCaptureDevice> availableAudioDevices();
  QList<OxideQVideoCaptureDevice> availableVideoDevices();

 Q_SIGNALS:
  void availableAudioDevicesChanged();
  void availableVideoDevicesChanged();

 private:
  QScopedPointer<OxideQMediaCaptureDevicesPrivate> d_ptr;
};

#endif // OXIDE_QTCORE_MEDIA_CAPTURE_DEVICES
