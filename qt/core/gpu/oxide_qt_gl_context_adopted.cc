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

#include "oxide_qt_gl_context_adopted.h"

#include <QGuiApplication>
#include <QString>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "base/logging.h"

namespace oxide {
namespace qt {

GLContextAdopted::GLContextAdopted(void* handle,
                                   gfx::GLImplementation implementation)
    : oxide::GLContextAdopted(handle, false),
      implementation_(implementation) {}

// static
scoped_refptr<GLContextAdopted> GLContextAdopted::Create(
    QOpenGLContext* context) {
  if (!context) {
    return scoped_refptr<GLContextAdopted>();
  }

  QString platform = QGuiApplication::platformName();

  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  if (!pni) {
    LOG(WARNING)
        << "Unable to create adopted GL context for platform: "
        << qPrintable(platform) << " - no QPlatformNativeInterface";
    return scoped_refptr<GLContextAdopted>();
  }

  if (platform == "xcb") {
    // QXcbNativeInterface creates a GLXContext if GLX is enabled, else
    // it creates an EGLContext is EGL is enabled, so this should be safe
    // XXX: Check this matches the GL implementation selected by Chrome?
    void* handle = pni->nativeResourceForContext("glxcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationDesktopGL));
    }

    handle = pni->nativeResourceForContext("eglcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationEGLGLES2));
    }
  } else if (platform.startsWith("ubuntu")) {
    void* handle = pni->nativeResourceForContext("eglcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationEGLGLES2));
    }
  } else {
    LOG(WARNING)
        << "Unable to create adopted GL context for platform: "
        << qPrintable(platform) << " - unrecognized platform";
    return scoped_refptr<GLContextAdopted>();
  }

  LOG(ERROR)
      << "Unable to create adopted GL context for platform: "
      << qPrintable(platform) << " - unexpected result from "
      << "QPlatformNativeInterface::nativeResourceForContext";

  return scoped_refptr<GLContextAdopted>();
}

} // namespace qt
} // namespace oxide
