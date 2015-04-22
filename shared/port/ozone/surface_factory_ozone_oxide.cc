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

#include "surface_factory_ozone_oxide.h"

#include "EGL/egl.h"

namespace ui {

bool SurfaceFactoryOzoneOxide::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) {
  return false;
}

const int32* SurfaceFactoryOzoneOxide::GetEGLSurfaceProperties(
    const int32* desired_list) {
  // The Mir EGL backend in mesa doesn't support pbuffer surfaces,
  // so the default attributes passed to eglChooseConfig in Chromium
  // will result in 0 configs - see https://launchpad.net/bugs/1307709

  // This detection is a bit of a hack. Not sure if there's a better way
  // to do this?
  char* egl_platform = getenv("EGL_PLATFORM");
  if (!egl_platform || 
        ((strcmp(egl_platform, "mir") != 0) && (strcmp(egl_platform, "drm") != 0))) {
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

SurfaceFactoryOzoneOxide::SurfaceFactoryOzoneOxide() {}

} // namespace ui
