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
                                   gfx::GLImplementation implementation,
                                   gfx::GLShareGroup* share_group)
    : oxide::GLContextAdopted(share_group),
      handle_(handle),
      implementation_(implementation) {}

// static
scoped_refptr<GLContextAdopted> GLContextAdopted::Create(
    QOpenGLContext* context,
    gfx::GLShareGroup* share_group) {
  if (!context) {
    return scoped_refptr<GLContextAdopted>();
  }

  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  if (!pni) {
    return scoped_refptr<GLContextAdopted>();
  }

  QString platform = QGuiApplication::platformName();
  if (platform == "xcb") {
    // QXcbNativeInterface creates a GLXContext if GLX is enabled, else
    // it creates an EGLContext is EGL is enabled, so this should be safe
    // XXX: Check this matches the GL implementation selected by Chrome?
    void* handle = pni->nativeResourceForContext("glxcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationDesktopGL,
                               share_group));
    }

    handle = pni->nativeResourceForContext("eglcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationEGLGLES2,
                               share_group));
    }
  } else if (platform.startsWith("ubuntu")) {
    void* handle = pni->nativeResourceForContext("eglcontext", context);
    if (handle) {
      return make_scoped_refptr(
          new GLContextAdopted(handle,
                               gfx::kGLImplementationEGLGLES2,
                               share_group));
    }
  } else {
    DLOG(WARNING) << "Unrecognized platform: " << qPrintable(platform);
  }

  LOG(ERROR) << "Failed to determine native GL context for platform: "
             << qPrintable(platform);

  return scoped_refptr<GLContextAdopted>();
}

void* GLContextAdopted::GetHandle() {
  return handle_;
}

gfx::GLImplementation GLContextAdopted::GetImplementation() const {
  return implementation_;
}

} // namespace qt
} // namespace oxide
