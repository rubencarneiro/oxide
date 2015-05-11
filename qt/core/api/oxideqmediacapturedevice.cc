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

#include "oxideqmediacapturedevice.h"

#include "content/public/browser/media_capture_devices.h"
#include "content/public/common/media_stream_request.h"

namespace {

OxideQVideoCaptureDevice::Position VideoFacingModeToPosition(
    content::VideoFacingMode mode) {
  switch (mode) {
    case content::MEDIA_VIDEO_FACING_USER:
      return OxideQVideoCaptureDevice::PositionFrontFace;
    case content::MEDIA_VIDEO_FACING_ENVIRONMENT:
      return OxideQVideoCaptureDevice::PositionBackFace;
    default:
      return OxideQVideoCaptureDevice::PositionUnspecified;
  }
}

}

class OxideQAudioCaptureDeviceData : public QSharedData {
 public:
  OxideQAudioCaptureDeviceData(const QString& id,
                               const QString& name)
      : id(id), name(name) {}

  QString id;
  QString name;
};

class OxideQVideoCaptureDeviceData : public QSharedData {
 public:
  OxideQVideoCaptureDeviceData(const QString& id,
                               const QString& name,
                               OxideQVideoCaptureDevice::Position position)
      : id(id), name(name), position(position) {}

  QString id;
  QString name;
  OxideQVideoCaptureDevice::Position position;
};

OxideQAudioCaptureDevice::OxideQAudioCaptureDevice(const QString& id,
                                                   const QString& name)
    : d(new OxideQAudioCaptureDeviceData(id, name)) {}

// static
QList<OxideQAudioCaptureDevice> OxideQAudioCaptureDevice::availableDevices() {
  const content::MediaStreamDevices& devices =
      content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();

  QList<OxideQAudioCaptureDevice> rv;
  for (auto it = devices.begin(); it != devices.end(); ++it) {
    const content::MediaStreamDevice& device = *it;
    if (device.type != content::MEDIA_DEVICE_AUDIO_CAPTURE) {
      continue;
    }

    rv << OxideQAudioCaptureDevice(QString::fromStdString(device.id),
                                   QString::fromStdString(device.name));
  }

  return rv;
}

OxideQAudioCaptureDevice::OxideQAudioCaptureDevice(
    const OxideQAudioCaptureDevice& other)
    : d(other.d) {}

OxideQAudioCaptureDevice::~OxideQAudioCaptureDevice() {}

QString OxideQAudioCaptureDevice::id() const {
  return d->id;
}

QString OxideQAudioCaptureDevice::displayName() const {
  return d->name;
}

OxideQVideoCaptureDevice::OxideQVideoCaptureDevice(const QString& id,
                                                   const QString& name,
                                                   Position position)
    : d(new OxideQVideoCaptureDeviceData(id, name, position)) {}

// static
QList<OxideQVideoCaptureDevice> OxideQVideoCaptureDevice::availableDevices() {
  const content::MediaStreamDevices& devices =
      content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();

  QList<OxideQVideoCaptureDevice> rv;
  for (auto it = devices.begin(); it != devices.end(); ++it) {
    const content::MediaStreamDevice& device = *it;
    if (device.type != content::MEDIA_DEVICE_VIDEO_CAPTURE) {
      continue;
    }

    rv << OxideQVideoCaptureDevice(
        QString::fromStdString(device.id),
        QString::fromStdString(device.name),
        VideoFacingModeToPosition(device.video_facing));
  }

  return rv;
}

OxideQVideoCaptureDevice::OxideQVideoCaptureDevice(
    const OxideQVideoCaptureDevice& other)
    : d(other.d) {}

OxideQVideoCaptureDevice::~OxideQVideoCaptureDevice() {}

QString OxideQVideoCaptureDevice::id() const {
  return d->id;
}

QString OxideQVideoCaptureDevice::displayName() const {
  return d->name;
}

OxideQVideoCaptureDevice::Position OxideQVideoCaptureDevice::position() const {
  return d->position;
}
