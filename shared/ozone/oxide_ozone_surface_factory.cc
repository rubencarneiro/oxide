// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_ozone_surface_factory.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/native_library.h"
#include "ui/gfx/vsync_provider.h"
#include "ui/gl/gl_bindings.h"

#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {

namespace {

// Load a library, printing an error message on failure.
base::NativeLibrary LoadLibrary(const base::FilePath& filename) {
  base::NativeLibraryLoadError error;
  base::NativeLibrary library = base::LoadNativeLibrary(filename, &error);
  if (!library) {
    DVLOG(1) << "Failed to load " << filename.MaybeAsASCII() << ": " << error.ToString();
    return NULL;
  }
  return library;
}

base::NativeLibrary LoadLibrary(const char* filename) {
  return LoadLibrary(base::FilePath(filename));
}

} // namespace

gfx::SurfaceFactoryOzone::HardwareState
OzoneSurfaceFactory::InitializeHardware() {
  return gfx::SurfaceFactoryOzone::INITIALIZED;
}

void OzoneSurfaceFactory::ShutdownHardware() {}

intptr_t OzoneSurfaceFactory::GetNativeDisplay() {
  return BrowserProcessMain::instance()->GetNativeDisplay();
}

gfx::AcceleratedWidget OzoneSurfaceFactory::GetAcceleratedWidget() {
  NOTREACHED();
  return 0;
}

bool OzoneSurfaceFactory::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) {
  base::NativeLibrary gles_library = LoadLibrary("libGLESv2.so.2");
  if (!gles_library) {
    return false;
  }

  base::NativeLibrary egl_library = LoadLibrary("libEGL.so.1");
  if (!egl_library) {
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  GLGetProcAddressProc get_proc_address =
      reinterpret_cast<GLGetProcAddressProc>(
        base::GetFunctionPointerFromNativeLibrary(
          egl_library, "eglGetProcAddress"));
  if (!get_proc_address) {
    LOG(ERROR) << "eglGetProcAddress not found.";
    base::UnloadNativeLibrary(egl_library);
    base::UnloadNativeLibrary(gles_library);
    return false;
  }

  set_gl_get_proc_address.Run(get_proc_address);
  add_gl_library.Run(egl_library);
  add_gl_library.Run(gles_library);

  return true;
}

const int32* OzoneSurfaceFactory::GetEGLSurfaceProperties(
    const int32* desired_list) {
  // The Mir EGL backend in mesa doesn't support pbuffer surfaces,
  // so the default attributes passed to eglChooseConfig in Chromium
  // will result in 0 configs - see https://launchpad.net/bugs/1307709

  // This detection is a bit of a hack. Not sure if there's a better way
  // to do this?
  char* egl_platform = getenv("EGL_PLATFORM");
  if (!egl_platform || strcmp(egl_platform, "mir") != 0) {
    return desired_list;
  }
 
  // We should probably filter EGL_PBUFFER_BIT out of desired_list,
  // but for now just copy kConfigAttribs from gl_surface_egl.cc and omit
  // EGL_PBUFFER_BIT as it's easier
  static const EGLint kConfigAttribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  return kConfigAttribs;
}

OzoneSurfaceFactory::OzoneSurfaceFactory() {}

} // namespace oxide
