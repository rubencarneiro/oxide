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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_hybris_utils.h"

#include <cstdio>
#include <hybris/properties/properties.h>

#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/native_library.h"
#include "base/strings/stringprintf.h"

namespace oxide {

// Copied from base/sys_info_internal.h
template<typename T, T (*F)(void)>
class LazyHybrisValue {
 public:
  LazyHybrisValue()
      : value_(F()) { }

  ~LazyHybrisValue() { }

  const T& value() { return value_; }

 private:
  const T value_;

  DISALLOW_COPY_AND_ASSIGN(LazyHybrisValue);
};

struct DevicePropertyData {
  HybrisUtils::DeviceProperties properties;
  bool available;
};

namespace {

std::string ParseOSVersion(const char* os_version_str) {
  int32_t major, minor, bugfix;

  if (!os_version_str[0]) {
    return std::string();
  }

  int num_read = sscanf(os_version_str, "%d.%d.%d", &major, &minor, &bugfix);
  if (num_read <= 0) {
    return std::string();
  }

  if (num_read < 2) {
    minor = 0;
  }
  if (num_read < 3) {
    bugfix = 0;
  }

  return base::StringPrintf("%d.%d.%d", major, minor, bugfix);
}

DevicePropertyData PopulateDeviceProperties() {
  DevicePropertyData data;
  data.available = false;

  char value[PROP_VALUE_MAX];

  if (::property_get("ro.product.name", value, nullptr) <= 0) {
    return data;
  }

  data.available = true;
  data.properties.product = value;

  ::property_get("ro.product.device", value, nullptr);
  data.properties.device = value;

  ::property_get("ro.product.board", value, nullptr);
  data.properties.board = value;

  ::property_get("ro.product.brand", value, nullptr);
  data.properties.brand = value;

  ::property_get("ro.product.model", value, nullptr);
  data.properties.model = value;

  ::property_get("ro.build.version.release", value, nullptr);
  data.properties.os_version = ParseOSVersion(value);

  return data;
}

bool UsingAndroidEGL() {
  base::NativeLibrary egl_lib =
      base::LoadNativeLibrary(base::FilePath("libEGL.so.1"), nullptr);
  if (!egl_lib) {
    return false;
  }

  return !!base::GetFunctionPointerFromNativeLibrary(
      egl_lib, "hybris_egl_display_get_mapping");
}

#if defined(ENABLE_HYBRIS_CAMERA)
bool CameraCompatAvailable() {
  base::NativeLibrary camera_lib =
      base::LoadNativeLibrary(base::FilePath("libcamera.so.1"), nullptr);
  DCHECK(camera_lib);

  typedef void (*hybris_camera_initialize_fn)();
  auto hybris_camera_initialize =
      reinterpret_cast<hybris_camera_initialize_fn>(
        base::GetFunctionPointerFromNativeLibrary(camera_lib,
                                                  "hybris_camera_initialize"));
  DCHECK(hybris_camera_initialize);

  hybris_camera_initialize();

  void** handle =
      reinterpret_cast<void**>(base::GetFunctionPointerFromNativeLibrary(
        camera_lib, "camera_handle"));
  DCHECK(handle);

  return !!*handle;
}
#endif

base::LazyInstance<
    LazyHybrisValue<DevicePropertyData, PopulateDeviceProperties>>
      g_device_properties = LAZY_INSTANCE_INITIALIZER;
base::LazyInstance<LazyHybrisValue<bool, UsingAndroidEGL>>
    g_using_android_egl = LAZY_INSTANCE_INITIALIZER;
#if defined(ENABLE_HYBRIS_CAMERA)
base::LazyInstance<LazyHybrisValue<bool, CameraCompatAvailable>>
    g_camera_compat_available = LAZY_INSTANCE_INITIALIZER;
#endif

}

// static
bool HybrisUtils::HasDeviceProperties() {
  return g_device_properties.Get().value().available;
}

// static
const HybrisUtils::DeviceProperties& HybrisUtils::GetDeviceProperties() {
  DCHECK(HasDeviceProperties());
  return g_device_properties.Get().value().properties;
}

// static
bool HybrisUtils::IsUsingAndroidEGL() {
  return g_using_android_egl.Get().value();
}

#if defined(ENABLE_HYBRIS_CAMERA)
// static
bool HybrisUtils::IsCameraCompatAvailable() {
  return g_camera_compat_available.Get().value();
}
#endif

} // namespace oxide
