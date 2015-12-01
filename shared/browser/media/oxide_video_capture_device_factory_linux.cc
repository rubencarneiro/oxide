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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
// USA

#include "oxide_video_capture_device_factory_linux.h"

#include "base/logging.h"
#include "media/capture/video/video_capture_device.h"

#if defined(ENABLE_HYBRIS_CAMERA)
#include <algorithm>
#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include "base/bind.h"
#include "base/callback.h"
#include "base/strings/stringprintf.h"

#include "shared/browser/oxide_android_properties.h"

#include "oxide_video_capture_device_hybris.h"
#endif

namespace oxide {

namespace {

#if defined(ENABLE_HYBRIS_CAMERA)

const char* GetDeviceNameFromCameraType(CameraType type) {
  switch (type) {
    case BACK_FACING_CAMERA_TYPE:
      return "Rear camera";
    case FRONT_FACING_CAMERA_TYPE:
      return "Front camera";
  }

  NOTREACHED();
  return "";
}

scoped_ptr<media::VideoCaptureDevice::Names> GetDeviceNamesFromHybris() {
  DCHECK(AndroidProperties::GetInstance()->Available());

  int32_t number_of_devices = android_camera_get_number_of_devices();

  scoped_ptr<media::VideoCaptureDevice::Names> names(
      new media::VideoCaptureDevice::Names());

  for (int32_t camera_id = 0; camera_id < number_of_devices; ++camera_id) {
    CameraType type;
    int orientation;
    android_camera_get_device_info(camera_id,
                                   reinterpret_cast<int*>(&type),
                                   &orientation);

    std::string device_id =
        base::StringPrintf("%s%d",
                           VideoCaptureDeviceHybris::GetDeviceIdPrefix(),
                           camera_id);

    names->push_back(
        media::VideoCaptureDevice::Name(
          // XXX: We should append an integer to this when there are multiple
          // cameras facing the same direction
          GetDeviceNameFromCameraType(type),
          device_id,
          media::VideoCaptureDevice::Name::API_TYPE_UNKNOWN));
  }

  return names;
}

bool IsDeviceNameIn(const media::VideoCaptureDevice::Name& name,
                    media::VideoCaptureDevice::Names* names) {
  return std::find_if(
      names->begin(),
      names->end(),
      [name](const media::VideoCaptureDevice::Name& i) {
        return name.id() == i.id();
      }) != names->end();
}

#endif

}

scoped_ptr<media::VideoCaptureDevice> VideoCaptureDeviceFactoryLinux::Create(
    const media::VideoCaptureDevice::Name& device_name) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (!AndroidProperties::GetInstance()->Available()) {
    return delegate_->Create(device_name);
  }

  scoped_ptr<media::VideoCaptureDevice::Names> names =
      GetDeviceNamesFromHybris();
  if (!names) {
    return nullptr;
  }

  if (!IsDeviceNameIn(device_name, names.get())) {
    return nullptr;
  }

  return make_scoped_ptr(new VideoCaptureDeviceHybris(device_name));
#else
  return delegate_->Create(device_name);
#endif
}

void VideoCaptureDeviceFactoryLinux::EnumerateDeviceNames(
    const EnumerateDevicesCallback& callback) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (AndroidProperties::GetInstance()->Available()) {
    scoped_ptr<media::VideoCaptureDevice::Names> names =
        GetDeviceNamesFromHybris();
    callback.Run(names.Pass());
  } else
#endif
  {
    delegate_->EnumerateDeviceNames(callback);
  }
}

void VideoCaptureDeviceFactoryLinux::GetDeviceSupportedFormats(
    const media::VideoCaptureDevice::Name& device,
    media::VideoCaptureFormats* supported_formats) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (AndroidProperties::GetInstance()->Available()) {
    return;
  }
#endif

  delegate_->GetDeviceSupportedFormats(device, supported_formats);
}

void VideoCaptureDeviceFactoryLinux::GetDeviceNames(
    media::VideoCaptureDevice::Names* device_names) {
  NOTREACHED();
}

VideoCaptureDeviceFactoryLinux::VideoCaptureDeviceFactoryLinux(
    scoped_ptr<media::VideoCaptureDeviceFactory> delegate)
    : delegate_(delegate.Pass()) {}

VideoCaptureDeviceFactoryLinux::~VideoCaptureDeviceFactoryLinux() {}

} // namespace oxide
