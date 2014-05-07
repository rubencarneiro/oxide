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

#include "ui/gl/gl_implementation.h"

#include <vector>

#include "base/bind.h"
#include "base/logging.h"
#include "base/native_library.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_glx_api_implementation.h"
#include "ui/gl/gl_implementation_osmesa.h"
#include "ui/gl/gl_osmesa_api_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "ui/ozone/ozone_platform.h"

#include "shared/browser/oxide_browser_process_main.h"

#include "oxide_gl_implementation.h"
#include "oxide_shared_gl_context.h"

namespace gfx {

namespace {

void GL_BINDING_CALL MarshalClearDepthToClearDepthf(GLclampd depth) {
  glClearDepthf(static_cast<GLclampf>(depth));
}

void GL_BINDING_CALL MarshalDepthRangeToDepthRangef(GLclampd z_near,
                                                    GLclampd z_far) {
  glDepthRangef(static_cast<GLclampf>(z_near), static_cast<GLclampf>(z_far));
}

} // namespace

void GetAllowedGLImplementations(std::vector<GLImplementation>* impls) {
  std::vector<GLImplementation> supported;
  supported.push_back(kGLImplementationDesktopGL);
  supported.push_back(kGLImplementationEGLGLES2);
  supported.push_back(kGLImplementationOSMesaGL);

  oxide::GetAllowedGLImplementations(impls);

  if (impls->size() == 0) {
    impls->push_back(kGLImplementationOSMesaGL);
  } else {
    for (std::vector<GLImplementation>::iterator it = impls->begin();
         it != impls->end(); ++it) {
      DCHECK(std::find(supported.begin(), supported.end(), *it) !=
             supported.end());
    }
  }

  GLShareGroup* group = NULL;
  oxide::SharedGLContext* context =
      oxide::BrowserProcessMain::instance()->shared_gl_context();
  if (context) {
    group = context->share_group();
  }
  if (group) {
    GLImplementation preferred = context->GetImplementation();
    DCHECK(std::find(impls->begin(), impls->end(), preferred) !=
           impls->end());
    if (impls->front() != preferred) {
      impls->insert(impls->begin(), preferred);
      for (std::vector<GLImplementation>::iterator it = impls->begin() + 1;
           it != impls->end(); ++it) {
        if (*it == preferred) {
          impls->erase(it);
          break;
        }
      }
    }
  }
}

bool InitializeStaticGLBindings(GLImplementation implementation) {
  if (GetGLImplementation() != kGLImplementationNone) {
    return true;
  }

  ui::OzonePlatform::Initialize();

  switch (implementation) {
    case kGLImplementationDesktopGL: {
      base::NativeLibrary library = LoadLibraryAndPrintError("libGL.so.1");
      if (!library) {
        return false;
      }

      GLGetProcAddressProc get_proc_address =
          reinterpret_cast<GLGetProcAddressProc>(
            base::GetFunctionPointerFromNativeLibrary(
              library, "glXGetProcAddress"));
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
      if (!SurfaceFactoryOzone::GetInstance()->LoadEGLGLES2Bindings(
              base::Bind(&AddGLNativeLibrary),
              base::Bind(&SetGLGetProcAddressProc))) {
        return false;
      }

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
