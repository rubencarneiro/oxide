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

#include "oxideqmediacapturedevices.h"

#include <QGlobalStatic>

#include "base/macros.h"
#include "content/public/common/media_stream_request.h"

#include "shared/browser/media/oxide_media_capture_devices_dispatcher.h"
#include "shared/browser/media/oxide_media_capture_devices_dispatcher_observer.h"

namespace {

Q_GLOBAL_STATIC(OxideQMediaCaptureDevices, g_instance)

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

using oxide::MediaCaptureDevicesDispatcher;

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

class OxideQMediaCaptureDevicesPrivate :
    public oxide::MediaCaptureDevicesDispatcherObserver {
  Q_DECLARE_PUBLIC(OxideQMediaCaptureDevices)

 public:
  ~OxideQMediaCaptureDevicesPrivate() override {}

 private:
  OxideQMediaCaptureDevicesPrivate(OxideQMediaCaptureDevices* q)
      : q_ptr(q),
        audio_devices_need_update_(true),
        video_devices_need_update_(true) {}

  // oxide::MediaCaptureDevicesDispatcherObserver implementation
  void OnAudioCaptureDevicesChanged() override;
  void OnVideoCaptureDevicesChanged() override;

  OxideQMediaCaptureDevices* q_ptr;

  bool audio_devices_need_update_;
  QList<OxideQAudioCaptureDevice> audio_devices_;

  bool video_devices_need_update_;
  QList<OxideQVideoCaptureDevice> video_devices_;

  DISALLOW_COPY_AND_ASSIGN(OxideQMediaCaptureDevicesPrivate);
};

void OxideQMediaCaptureDevicesPrivate::OnAudioCaptureDevicesChanged() {
  Q_Q(OxideQMediaCaptureDevices);

  audio_devices_need_update_ = true;
  Q_EMIT q->availableAudioDevicesChanged();
}

void OxideQMediaCaptureDevicesPrivate::OnVideoCaptureDevicesChanged() {
  Q_Q(OxideQMediaCaptureDevices);

  video_devices_need_update_ = true;
  Q_EMIT q->availableVideoDevicesChanged();
}

OxideQAudioCaptureDevice::OxideQAudioCaptureDevice(const QString& id,
                                                   const QString& name)
    : d(new OxideQAudioCaptureDeviceData(id, name)) {}

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

OxideQMediaCaptureDevices::OxideQMediaCaptureDevices()
    : d_ptr(new OxideQMediaCaptureDevicesPrivate(this)) {}

OxideQMediaCaptureDevices::~OxideQMediaCaptureDevices() {}

// static
OxideQMediaCaptureDevices* OxideQMediaCaptureDevices::instance() {
  return g_instance();
}

QList<OxideQAudioCaptureDevice>
OxideQMediaCaptureDevices::availableAudioDevices() {
  Q_D(OxideQMediaCaptureDevices);

  if (!d->audio_devices_need_update_) {
    return d->audio_devices_;
  }

  d->audio_devices_need_update_ = false;
  d->audio_devices_.clear();

  const content::MediaStreamDevices& devices =
      MediaCaptureDevicesDispatcher::GetInstance()->GetAudioCaptureDevices();

  for (auto& device : devices) {
    if (device.type != content::MEDIA_DEVICE_AUDIO_CAPTURE) {
      continue;
    }

    d->audio_devices_ <<
        OxideQAudioCaptureDevice(QString::fromStdString(device.id),
                                 QString::fromStdString(device.name));
  }

  return d->audio_devices_;
}

QList<OxideQVideoCaptureDevice>
OxideQMediaCaptureDevices::availableVideoDevices() {
  Q_D(OxideQMediaCaptureDevices);

  if (!d->video_devices_need_update_) {
    return d->video_devices_;
  }

  d->video_devices_need_update_ = false;
  d->video_devices_.clear();

  const content::MediaStreamDevices& devices =
      MediaCaptureDevicesDispatcher::GetInstance()->GetVideoCaptureDevices();

  for (auto& device : devices) {
    if (device.type != content::MEDIA_DEVICE_VIDEO_CAPTURE) {
      continue;
    }

    d->video_devices_ <<
        OxideQVideoCaptureDevice(
          QString::fromStdString(device.id),
          QString::fromStdString(device.name),
          VideoFacingModeToPosition(device.video_facing));
  }

  return d->video_devices_;
}
