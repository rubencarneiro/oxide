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

#if defined(ENABLE_HYBRIS)
#include <algorithm>

#include "base/bind.h"
#include "base/callback.h"
#include "hybris/camera/camera_compatibility_layer.h"
#include "hybris/camera/camera_compatibility_layer_capabilities.h"
#endif

namespace oxide {

namespace {

#if defined(ENABLE_HYBRIS)

const char* GetDeviceNameForCameraType(CameraType type) {
  switch (type) {
    case BACK_FACING_CAMERA_TYPE:
      return "Rear camera";
    case FRONT_FACING_CAMERA_TYPE:
      return "Front camera";
  }

  NOTREACHED();
  return "";
}

const char* GetDeviceIdForCameraType(CameraType type) {
  switch (type) {
    case BACK_FACING_CAMERA_TYPE:
      return "HybrisRear";
    case FRONT_FACING_CAMERA_TYPE:
      return "HybrisFront";
  }

  NOTREACHED();
  return "";
}

scoped_ptr<media::VideoCaptureDevice::Names> GetDeviceNamesFromHybris() {
  // Although hybris provides a way to detect the number of cameras attached,
  // it provides no way to enumerate these. So we just check for a front or
  // rear camera

  if (android_camera_get_number_of_devices() == 0) {
    return nullptr;
  }

  scoped_ptr<media::VideoCaptureDevice::Names> names(
      new media::VideoCaptureDevice::Names());

  static const CameraType types[] = {
    BACK_FACING_CAMERA_TYPE,
    FRONT_FACING_CAMERA_TYPE
  };

  for (size_t i = 0; i < arraysize(types); ++i) {
    CameraType type = types[i];
    CameraControl* control = android_camera_connect_to(type, nullptr);
    if (!control) {
      continue;
    }

    android_camera_disconnect(control);
    android_camera_delete(control);

    names->push_back(
        media::VideoCaptureDevice::Name(
          GetDeviceNameForCameraType(type),
          GetDeviceIdForCameraType(type),
          media::VideoCaptureDevice::Name::API_TYPE_UNKNOWN));
  }

  return names;
}

void RespondToEnumerateDeviceNames(
    const EnumerateDevicesCallback& callback,
    media::VideoCaptureDevice::Names* hybris_names,
    scoped_ptr<media::VideoCaptureDevice::Names> names) {
  if (hybris_names) {
    for (const auto& hybris_name : *hybris_names) {
      bool duplicate = std::find_if(
          names->begin(),
          names->end(),
          [hybris_name](const media::VideoCaptureDevice::Name& name) {
        return hybris_name.id() == name.id();
      }) != names->end();
      if (duplicate) {
        LOG(ERROR) <<
            "A capture device with the ID \"" << hybris_name.id() <<
            "\" was produced by both the default backend and from Hybris. "
            "This is not supported";
        continue;
      }

      names->push_back(hybris_name);
    }
  }

  callback.Run(names.Pass());
}

#endif

}

scoped_ptr<media::VideoCaptureDevice> VideoCaptureDeviceFactoryLinux::Create(
    const media::VideoCaptureDevice::Name& device_name) {
  return delegate_->Create(device_name);
}

void VideoCaptureDeviceFactoryLinux::EnumerateDeviceNames(
    const EnumerateDevicesCallback& callback) {
  EnumerateDevicesCallback local_callback = callback;
#if defined(ENABLE_HYBRIS)
  scoped_ptr<media::VideoCaptureDevice::Names> names =
      GetDeviceNamesFromHybris();
  local_callback = base::Bind(RespondToEnumerateDeviceNames,
                              local_callback,
                              base::Owned(names.release()));
#endif
  delegate_->EnumerateDeviceNames(local_callback);
}

void VideoCaptureDeviceFactoryLinux::GetDeviceSupportedFormats(
    const media::VideoCaptureDevice::Name& device,
    media::VideoCaptureFormats* supported_formats) {
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
