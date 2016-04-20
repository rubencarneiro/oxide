// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "ui/gl/gl_surface.h"

#include <memory>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/sys_info.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/gl_surface_glx.h"
#include "ui/gl/gl_surface_osmesa.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/surface_factory_ozone.h"

#if !defined(EGL_OPENGL_ES3_BIT)
#define EGL_OPENGL_ES3_BIT 0x00000040
#endif

namespace gfx {

namespace {

bool ValidateEglConfig(EGLDisplay display,
                       const EGLint* config_attribs,
                       EGLint* num_configs) {
  if (!eglChooseConfig(display,
                       config_attribs,
                       NULL,
                       0,
                       num_configs)) {
    LOG(ERROR) << "eglChooseConfig failed with error "
               << ui::GetLastEGLErrorString();
    return false;
  }
  if (*num_configs == 0) {
    return false;
  }
  return true;
}

EGLConfig ChooseRGB565Config(EGLDisplay display) {
  EGLint config_attribs[] = {
    EGL_BUFFER_SIZE, 16,
    EGL_BLUE_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_RED_SIZE, 5,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
    EGL_NONE
  };

  EGLint num_configs;
  if (!ValidateEglConfig(display, config_attribs, &num_configs)) {
    return nullptr;
  }

  std::unique_ptr<EGLConfig[]> matching_configs(new EGLConfig[num_configs]);
  EGLint config_size = num_configs;

  if (!eglChooseConfig(display,
                       config_attribs,
                       matching_configs.get(),
                       config_size,
                       &num_configs)) {
    LOG(ERROR) << "eglChooseConfig failed with error "
               << ui::GetLastEGLErrorString();
    return nullptr;
  }

  for (int i = 0; i < num_configs; i++) {
    EGLint red, green, blue, alpha;
    // Read the relevant attributes of the EGLConfig.
    if (eglGetConfigAttrib(display, matching_configs[i],
                           EGL_RED_SIZE, &red) &&
        eglGetConfigAttrib(display, matching_configs[i],
                           EGL_BLUE_SIZE, &blue) &&
        eglGetConfigAttrib(display, matching_configs[i],
                           EGL_GREEN_SIZE, &green) &&
        eglGetConfigAttrib(display, matching_configs[i],
                           EGL_ALPHA_SIZE, &alpha) &&
        alpha == 0 &&
        red == 5 &&
        green == 6 &&
        blue == 5) {
      return matching_configs[i];
    }
  }

  return nullptr;
}

EGLConfig ChooseFirstConfigForAttributes(EGLDisplay display,
                                         EGLint* config_attribs) {
  EGLint num_configs;
  if (!ValidateEglConfig(display, config_attribs, &num_configs)) {
    return nullptr;
  }

  EGLConfig config = nullptr;
  if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs)) {
    LOG(ERROR) << "eglChooseConfig failed with error "
               << ui::GetLastEGLErrorString();
    return nullptr;
  }

  return config;
}

EGLConfig ChooseRGBA8888Config(EGLDisplay display, EGLint renderable_type) {
  EGLint config_attribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, renderable_type,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
    EGL_NONE
  };

  return ChooseFirstConfigForAttributes(display, config_attribs);
}

EGLConfig ChooseRGBA8888NoPbufferConfig(EGLDisplay display,
                                        EGLint renderable_type) {
  EGLint config_attribs[] = {
    EGL_BUFFER_SIZE, 32,
    EGL_ALPHA_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_RENDERABLE_TYPE, renderable_type,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  return ChooseFirstConfigForAttributes(display, config_attribs);
}

EGLConfig ChooseConfig(EGLDisplay display, GLSurface::Format format) {
  static std::map<GLSurface::Format, EGLConfig> config_map;

  if (config_map.find(format) != config_map.end()) {
    return config_map[format];
  }

  EGLConfig config = nullptr;

  if (format == GLSurface::SURFACE_RGB565) {
    config = ChooseRGB565Config(display);
  }

  EGLint renderable_type = EGL_OPENGL_ES2_BIT;
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableUnsafeES3APIs)) {
    renderable_type = EGL_OPENGL_ES3_BIT;
  }

  if (!config) {
    config = ChooseRGBA8888Config(display, renderable_type);
  }

  if (!config) {
    config = ChooseRGBA8888NoPbufferConfig(display, renderable_type);
  }

  if (!config) {
    LOG(ERROR) << "No suitable EGL configs found!";
    return nullptr;
  }

  config_map[format] = config;
  return config;
}

}

class OxideSurfacelessEGL : public SurfacelessEGL {
 public:
  OxideSurfacelessEGL(const gfx::Size& size);

  bool Initialize() override;
  EGLConfig GetConfig() override;
};

class OxidePbufferGLSurfaceEGL : public PbufferGLSurfaceEGL {
 public:
  OxidePbufferGLSurfaceEGL(const gfx::Size& size);

  bool Initialize() override;
  EGLConfig GetConfig() override;
};

OxideSurfacelessEGL::OxideSurfacelessEGL(const gfx::Size& size)
    : SurfacelessEGL(size) {}

bool OxideSurfacelessEGL::Initialize() {
  GLSurface::Format format = SURFACE_DEFAULT;
  if (base::SysInfo::IsLowEndDevice()) {
    format = SURFACE_RGB565;
  }
  return SurfacelessEGL::Initialize(format);
}

EGLConfig OxideSurfacelessEGL::GetConfig() {
  if (!config_) {
    config_ = ChooseConfig(GetDisplay(), format_);
  }
  return config_;
}

OxidePbufferGLSurfaceEGL::OxidePbufferGLSurfaceEGL(const gfx::Size& size)
    : PbufferGLSurfaceEGL(size) {}

bool OxidePbufferGLSurfaceEGL::Initialize() {
  GLSurface::Format format = SURFACE_DEFAULT;
  if (base::SysInfo::IsLowEndDevice()) {
    format = SURFACE_RGB565;
  }
  return PbufferGLSurfaceEGL::Initialize(format);
}

EGLConfig OxidePbufferGLSurfaceEGL::GetConfig() {
  if (!config_) {
    config_ = ChooseConfig(GetDisplay(), format_);
  }
  return config_;
}

bool GLSurface::InitializeOneOffInternal() {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL: {
      if (!GLSurfaceGLX::InitializeOneOff()) {
        LOG(ERROR) << "GLSurfaceGLX::InitializeOneOff failed.";
        return false;
      }
      break;
    }

    case kGLImplementationEGLGLES2: {
      if (!GLSurfaceEGL::InitializeOneOff()) {
        LOG(ERROR) << "GLSurfaceEGL::InitializeOneOff failed.";
        return false;
      }
      break;
    }

    case kGLImplementationOSMesaGL:
      return true;

    default:
      return false;
  }

  return true;
}

scoped_refptr<GLSurface> GLSurface::CreateSurfacelessViewGLSurface(
    gfx::AcceleratedWidget window) {
  NOTREACHED();
  return nullptr;
}

scoped_refptr<GLSurface> GLSurface::CreateViewGLSurface(
    gfx::AcceleratedWidget window) {
  NOTREACHED();
  return nullptr;
}

scoped_refptr<GLSurface> GLSurface::CreateOffscreenGLSurface(
    const gfx::Size& size) {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL: {
      scoped_refptr<GLSurface> surface(
          new UnmappedNativeViewGLSurfaceGLX(size));
      if (!surface->Initialize()) {
        return nullptr;
      }

      return surface;
    }

    case kGLImplementationEGLGLES2: {
      scoped_refptr<GLSurface> surface;
      if (GLSurfaceEGL::IsEGLSurfacelessContextSupported() &&
          size.width() == 0 && size.height() == 0) {
        surface = new OxideSurfacelessEGL(size);
      } else {
        surface = new OxidePbufferGLSurfaceEGL(size);
      }
      if (!surface->Initialize()) {
        return nullptr;
      }

      return surface;
    }

    case kGLImplementationOSMesaGL: {
      scoped_refptr<GLSurface> surface(
          new GLSurfaceOSMesa(GLSurface::SURFACE_OSMESA_BGRA, size));
      if (!surface->Initialize()) {
        return nullptr;
      }

      return surface;
    }

    default:
      return nullptr;
  }
}

EGLNativeDisplayType GetPlatformDefaultEGLNativeDisplay() {
  return ui::OzonePlatform::GetInstance()
      ->GetSurfaceFactoryOzone()
      ->GetNativeDisplay();
}

} // namespace gfx
