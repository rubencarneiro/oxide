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

#ifndef OXIDE_Q_MEDIA_CAPTURE_DEVICE
#define OXIDE_Q_MEDIA_CAPTURE_DEVICE

#include <QList>
#include <QSharedDataPointer>
#include <QString>
#include <QtGlobal>

class OxideQAudioCaptureDeviceData;
class OxideQVideoCaptureDeviceData;

class Q_DECL_EXPORT OxideQAudioCaptureDevice {
 public:
  static QList<OxideQAudioCaptureDevice> availableDevices();

  OxideQAudioCaptureDevice(const OxideQAudioCaptureDevice& other);
  ~OxideQAudioCaptureDevice();

  QString id() const;

  QString displayName() const;

 private:
  Q_DECL_HIDDEN OxideQAudioCaptureDevice(const QString& id,
                                         const QString& name);

  QSharedDataPointer<OxideQAudioCaptureDeviceData> d;
};

class Q_DECL_EXPORT OxideQVideoCaptureDevice {
 public:
  static QList<OxideQVideoCaptureDevice> availableDevices();

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
  Q_DECL_HIDDEN OxideQVideoCaptureDevice(const QString& id,
                                         const QString& name,
                                         Position position);

  QSharedDataPointer<OxideQVideoCaptureDeviceData> d;
};

#endif // OXIDE_Q_MEDIA_CAPTURE_DEVICE
