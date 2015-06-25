// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "surface_factory_ozone_oxide.h"

#include <map>
#include <utility>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/scoped_native_library.h"
#include "base/strings/string_piece.h"
#include "EGL/egl.h"

#include "../../../shared/port/gfx/gfx_utils_oxide.h"

namespace ui {

namespace {

struct MirMesaEGLNativeDisplay;

bool IsDriverVendorMesa(EGLNativeDisplayType native_display) {
  // We can't create a context here, as we haven't yet chosen an appropriate
  // config so we can't create a surface.
  // We also can't use Chromium's GL bindings, as //ui/gl depends on us

  typedef EGLDisplay (*eglGetDisplayFnType)(EGLNativeDisplayType);
  typedef EGLBoolean (*eglInitializeFnType)(EGLDisplay, EGLint*, EGLint*);
  typedef EGLBoolean (*eglTerminateFnType)(EGLDisplay);
  typedef const char* (*eglQueryStringFnType)(EGLDisplay, EGLint);

  base::FilePath empty;
  base::ScopedNativeLibrary library(empty);

  eglGetDisplayFnType eglGetDisplayFn =
      reinterpret_cast<eglGetDisplayFnType>(
        library.GetFunctionPointer("eglGetDisplay"));
  eglInitializeFnType eglInitializeFn =
      reinterpret_cast<eglInitializeFnType>(
        library.GetFunctionPointer("eglInitialize"));
  eglTerminateFnType eglTerminateFn =
      reinterpret_cast<eglTerminateFnType>(
        library.GetFunctionPointer("eglTerminate"));
  eglQueryStringFnType eglQueryStringFn =
      reinterpret_cast<eglQueryStringFnType>(
        library.GetFunctionPointer("eglQueryString"));

  if (!eglGetDisplayFn ||
      !eglInitializeFn ||
      !eglTerminateFn ||
      !eglQueryStringFn) {
    return false;
  }

  EGLDisplay display = eglGetDisplayFn(native_display);

  if (!eglInitializeFn(display, nullptr, nullptr)) {
    return false;
  }

  base::StringPiece egl_vendor(eglQueryStringFn(display, EGL_VENDOR));
  if (egl_vendor == base::StringPiece("Mesa Project")) {
    eglTerminateFn(display);
    return true;
  }

  eglTerminateFn(display);
  return false;
}

bool TestIsValidMirMesaDisplay(const base::ScopedNativeLibrary& library,
                               const char* function_name,
                               EGLNativeDisplayType display) {
  typedef int (*MirEGLNativeDisplayIsValidFunc)(MirMesaEGLNativeDisplay*);

  MirEGLNativeDisplayIsValidFunc function =
      reinterpret_cast<MirEGLNativeDisplayIsValidFunc>(
        library.GetFunctionPointer(function_name));
  if (!function) {
    return false;
  }

  return function(reinterpret_cast<MirMesaEGLNativeDisplay*>(display));
}

bool IsRunningOnMesaMir(EGLNativeDisplayType native_display) {
  if (!IsDriverVendorMesa(native_display)) {
    return false;
  }

  base::FilePath empty;
  base::ScopedNativeLibrary library(empty);
  DCHECK(library.is_valid());

  static const char* kTestFunctions[] = {
    "mir_egl_mesa_display_is_valid",
    "mir_client_mesa_egl_native_display_is_valid",
    "mir_server_mesa_egl_native_display_is_valid"
  };

  for (size_t i = 0; i < arraysize(kTestFunctions); ++i) {
    if (TestIsValidMirMesaDisplay(library,
                                  kTestFunctions[i],
                                  native_display)) {
      return true;
    }
  }

  return false;
}

bool IsRunningOnMesaDrm(EGLNativeDisplayType native_display) {
  if (!IsDriverVendorMesa(native_display)) {
    return false;
  }

  if (!native_display) {
    return false;
  }

  base::FilePath empty;
  base::ScopedNativeLibrary library(empty);
  DCHECK(library.is_valid());

  void* gbm_create_device = library.GetFunctionPointer("gbm_create_device");
  if (!gbm_create_device) {
    return false;
  }

  void* first_pointer = *reinterpret_cast<void**>(native_display);
  return first_pointer == gbm_create_device;
}

bool PBufferSurfacesSupported(EGLNativeDisplayType native_display) {
  if (IsRunningOnMesaMir(native_display)) {
    return false;
  }

  if (IsRunningOnMesaDrm(native_display)) {
    return false;
  }

  return true;
}

std::map<EGLint, EGLint> GetPlatformAttribOverrides(
    EGLNativeDisplayType native_display) {
  std::map<EGLint, EGLint> overrides;

  if (!PBufferSurfacesSupported(native_display)) {
    overrides[EGL_SURFACE_TYPE] = EGL_WINDOW_BIT;
  }

  return overrides;
}

std::pair<EGLint, EGLint> FilterAttribute(
    EGLint attrib,
    EGLint value,
    std::map<EGLint, EGLint>* overrides) {
  auto it = overrides->find(attrib);
  if (it != overrides->end()) {
    value = it->second;
    overrides->erase(it);
  }

  return std::make_pair(attrib, value);
}

}

intptr_t SurfaceFactoryOzoneOxide::GetNativeDisplay() {
  return gfx::GetOxideNativeDisplay();
}

bool SurfaceFactoryOzoneOxide::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) {
  return false;
}

const int32_t* SurfaceFactoryOzoneOxide::GetEGLSurfaceProperties(
    const int32_t* desired_list) {
  static const int kMaxNumberOfAttribs = 7;

  static bool s_initialized = false;
  static EGLint s_config_attribs[kMaxNumberOfAttribs * 2 + 1] = { EGL_NONE };

  if (s_initialized) {
    return s_config_attribs;
  }

  s_initialized = true;

  std::map<EGLint, EGLint> attribute_overrides =
      GetPlatformAttribOverrides(GetNativeDisplay());
  if (attribute_overrides.size() == 0) {
    return desired_list;
  }

  int i = 0;
  while (i < kMaxNumberOfAttribs) {
    int j = i * 2;

    if (desired_list[j] == EGL_NONE) {
      break;
    }

    ++i;

    auto filtered = FilterAttribute(desired_list[j],
                                    desired_list[j + 1],
                                    &attribute_overrides);
    s_config_attribs[j] = filtered.first;
    s_config_attribs[j + 1] = filtered.second;
  }

  CHECK_EQ(desired_list[i * 2], EGL_NONE) <<
      "If you hit this then you need to increase kMaxNumberOfAttribs";
  CHECK_EQ(attribute_overrides.size(), 0U) <<
      "If you hit this, some override attributes were not present in the set "
      "from Chromium. You'll need to provide extra code to add them";
  CHECK_LE(i, kMaxNumberOfAttribs);

  s_config_attribs[i * 2] = EGL_NONE;

  return s_config_attribs;
}

SurfaceFactoryOzoneOxide::SurfaceFactoryOzoneOxide() {}

} // namespace ui
