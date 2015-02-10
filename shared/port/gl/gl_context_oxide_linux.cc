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

#include "ui/gl/gl_context.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "ui/gl/gl_context_egl.h"
#include "ui/gl/gl_context_glx.h"
#include "ui/gl/gl_context_osmesa.h"
#include "ui/gl/gl_implementation.h"

namespace gfx {

// static
scoped_refptr<GLContext> GLContext::CreateGLContext(
    GLShareGroup* share_group,
    GLSurface* compatible_surface,
    GpuPreference gpu_preference) {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL: {
      scoped_refptr<GLContext> context(new GLContextGLX(share_group));
      if (!context->Initialize(compatible_surface, gpu_preference)) {
        return nullptr;
      }

      return context;
    }

    case kGLImplementationEGLGLES2: {
      scoped_refptr<GLContext> context(new GLContextEGL(share_group));
      if (!context->Initialize(compatible_surface, gpu_preference)) {
        return nullptr;
      }

      return context;
    }

    case kGLImplementationOSMesaGL: {
      scoped_refptr<GLContext> context(new GLContextOSMesa(share_group));
      if (!context->Initialize(compatible_surface, gpu_preference)) {
        return nullptr;
      }

      return context;
    }

    default:
      return nullptr;
  }
}

} // namespace gfx
