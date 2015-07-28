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

// This is based on the Chromium GPU info collector implementations for both Android
// and Linux, so some of the code is copied from the following files:
//
//  gpu/config/gpu_info_collector_android.cc
//   Copyright (c) 2012 The Chromium Authors. All rights reserved.
//   Use of this source code is governed by a BSD-style license that can be
//   found in the LICENSE file.
//
//  gpu/config/gpu_info_collector_linux.cc
//   Copyright (c) 2014 The Chromium Authors. All rights reserved.
//   Use of this source code is governed by a BSD-style license that can be
//   found in the LICENSE file.
//
//  gpu/config/gpu_info_collector.cc
//   Copyright (c) 2012 The Chromium Authors. All rights reserved.
//   Use of this source code is governed by a BSD-style license that can be
//   found in the LICENSE file.

#include "oxide_gpu_info_collector_linux.h"

#include <string>
#include <vector>
#include <X11/Xlib.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/native_library.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "gpu/config/gpu_info.h"
#include "gpu/config/gpu_info_collector.h"
#include "third_party/libXNVCtrl/NVCtrl.h"
#include "third_party/libXNVCtrl/NVCtrlLib.h"
#include "ui/gfx/x/x11_types.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"

#include "library_loaders/libpci.h"

#include "shared/port/gpu_config/gpu_info_collector_oxide_linux.h"

#include "oxide_android_properties.h"

namespace oxide {

namespace {

std::string GetDriverVersionFromString(const std::string& version_string) {
  // Extract driver version from the second number in a string like:
  // "OpenGL ES 2.0 V@6.0 AU@ (CL@2946718)"

  // Exclude first "2.0".
  size_t begin = version_string.find_first_of("0123456789");
  if (begin == std::string::npos) {
    return "0";
  }
  size_t end = version_string.find_first_not_of("01234567890.", begin);

  // Extract number of the form "%d.%d"
  begin = version_string.find_first_of("0123456789", end);
  if (begin == std::string::npos) {
    return "0";
  }
  end = version_string.find_first_not_of("01234567890.", begin);
  std::string sub_string;
  if (end != std::string::npos) {
    sub_string = version_string.substr(begin, end - begin);
  } else {
    sub_string = version_string.substr(begin);
  }
  std::vector<std::string> pieces;
  base::SplitString(sub_string, '.', &pieces);
  if (pieces.size() < 2) {
    return "0";
  }
  return pieces[0] + "." + pieces[1];
}

std::pair<std::string, size_t> GetVersionFromString(
    const std::string& version_string,
    size_t begin = 0) {
  begin = version_string.find_first_of("0123456789", begin);
  if (begin == std::string::npos) {
    return std::make_pair("", std::string::npos);
  }

  size_t end = version_string.find_first_not_of("01234567890.", begin);
  std::string sub_string;
  if (end != std::string::npos) {
    sub_string = version_string.substr(begin, end - begin);
  } else {
    sub_string = version_string.substr(begin);
  }
  std::vector<std::string> pieces;
  base::SplitString(sub_string, '.', &pieces);
  if (pieces.size() >= 2) {
    return std::make_pair(pieces[0] + "." + pieces[1], end);
  } else {
    return std::make_pair("", end);
  }
}

gpu::CollectInfoResult CollectDriverInfo(gpu::GPUInfo* gpu_info) {
  // Go through the process of loading GL libs and initializing an EGL
  // context so that we can get GL vendor/version/renderer strings.
  base::NativeLibrary gles_library, egl_library;
  base::NativeLibraryLoadError error;
  gles_library =
      base::LoadNativeLibrary(base::FilePath("libGLESv2.so.2"), &error);
  if (!gles_library) {
    LOG(FATAL) << "Failed to load libGLESv2.so.2";
  }

  egl_library = base::LoadNativeLibrary(base::FilePath("libEGL.so.1"), &error);
  if (!egl_library) {
    LOG(FATAL) << "Failed to load libEGL.so.1";
  }

  typedef void* (*eglGetProcAddressProc)(const char* name);

  auto eglGetProcAddressFn = reinterpret_cast<eglGetProcAddressProc>(
      base::GetFunctionPointerFromNativeLibrary(egl_library,
                                                "eglGetProcAddress"));
  if (!eglGetProcAddressFn) {
    LOG(FATAL) << "eglGetProcAddress not found.";
  }

  auto get_func = [eglGetProcAddressFn, gles_library, egl_library](
      const char* name) {
    void *proc;
    proc = base::GetFunctionPointerFromNativeLibrary(egl_library, name);
    if (proc) {
      return proc;
    }
    proc = base::GetFunctionPointerFromNativeLibrary(gles_library, name);
    if (proc) {
      return proc;
    }
    proc = eglGetProcAddressFn(name);
    if (proc) {
      return proc;
    }
    LOG(FATAL) << "Failed to look up " << name;
    return (void *)nullptr;
  };

#define LOOKUP_FUNC(x) auto x##Fn = reinterpret_cast<gfx::x##Proc>(get_func(#x))

  LOOKUP_FUNC(eglGetError);
  LOOKUP_FUNC(eglQueryString);
  LOOKUP_FUNC(eglGetCurrentContext);
  LOOKUP_FUNC(eglGetCurrentDisplay);
  LOOKUP_FUNC(eglGetCurrentSurface);
  LOOKUP_FUNC(eglGetDisplay);
  LOOKUP_FUNC(eglInitialize);
  LOOKUP_FUNC(eglChooseConfig);
  LOOKUP_FUNC(eglCreateContext);
  LOOKUP_FUNC(eglCreatePbufferSurface);
  LOOKUP_FUNC(eglMakeCurrent);
  LOOKUP_FUNC(eglDestroySurface);
  LOOKUP_FUNC(eglDestroyContext);

  LOOKUP_FUNC(glGetString);
  LOOKUP_FUNC(glGetIntegerv);

#undef LOOKUP_FUNC

  EGLDisplay curr_display = eglGetCurrentDisplayFn();
  EGLContext curr_context = eglGetCurrentContextFn();
  EGLSurface curr_draw_surface = eglGetCurrentSurfaceFn(EGL_DRAW);
  EGLSurface curr_read_surface = eglGetCurrentSurfaceFn(EGL_READ);

  EGLDisplay temp_display = EGL_NO_DISPLAY;
  EGLContext temp_context = EGL_NO_CONTEXT;
  EGLSurface temp_surface = EGL_NO_SURFACE;

  const EGLint kConfigAttribs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
      EGL_NONE};
  const EGLint kContextAttribs[] = {
      EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT,
          EGL_LOSE_CONTEXT_ON_RESET_EXT,
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE};
  const EGLint kSurfaceAttribs[] = {
      EGL_WIDTH, 1,
      EGL_HEIGHT, 1,
      EGL_NONE};

  EGLint major, minor;

  EGLConfig config;
  EGLint num_configs;

  auto errorstr = [eglGetErrorFn]() {
    uint32_t err = eglGetErrorFn();
    return base::StringPrintf("%s (%x)", ui::GetEGLErrorString(err), err);
  };

  temp_display = eglGetDisplayFn(EGL_DEFAULT_DISPLAY);

  if (temp_display == EGL_NO_DISPLAY) {
    LOG(FATAL) << "failed to get display. " << errorstr();
  }

  eglInitializeFn(temp_display, &major, &minor);

  bool egl_create_context_robustness_supported =
      strstr(reinterpret_cast<const char*>(
                 eglQueryStringFn(temp_display, EGL_EXTENSIONS)),
             "EGL_EXT_create_context_robustness") != NULL;

  if (!eglChooseConfigFn(temp_display, kConfigAttribs, &config, 1,
                         &num_configs)) {
    LOG(FATAL) << "failed to choose an egl config. " << errorstr();
  }

  temp_context = eglCreateContextFn(
      temp_display, config, EGL_NO_CONTEXT,
      kContextAttribs + (egl_create_context_robustness_supported ? 0 : 2));
  if (temp_context == EGL_NO_CONTEXT) {
    LOG(FATAL)
        << "failed to create a temporary context for fetching driver strings. "
        << errorstr();
  }

  temp_surface =
      eglCreatePbufferSurfaceFn(temp_display, config, kSurfaceAttribs);

  if (temp_surface == EGL_NO_SURFACE) {
    eglDestroyContextFn(temp_display, temp_context);
    LOG(FATAL)
        << "failed to create a pbuffer surface for fetching driver strings. "
        << errorstr();
  }

  eglMakeCurrentFn(temp_display, temp_surface, temp_surface, temp_context);

  gpu_info->gl_vendor = reinterpret_cast<const char*>(glGetStringFn(GL_VENDOR));
  gpu_info->gl_version =
      reinterpret_cast<const char*>(glGetStringFn(GL_VERSION));
  gpu_info->gl_renderer =
      reinterpret_cast<const char*>(glGetStringFn(GL_RENDERER));
  gpu_info->gl_extensions =
      reinterpret_cast<const char*>(glGetStringFn(GL_EXTENSIONS));

  GLint max_samples = 0;
  glGetIntegervFn(GL_MAX_SAMPLES, &max_samples);
  gpu_info->max_msaa_samples = base::IntToString(max_samples);

  bool supports_robustness =
      gpu_info->gl_extensions.find("GL_EXT_robustness") != std::string::npos ||
      gpu_info->gl_extensions.find("GL_KHR_robustness") != std::string::npos ||
      gpu_info->gl_extensions.find("GL_ARB_robustness") != std::string::npos;

  if (supports_robustness) {
    glGetIntegervFn(
        GL_RESET_NOTIFICATION_STRATEGY_ARB,
        reinterpret_cast<GLint*>(&gpu_info->gl_reset_notification_strategy));
  }

  std::string glsl_version_string =
      reinterpret_cast<const char*>(glGetStringFn(GL_SHADING_LANGUAGE_VERSION));

  std::string glsl_version = GetVersionFromString(glsl_version_string).first;
  gpu_info->pixel_shader_version = glsl_version;
  gpu_info->vertex_shader_version = glsl_version;

  if (curr_display != EGL_NO_DISPLAY &&
      curr_context != EGL_NO_CONTEXT) {
    eglMakeCurrentFn(curr_display, curr_draw_surface, curr_read_surface,
                     curr_context);
  } else {
    eglMakeCurrentFn(temp_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT);
  }

  eglDestroySurfaceFn(temp_display, temp_surface);
  eglDestroyContextFn(temp_display, temp_context);

  return gpu::kCollectInfoSuccess;
}

gpu::CollectInfoResult CollectBasicGraphicsInfoAndroid(
    gpu::GPUInfo* gpu_info) {
  gpu_info->can_lose_context = false;

  gpu_info->machine_model_name = AndroidProperties::GetInstance()->GetModel();

  // Create a short-lived context on the UI thread to collect the GL strings.
  gpu::CollectInfoResult result = CollectDriverInfo(gpu_info);
  gpu_info->basic_info_state = result;
  gpu_info->context_info_state = result;

  return result;
}

gpu::CollectInfoResult CollectContextGraphicsInfoAndroid(
    gpu::GPUInfo* gpu_info) {
  return CollectBasicGraphicsInfoAndroid(gpu_info);
}

gpu::CollectInfoResult CollectDriverInfoGLAndroid(gpu::GPUInfo* gpu_info) {
  gpu_info->driver_version = GetDriverVersionFromString(
      gpu_info->gl_version);
  gpu_info->gpu.vendor_string = gpu_info->gl_vendor;
  gpu_info->gpu.device_string = gpu_info->gl_renderer;

  return gpu::kCollectInfoSuccess;
}

// Scan /etc/ati/amdpcsdb.default for "ReleaseVersion".
// Return empty string on failing.
std::string CollectDriverVersionATI() {
  const base::FilePath::CharType kATIFileName[] =
      FILE_PATH_LITERAL("/etc/ati/amdpcsdb.default");
  base::FilePath ati_file_path(kATIFileName);
  if (!base::PathExists(ati_file_path)) {
    return std::string();
  }
  std::string contents;
  if (!base::ReadFileToString(ati_file_path, &contents)) {
    return std::string();
  }
  base::StringTokenizer t(contents, "\r\n");
  while (t.GetNext()) {
    std::string line = t.token();
    if (base::StartsWith(line,
                         "ReleaseVersion=",
                         base::CompareCase::SENSITIVE)) {
      size_t begin = line.find_first_of("0123456789");
      if (begin != std::string::npos) {
        size_t end = line.find_first_not_of("0123456789.", begin);
        if (end == std::string::npos) {
          return line.substr(begin);
        } else {
          return line.substr(begin, end - begin);
        }
      }
    }
  }
  return std::string();
}

// Use NVCtrl extention to query NV driver version.
// Return empty string on failing.
std::string CollectDriverVersionNVidia() {
  Display* display = gfx::GetXDisplay();
  if (!display) {
    VLOG(1) << "XOpenDisplay failed.";
    return std::string();
  }
  int event_base = 0, error_base = 0;
  if (!XNVCTRLQueryExtension(display, &event_base, &error_base)) {
    VLOG(1) << "NVCtrl extension does not exist.";
    return std::string();
  }
  int screen_count = ScreenCount(display);
  for (int screen = 0; screen < screen_count; ++screen) {
    char* buffer = NULL;
    if (XNVCTRLIsNvScreen(display, screen) &&
        XNVCTRLQueryStringAttribute(display, screen, 0,
                                    NV_CTRL_STRING_NVIDIA_DRIVER_VERSION,
                                    &buffer)) {
      std::string driver_version(buffer);
      XFree(buffer);
      return driver_version;
    }
  }
  return std::string();
}

// This checks if a system supports PCI bus.
// We check the existence of /sys/bus/pci or /sys/bug/pci_express.
bool IsPciSupported() {
  const base::FilePath pci_path("/sys/bus/pci/");
  const base::FilePath pcie_path("/sys/bus/pci_express/");
  return (base::PathExists(pci_path) || base::PathExists(pcie_path));
}

const uint32 kVendorIDIntel = 0x8086;
const uint32 kVendorIDNVidia = 0x10de;
const uint32 kVendorIDAMD = 0x1002;

gpu::CollectInfoResult CollectPCIVideoCardInfo(gpu::GPUInfo* gpu_info) {
  DCHECK(gpu_info);

  if (!IsPciSupported()) {
    VLOG(1) << "PCI bus scanning is not supported";
    return gpu::kCollectInfoNonFatalFailure;
  }

  // TODO(zmo): be more flexible about library name.
  LibPciLoader libpci_loader;
  if (!libpci_loader.Load("libpci.so.3") &&
      !libpci_loader.Load("libpci.so")) {
    VLOG(1) << "Failed to locate libpci";
    return gpu::kCollectInfoNonFatalFailure;
  }

  pci_access* access = (libpci_loader.pci_alloc)();
  DCHECK(access);
  (libpci_loader.pci_init)(access);
  (libpci_loader.pci_scan_bus)(access);
  bool primary_gpu_identified = false;
  for (pci_dev* device = access->devices; device; device = device->next) {
    // Fill the IDs and class fields.
    (libpci_loader.pci_fill_info)(device, 33);
    bool is_gpu = false;
    switch (device->device_class) {
      case PCI_CLASS_DISPLAY_VGA:
      case PCI_CLASS_DISPLAY_XGA:
      case PCI_CLASS_DISPLAY_3D:
        is_gpu = true;
        break;
      case PCI_CLASS_DISPLAY_OTHER:
      default:
        break;
    }
    if (!is_gpu) {
      continue;
    }
    if (device->vendor_id == 0 || device->device_id == 0) {
      continue;
    }

    gpu::GPUInfo::GPUDevice gpu;
    gpu.vendor_id = device->vendor_id;
    gpu.device_id = device->device_id;

    if (!primary_gpu_identified) {
      primary_gpu_identified = true;
      gpu_info->gpu = gpu;
    } else {
      // TODO(zmo): if there are multiple GPUs, we assume the non Intel
      // one is primary. Revisit this logic because we actually don't know
      // which GPU we are using at this point.
      if (gpu_info->gpu.vendor_id == kVendorIDIntel &&
          gpu.vendor_id != kVendorIDIntel) {
        gpu_info->secondary_gpus.push_back(gpu_info->gpu);
        gpu_info->gpu = gpu;
      } else {
        gpu_info->secondary_gpus.push_back(gpu);
      }
    }
  }

  // Detect Optimus or AMD Switchable GPU.
  if (gpu_info->secondary_gpus.size() == 1 &&
      gpu_info->secondary_gpus[0].vendor_id == kVendorIDIntel) {
    if (gpu_info->gpu.vendor_id == kVendorIDNVidia) {
      gpu_info->optimus = true;
    }
    if (gpu_info->gpu.vendor_id == kVendorIDAMD) {
      gpu_info->amd_switchable = true;
    }
  }

  (libpci_loader.pci_cleanup)(access);
  if (!primary_gpu_identified) {
    return gpu::kCollectInfoNonFatalFailure;
  }

  return gpu::kCollectInfoSuccess;
}

gpu::CollectInfoResult CollectContextGraphicsInfoLinux(
    gpu::GPUInfo* gpu_info) {
  gpu_info->can_lose_context =
      (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2);

  gpu::CollectInfoResult result = CollectGraphicsInfoGL(gpu_info);
  gpu_info->context_info_state = result;

  return result;
}

gpu::CollectInfoResult CollectBasicGraphicsInfoLinux(gpu::GPUInfo* gpu_info) {
  gpu::CollectInfoResult result = CollectPCIVideoCardInfo(gpu_info);

  std::string driver_version;
  switch (gpu_info->gpu.vendor_id) {
    case kVendorIDAMD:
      driver_version = CollectDriverVersionATI();
      if (!driver_version.empty()) {
        gpu_info->driver_vendor = "ATI / AMD";
        gpu_info->driver_version = driver_version;
      }
      break;
    case kVendorIDNVidia:
      driver_version = CollectDriverVersionNVidia();
      if (!driver_version.empty()) {
        gpu_info->driver_vendor = "NVIDIA";
        gpu_info->driver_version = driver_version;
      }
      break;
    case kVendorIDIntel:
      // In dual-GPU cases, sometimes PCI scan only gives us the
      // integrated GPU (i.e., the Intel one).
      if (gpu_info->secondary_gpus.size() == 0) {
        driver_version = CollectDriverVersionNVidia();
        if (!driver_version.empty()) {
          gpu_info->driver_vendor = "NVIDIA";
          gpu_info->driver_version = driver_version;
          gpu_info->optimus = true;
          // Put Intel to the secondary GPU list.
          gpu_info->secondary_gpus.push_back(gpu_info->gpu);
          // Put NVIDIA as the primary GPU.
          gpu_info->gpu.vendor_id = kVendorIDNVidia;
          gpu_info->gpu.device_id = 0;  // Unknown Device.
        }
      }
      break;
  }

  gpu_info->basic_info_state = result;

  return result;
}

gpu::CollectInfoResult CollectDriverInfoGLLinux(gpu::GPUInfo* gpu_info) {
  std::string gl_version = gpu_info->gl_version;
  if (base::StartsWith(gl_version, "OpenGL ES", base::CompareCase::SENSITIVE)) {
    gl_version = gl_version.substr(10);
  }
  std::vector<std::string> pieces;
  base::SplitStringAlongWhitespace(gl_version, &pieces);
  // In linux, the gl version string might be in the format of
  //   GLVersion DriverVendor DriverVersion
  if (pieces.size() < 3) {
    return gpu::kCollectInfoNonFatalFailure;
  }

  std::string driver_version = pieces[2];
  size_t pos = driver_version.find_first_not_of("0123456789.");
  if (pos == 0) {
    return gpu::kCollectInfoNonFatalFailure;
  }
  if (pos != std::string::npos) {
    driver_version = driver_version.substr(0, pos);
  }

  gpu_info->driver_vendor = pieces[1];
  gpu_info->driver_version = driver_version;

  return gpu::kCollectInfoSuccess;
}

}

class GpuInfoCollectorLinux : public gpu::GpuInfoCollectorOxideLinux {
 public:
  GpuInfoCollectorLinux() {}
  ~GpuInfoCollectorLinux() override {}

  gpu::CollectInfoResult CollectGpuID(uint32* vendor_id,
                                      uint32* device_id) override;
  gpu::CollectInfoResult CollectContextGraphicsInfo(
      gpu::GPUInfo* gpu_info) override;
  gpu::CollectInfoResult CollectBasicGraphicsInfo(
      gpu::GPUInfo* gpu_info) override;
  gpu::CollectInfoResult CollectDriverInfoGL(gpu::GPUInfo* gpu_info) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(GpuInfoCollectorLinux);
};

gpu::CollectInfoResult GpuInfoCollectorLinux::CollectGpuID(uint32* vendor_id,
                                                           uint32* device_id) {
  *vendor_id = 0;
  *device_id = 0;

  if (AndroidProperties::GetInstance()->Available()) {
    return gpu::kCollectInfoNonFatalFailure;
  }

  gpu::GPUInfo gpu_info;
  gpu::CollectInfoResult result = CollectPCIVideoCardInfo(&gpu_info);
  if (result == gpu::kCollectInfoSuccess) {
    *vendor_id = gpu_info.gpu.vendor_id;
    *device_id = gpu_info.gpu.device_id;
  }

  return result;
}

gpu::CollectInfoResult GpuInfoCollectorLinux::CollectContextGraphicsInfo(
    gpu::GPUInfo* gpu_info) {
  if (AndroidProperties::GetInstance()->Available()) {
    return CollectContextGraphicsInfoAndroid(gpu_info);
  }

  return CollectContextGraphicsInfoLinux(gpu_info);
}

gpu::CollectInfoResult GpuInfoCollectorLinux::CollectBasicGraphicsInfo(
    gpu::GPUInfo* gpu_info) {
  if (AndroidProperties::GetInstance()->Available()) {
    return CollectBasicGraphicsInfoAndroid(gpu_info);
  }

  return CollectBasicGraphicsInfoLinux(gpu_info);
}

gpu::CollectInfoResult GpuInfoCollectorLinux::CollectDriverInfoGL(
    gpu::GPUInfo* gpu_info) {
  if (AndroidProperties::GetInstance()->Available()) {
    return CollectDriverInfoGLAndroid(gpu_info);
  }

  return CollectDriverInfoGLLinux(gpu_info);
}

gpu::GpuInfoCollectorOxideLinux* CreateGpuInfoCollectorLinux() {
  return new GpuInfoCollectorLinux();
}

} // namespace oxide
