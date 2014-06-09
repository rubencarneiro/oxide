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

#include "ui/gl/gl_surface.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/gl_surface_glx.h"
#include "ui/gl/gl_surface_osmesa.h"

namespace gfx {

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

scoped_refptr<GLSurface> GLSurface::CreateViewGLSurface(
    gfx::AcceleratedWidget window) {
  NOTREACHED();
  return NULL;
}


scoped_refptr<GLSurface> GLSurface::CreateOffscreenGLSurface(
    const gfx::Size& size) {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL: {
      scoped_refptr<GLSurface> surface(new PbufferGLSurfaceGLX(size));
      if (!surface->Initialize()) {
        return NULL;
      }

      return surface;
    }

    case kGLImplementationEGLGLES2: {
      scoped_refptr<GLSurface> surface;
      if (GLSurfaceEGL::IsEGLSurfacelessContextSupported() &&
          size.width() == 0 && size.height() == 0) {
        surface = new SurfacelessEGL(size);
      } else {
        surface = new PbufferGLSurfaceEGL(size);
      }
      if (!surface->Initialize()) {
        return NULL;
      }

      return surface;
    }

    case kGLImplementationOSMesaGL: {
      scoped_refptr<GLSurface> surface(new GLSurfaceOSMesa(1, size));
      if (!surface->Initialize()) {
        return NULL;
      }

      return surface;
    }

    default:
      return NULL;
  }
}

EGLNativeDisplayType GetPlatformDefaultEGLNativeDisplay() {
  return SurfaceFactoryOzone::GetInstance()->GetNativeDisplay();
}

} // namespace gfx
