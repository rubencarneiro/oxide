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

#include "oxide_qt_video_capture_device_factory.h"

#include <algorithm>
#include <map>
#include <string>

#include <QByteArray>
#include <QCamera>
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
#include <QCameraInfo>
#endif
#include <QList>
#include <QString>

#include "shared/browser/oxide_browser_platform_integration.h"

#include "oxide_qt_video_capture_device.h"

namespace oxide {
namespace qt {

namespace {

typedef std::map<media::VideoCaptureDevice::Name, QCamera::Position>
    DefaultVideoDeviceNameMap;

DefaultVideoDeviceNameMap GetDefaultDeviceNames() {
  DefaultVideoDeviceNameMap rv;
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  const QCamera::Position positions[] = { QCamera::BackFace,
                                          QCamera::FrontFace };

  for (size_t i = 0; i < arraysize(positions); ++i) {
    QCamera camera(positions[i]);
    QCameraInfo info(camera);
    if (info.deviceName().isEmpty()) {
      // If there's no camera in the specified position, Qt will fall back to
      // the default. If there's no default, it's game over
      return rv;
    }
    if (!camera.isCaptureModeSupported(QCamera::CaptureVideo)) {
      continue;
    }

    media::VideoCaptureDevice::Name device_name(
        info.description().toStdString(),
        info.deviceName().toStdString(),
        media::VideoCaptureDevice::Name::API_TYPE_UNKNOWN);

    rv[device_name] = info.position();
  }
#endif
  return rv;
}

}

scoped_ptr<media::VideoCaptureDevice> VideoCaptureDeviceFactory::Create(
    const media::VideoCaptureDevice::Name& device_name) {
  DCHECK(thread_checker_.CalledOnValidThread());

  DefaultVideoDeviceNameMap default_names = GetDefaultDeviceNames();
  if (default_names.find(device_name) != default_names.end()) {
    // If the device name corresponds to a camera that is the default for its
    // position, then we request the camera by position. This is to workaround
    // a limitation in the QtUbuntu backend, which doesn't let us select a
    // camera by its ID
    return make_scoped_ptr(new VideoCaptureDevice(default_names[device_name]));
  }

  return make_scoped_ptr(new VideoCaptureDevice(device_name));
}

void VideoCaptureDeviceFactory::GetDeviceNames(
    media::VideoCaptureDevice::Names* const device_names) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(device_names->empty());

#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  QList<QCameraInfo> devices = QCameraInfo::availableCameras();
#else
  QList<QByteArray> devices = QCamera::availableDevices();
#endif
  for (const auto& info : devices) {
    QCamera camera(info);
    if (!camera.isCaptureModeSupported(QCamera::CaptureVideo)) {
      continue;
    }
    device_names->push_back(media::VideoCaptureDevice::Name(
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
        info.description().toStdString(),
        info.deviceName().toStdString(),
#else
        info.data(),
        info.data(),
#endif
        media::VideoCaptureDevice::Name::API_TYPE_UNKNOWN));
  }

  // The QtUbuntu backend doesn't support device enumeration yet, so we
  // emulate enumeration here in a second pass by constructing default
  // back and forward facing cameras to try to establish if they exist

  DefaultVideoDeviceNameMap default_names = GetDefaultDeviceNames();
  for (const auto& name : default_names) {
    const std::string& id = name.first.id();
    bool duplicate = std::find_if(
        device_names->begin(),
        device_names->end(),
        [id](const media::VideoCaptureDevice::Name& name) {
          return name.id() == id;
        }) != device_names->end();
    if (duplicate) {
      // Don't duplicate devices
      continue;
    }

    device_names->push_back(name.first);
  }
}

void VideoCaptureDeviceFactory::GetDeviceSupportedFormats(
    const media::VideoCaptureDevice::Name& device,
    media::VideoCaptureFormats* supported_formats) {
  DCHECK(thread_checker_.CalledOnValidThread());
}

}
}
