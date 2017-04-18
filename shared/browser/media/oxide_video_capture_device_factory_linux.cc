// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include <libintl.h>
#include <utility>

#include "base/logging.h"
#include "media/capture/video/video_capture_device.h"

#if defined(ENABLE_HYBRIS_CAMERA)
#include <algorithm>
#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include "base/bind.h"
#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/strings/stringprintf.h"

#include "shared/browser/hybris_utils.h"

#include "oxide_video_capture_device_hybris.h"
#endif

namespace oxide {

namespace {

#if defined(ENABLE_HYBRIS_CAMERA)

const char* GetDeviceNameFromCameraType(CameraType type) {
  switch (type) {
    case BACK_FACING_CAMERA_TYPE:
      return dgettext(OXIDE_GETTEXT_DOMAIN, "Rear camera");
    case FRONT_FACING_CAMERA_TYPE:
      return dgettext(OXIDE_GETTEXT_DOMAIN, "Front camera");
  }

  NOTREACHED();
  return "";
}

void GetDeviceDescriptorsFromHybris(
    media::VideoCaptureDeviceDescriptors* descriptors) {
  DCHECK(HybrisUtils::GetInstance()->IsCameraCompatAvailable());

  int32_t number_of_devices = android_camera_get_number_of_devices();

  for (int32_t camera_id = 0; camera_id < number_of_devices; ++camera_id) {
    CameraType type;
    int orientation;
    if (android_camera_get_device_info(camera_id,
                                       reinterpret_cast<int*>(&type),
                                       &orientation) != 0) {
      LOG(ERROR) <<
          "Failed to get device info for camera with ID " << camera_id;
      continue;
    }

    std::string device_id =
        base::StringPrintf("%s%d",
                           VideoCaptureDeviceHybris::GetDeviceIdPrefix(),
                           camera_id);

    descriptors->push_back(
        media::VideoCaptureDeviceDescriptor(
          // XXX: We should append an integer to this when there are multiple
          // cameras facing the same direction
          GetDeviceNameFromCameraType(type),
          device_id));
  }
}

bool IsDeviceDescriptorIn(const media::VideoCaptureDeviceDescriptor& descriptor,
                          const media::VideoCaptureDeviceDescriptors& descriptors) {
  return std::find_if(
      descriptors.begin(),
      descriptors.end(),
      [descriptor](const media::VideoCaptureDeviceDescriptor& i) {
        return descriptor.device_id == i.device_id;
      }) != descriptors.end();
}

#endif

}

std::unique_ptr<media::VideoCaptureDevice>
VideoCaptureDeviceFactoryLinux::CreateDevice(
    const media::VideoCaptureDeviceDescriptor& device_descriptor) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (!HybrisUtils::GetInstance()->IsCameraCompatAvailable()) {
    return platform_factory_->CreateDevice(device_descriptor);
  }

  media::VideoCaptureDeviceDescriptors descriptors
  GetDeviceDescriptorsFromHybris(&descriptors);

  if (!IsDeviceDescriptorIn(device_descriptor, descriptors)) {
    return nullptr;
  }

  return base::WrapUnique(new VideoCaptureDeviceHybris(device_descriptor));
#else
  return platform_factory_->CreateDevice(device_descriptor);
#endif
}

void VideoCaptureDeviceFactoryLinux::GetSupportedFormats(
    const media::VideoCaptureDeviceDescriptor& device_descriptor,
    media::VideoCaptureFormats* supported_formats) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (HybrisUtils::GetInstance()->IsCameraCompatAvailable()) {
    return;
  }
#endif

  platform_factory_->GetSupportedFormats(device_descriptor, supported_formats);
}

void VideoCaptureDeviceFactoryLinux::GetDeviceDescriptors(
    media::VideoCaptureDeviceDescriptors* device_descriptors) {
#if defined(ENABLE_HYBRIS_CAMERA)
  if (HybrisUtils::GetInstance()->IsCameraCompatAvailable()) {
    GetDeviceDescriptorsFromHybris(device_descriptors);
  } else
#endif
  {
    platform_factory_->GetDeviceDescriptors(device_descriptors);
  }
}

VideoCaptureDeviceFactoryLinux::VideoCaptureDeviceFactoryLinux(
    std::unique_ptr<media::VideoCaptureDeviceFactory> platform_factory)
    : platform_factory_(std::move(platform_factory)) {}

VideoCaptureDeviceFactoryLinux::~VideoCaptureDeviceFactoryLinux() {}

} // namespace oxide
