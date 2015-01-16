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

#include "gl_implementation_oxide.h"
#include "ui/gl/gl_implementation.h"

#include <vector>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/native_library.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_glx_api_implementation.h"
#include "ui/gl/gl_implementation_osmesa.h"
#include "ui/gl/gl_osmesa_api_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "ui/ozone/public/ozone_platform.h"

namespace gfx {

namespace {

GLImplementation g_preferred_impl = kGLImplementationNone;
base::LazyInstance<std::vector<GLImplementation> > g_allowed_impls =
    LAZY_INSTANCE_INITIALIZER;

void GL_BINDING_CALL MarshalClearDepthToClearDepthf(GLclampd depth) {
  glClearDepthf(static_cast<GLclampf>(depth));
}

void GL_BINDING_CALL MarshalDepthRangeToDepthRangef(GLclampd z_near,
                                                    GLclampd z_far) {
  glDepthRangef(static_cast<GLclampf>(z_near), static_cast<GLclampf>(z_far));
}

bool IsSupportedGLImplementation(GLImplementation implementation) {
  switch (implementation) {
    case kGLImplementationDesktopGL:
    case kGLImplementationEGLGLES2:
    case kGLImplementationOSMesaGL:
      return true;
    default:
      return false;
  }
}

} // namespace

void InitializePreferredGLImplementation(GLImplementation implementation) {
  DCHECK_EQ(GetGLImplementation(), kGLImplementationNone);
  DCHECK(IsSupportedGLImplementation(implementation));
  g_preferred_impl = implementation;
}

void InitializeAllowedGLImplementations(
    const std::vector<GLImplementation>& implementations) {
  DCHECK_EQ(GetGLImplementation(), kGLImplementationNone);
#ifndef NDEBUG
  for (std::vector<GLImplementation>::const_iterator it =
           implementations.cbegin();
       it != implementations.cend(); ++it) {
    DCHECK(IsSupportedGLImplementation(*it));
  }
#endif
  g_allowed_impls.Get() = implementations;
}

void GetAllowedGLImplementations(std::vector<GLImplementation>* impls) {
  if (g_allowed_impls.Get().size() == 0) {
    impls->push_back(kGLImplementationOSMesaGL);
  } else {
    *impls = g_allowed_impls.Get();
  }

  if (g_preferred_impl == kGLImplementationNone) {
    return;
  }

  DCHECK(std::find(impls->begin(), impls->end(), g_preferred_impl) !=
         impls->end());
  if (impls->front() == g_preferred_impl) {
    return;
  }

  impls->insert(impls->begin(), g_preferred_impl);
  for (std::vector<GLImplementation>::iterator it = impls->begin() + 1;
       it != impls->end(); ++it) {
    if (*it == g_preferred_impl) {
      impls->erase(it);
      break;
    }
  }
}

bool InitializeStaticGLBindings(GLImplementation implementation) {
  if (GetGLImplementation() != kGLImplementationNone) {
    return true;
  }

  ui::OzonePlatform::InitializeForGPU();

  switch (implementation) {
    case kGLImplementationDesktopGL: {
      base::NativeLibrary library =
          base::LoadNativeLibrary(base::FilePath("libGL.so.1"), nullptr);
      if (!library) {
        return false;
      }

      GLGetProcAddressProc get_proc_address =
          reinterpret_cast<GLGetProcAddressProc>(
            base::GetFunctionPointerFromNativeLibrary(library,
                                                      "glXGetProcAddress"));
      if (!get_proc_address) {
        LOG(ERROR) << "glxGetProcAddress not found.";
        base::UnloadNativeLibrary(library);
        return false;
      }

      SetGLGetProcAddressProc(get_proc_address);
      AddGLNativeLibrary(library);
      SetGLImplementation(kGLImplementationDesktopGL);

      InitializeStaticGLBindingsGL();
      InitializeStaticGLBindingsGLX();
      break;
    }

    case kGLImplementationEGLGLES2: {
      base::NativeLibrary gles_library =
          base::LoadNativeLibrary(base::FilePath("libGLESv2.so.2"), nullptr);
      if (!gles_library) {
        return false;
      }

      base::NativeLibrary egl_library =
          base::LoadNativeLibrary(base::FilePath("libEGL.so.1"), nullptr);
      if (!egl_library) {
        base::UnloadNativeLibrary(gles_library);
        return false;
      }

      GLGetProcAddressProc get_proc_address =
          reinterpret_cast<GLGetProcAddressProc>(
            base::GetFunctionPointerFromNativeLibrary(egl_library,
                                                      "eglGetProcAddress"));
      if (!get_proc_address) {
        LOG(ERROR) << "eglGetProcAddress not found.";
        base::UnloadNativeLibrary(egl_library);
        base::UnloadNativeLibrary(gles_library);
        return false;
      }

      SetGLGetProcAddressProc(get_proc_address);
      AddGLNativeLibrary(egl_library);
      AddGLNativeLibrary(gles_library);
      SetGLImplementation(kGLImplementationEGLGLES2);

      InitializeStaticGLBindingsGL();
      InitializeStaticGLBindingsEGL();

      // These two functions take single precision float rather than double
      // precision float parameters in GLES.
      ::gfx::g_driver_gl.fn.glClearDepthFn = MarshalClearDepthToClearDepthf;
      ::gfx::g_driver_gl.fn.glDepthRangeFn = MarshalDepthRangeToDepthRangef;
      break;
    }

    case kGLImplementationOSMesaGL:
      return InitializeStaticGLBindingsOSMesaGL();

    default:
      NOTIMPLEMENTED();
      return false;
  }

  return true;
}

bool InitializeDynamicGLBindings(GLImplementation implementation,
                                 GLContext* context) {
  switch (implementation) {
    case kGLImplementationDesktopGL: {
      InitializeDynamicGLBindingsGL(context);
      InitializeDynamicGLBindingsGLX(context);
      break;
    }

    case kGLImplementationEGLGLES2: {
      InitializeDynamicGLBindingsGL(context);
      InitializeDynamicGLBindingsEGL(context);
      break;
    }

    case kGLImplementationOSMesaGL:
      InitializeDynamicGLBindingsGL(context);
      InitializeDynamicGLBindingsOSMESA(context);
      break;

    default:
      return false;
  }

  return true;
}

void InitializeDebugGLBindings() {
  InitializeDebugGLBindingsGL();
  InitializeDebugGLBindingsEGL();
  InitializeDebugGLBindingsGLX();
  InitializeDebugGLBindingsOSMESA();
}

void ClearGLBindings() {
  ClearGLBindingsGL();
  ClearGLBindingsEGL();
  ClearGLBindingsGLX();
  ClearGLBindingsOSMESA();

  SetGLImplementation(kGLImplementationNone);

  UnloadGLNativeLibraries();
}

bool GetGLWindowSystemBindingInfo(GLWindowSystemBindingInfo* info) {
  switch (GetGLImplementation()) {
    case kGLImplementationDesktopGL:
      return GetGLWindowSystemBindingInfoGLX(info);
    case kGLImplementationEGLGLES2:
      return GetGLWindowSystemBindingInfoEGL(info);
    default:
      return false;
  }
}

} // namespace gfx
