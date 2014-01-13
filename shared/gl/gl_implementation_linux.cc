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
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/native_library.h"
#include "ui/gfx/ozone/surface_factory_ozone.h"
#include "ui/gl/gl_egl_api_implementation.h"
#include "ui/gl/gl_gl_api_implementation.h"
#include "ui/gl/gl_glx_api_implementation.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/common/oxide_content_client.h"

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

// Load a library, printing an error message on failure.
base::NativeLibrary LoadLibrary(const base::FilePath& filename) {
  std::string error;
  base::NativeLibrary library = base::LoadNativeLibrary(filename, &error);
  if (!library) {
    DVLOG(1) << "Failed to load " << filename.MaybeAsASCII() << ": " << error;
    return NULL;
  }
  return library;
}

base::NativeLibrary LoadLibrary(const char* filename) {
  return LoadLibrary(base::FilePath(filename));
}

} // namespace

void GetAllowedGLImplementations(std::vector<GLImplementation>* impls) {
  std::vector<GLImplementation> supported;
  supported.push_back(kGLImplementationDesktopGL);
  supported.push_back(kGLImplementationEGLGLES2);

  oxide::ContentBrowserClient* client =
      oxide::ContentClient::instance()->browser();

  client->GetAllowedGLImplementations(impls);
  if (impls->size() == 0) {
    impls->push_back(kGLImplementationEGLGLES2);
  } else {
    for (std::vector<GLImplementation>::iterator it = impls->begin();
         it != impls->end(); ++it) {
      DCHECK(std::find(supported.begin(), supported.end(), *it) !=
             supported.end());
    }
  }

  GLShareGroup* group = client->GetGLShareGroup();
  if (group) {
    oxide::SharedGLContext* context =
        oxide::SharedGLContext::FromGfx(group->GetContext());
    if (context) {
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
}

bool InitializeGLBindings(GLImplementation implementation) {
  if (GetGLImplementation() != kGLImplementationNone) {
    return true;
  }

  switch (implementation) {
    case kGLImplementationDesktopGL: {
      base::NativeLibrary library = LoadLibrary("libGL.so.1");
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

      InitializeGLBindingsGL();
      InitializeGLBindingsGLX();
      break;
    }

    case kGLImplementationEGLGLES2: {
      if (!SurfaceFactoryOzone::GetInstance()->LoadEGLGLES2Bindings(
              base::Bind(&AddGLNativeLibrary),
              base::Bind(&SetGLGetProcAddressProc))) {
        return false;
      }

      SetGLImplementation(kGLImplementationEGLGLES2);

      InitializeGLBindingsGL();
      InitializeGLBindingsEGL();

      // These two functions take single precision float rather than double
      // precision float parameters in GLES.
      ::gfx::g_driver_gl.fn.glClearDepthFn = MarshalClearDepthToClearDepthf;
      ::gfx::g_driver_gl.fn.glDepthRangeFn = MarshalDepthRangeToDepthRangef;
      break;
    }

    default:
      NOTIMPLEMENTED();
      return false;
  }

  return true;
}

bool InitializeGLExtensionBindings(GLImplementation implementation,
                                   GLContext* context) {
  switch (implementation) {
    case kGLImplementationDesktopGL: {
      InitializeGLExtensionBindingsGL(context);
      InitializeGLExtensionBindingsGLX(context);
      break;
    }

    case kGLImplementationEGLGLES2: {
      InitializeGLExtensionBindingsGL(context);
      InitializeGLExtensionBindingsEGL(context);
      break;
    }

    default:
      return false;
  }

  return true;
}

void InitializeDebugGLBindings() {
  InitializeDebugGLBindingsGL();
  InitializeDebugGLBindingsEGL();
  InitializeDebugGLBindingsGLX();
}

void ClearGLBindings() {
  ClearGLBindingsGL();
  ClearGLBindingsEGL();
  ClearGLBindingsGLX();

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
