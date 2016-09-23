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

/*!
\class OxideQAudioCaptureDevice
\inmodule OxideQtCore
\inheaderfile oxideqmediacapturedevices.h

\brief Provides information about an audio capture device

OxideQAudioCaptureDevice provides information about an audio capture device.
The only information currently available is the device \l{id} and displayName.
*/

OxideQAudioCaptureDevice::OxideQAudioCaptureDevice(const QString& id,
                                                   const QString& name)
    : d(new OxideQAudioCaptureDeviceData(id, name)) {}

/*!
Copy construct a new OxideQAudioCaptureDevice from \a{other}.
*/

OxideQAudioCaptureDevice::OxideQAudioCaptureDevice(
    const OxideQAudioCaptureDevice& other)
    : d(other.d) {}

/*!
Destroy this OxideQAudioCaptureDevice.
*/

OxideQAudioCaptureDevice::~OxideQAudioCaptureDevice() {}

/*!
Return the ID of this device. This ID is unique for this device and this
session, but applications should not rely on this ID persisting between
sessions.
*/

QString OxideQAudioCaptureDevice::id() const {
  return d->id;
}

/*!
Return the display name of this device.
*/

QString OxideQAudioCaptureDevice::displayName() const {
  return d->name;
}

/*!
\class OxideQVideoCaptureDevice
\inmodule OxideQtCore
\inheaderfile oxideqmediacapturedevices.h

\brief Provides information about a video capture device

OxideQVideoCaptureDevice provides information about a video capture device. The
device ID can be determined by calling \l{id}, and the device's display name is
available via displayName.
 
On some devices it is possible to determine whether this is a front facing or
rear facing video capture device by calling \l{position}.
*/

/*!
\enum OxideQVideoCaptureDevice::Position

This describes a video capture device position.

\value PositionUnspecified
The camera position could not be determined.

\value PositionFrontFace
The camera is front facing, ie, pointing towards the user.

\value PositionBackFace
The camera is rear facing, ie, pointing away from the user.
*/

OxideQVideoCaptureDevice::OxideQVideoCaptureDevice(const QString& id,
                                                   const QString& name,
                                                   Position position)
    : d(new OxideQVideoCaptureDeviceData(id, name, position)) {}

/*!
Copy construct a new OxideQVideoCaptureDevice from \a{other}.
*/

OxideQVideoCaptureDevice::OxideQVideoCaptureDevice(
    const OxideQVideoCaptureDevice& other)
    : d(other.d) {}

/*!
Destroy this OxideQVideoCaptureDevice.
*/

OxideQVideoCaptureDevice::~OxideQVideoCaptureDevice() {}

/*!
Return the ID of this device. This ID is unique for this device and this
session, but applications should not rely on this ID persisting between
sessions.
*/

QString OxideQVideoCaptureDevice::id() const {
  return d->id;
}

/*!
Return the display name of this device.
*/

QString OxideQVideoCaptureDevice::displayName() const {
  return d->name;
}

/*!
Return the position of this device.

On devices where this is not supported, this will return PositionUnspecified.
*/

OxideQVideoCaptureDevice::Position OxideQVideoCaptureDevice::position() const {
  return d->position;
}

/*!
\class OxideQMediaCaptureDevices
\inmodule OxideQtCore
\inheaderfile oxideqmediacapturedevices.h

\brief Singleton for accessing media capture device info

OxideQMediaCaptureDevices provides a mechanism to discover information about
media capture devices detected by Oxide. Media capture devices can be made
available to web content via \e{MediaDevices.getUserMedia()}.
*/

/*!
\fn void OxideQMediaCaptureDevices::availableAudioDevicesChanged()

This signal is emitted when the list of available audio capture devices changes.

\sa availableAudioDevices
*/

/*!
\fn void OxideQMediaCaptureDevices::availableVideoDevicesChanged()

This signal is emitted when the list of available video capture devices changes.

\sa availableVideoDevices
*/

/*!
\internal
*/

OxideQMediaCaptureDevices::OxideQMediaCaptureDevices()
    : d_ptr(new OxideQMediaCaptureDevicesPrivate(this)) {}

/*!
\internal
*/

OxideQMediaCaptureDevices::~OxideQMediaCaptureDevices() {}

/*!
Return the OxideQMediaCaptureDevices singleton.
*/

// static
OxideQMediaCaptureDevices* OxideQMediaCaptureDevices::instance() {
  return g_instance();
}

/*!
Return a list of audio capture devices detected by Oxide.

\sa availableAudioDevicesChanged
*/

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

/*!
Return a list of video capture devices detected by Oxide.

\sa availableVideoDevicesChanged
*/

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
