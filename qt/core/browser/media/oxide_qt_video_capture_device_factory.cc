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

#include <QList>

#include "shared/browser/oxide_browser_platform_integration.h"

#include "oxide_qt_video_capture_device.h"

namespace oxide {
namespace qt {

scoped_ptr<media::VideoCaptureDevice> VideoCaptureDeviceFactory::Create(
    const media::VideoCaptureDevice::Name& device_name) {
  DCHECK(thread_checker_.CalledOnValidThread());

  return make_scoped_ptr(new VideoCaptureDevice(device_name));
}

void VideoCaptureDeviceFactory::GetDeviceNames(
    media::VideoCaptureDevice::Names* const device_names) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(device_names->empty());

  QList<QByteArray> devices = QCamera::availableDevices();
  for (const QByteArray& camera_id: devices) {
    device_names->push_back(media::VideoCaptureDevice::Name(
        camera_id.data(),
        camera_id.data(),
        media::VideoCaptureDevice::Name::API_TYPE_UNKNOWN));
  }
}

void VideoCaptureDeviceFactory::GetDeviceSupportedFormats(
    const media::VideoCaptureDevice::Name& device,
    media::VideoCaptureFormats* supported_formats) {
  DCHECK(thread_checker_.CalledOnValidThread());
}

}
}
