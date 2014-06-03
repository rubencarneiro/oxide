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

#include "oxide_qt_shared_gl_context.h"

#include <QGuiApplication>
#include <QString>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "base/logging.h"

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

namespace oxide {
namespace qt {

SharedGLContext::SharedGLContext(QOpenGLContext* context) :
    handle_(NULL),
    implementation_(gfx::kGLImplementationNone) {
  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  QString platform = QGuiApplication::platformName();
  if (platform == "xcb") {
    // QXcbNativeInterface creates a GLXContext if GLX is enabled, else
    // it creates an EGLContext is EGL is enabled, so this should be safe
    // XXX: Check this matches the GL implementation selected by Chrome?
    implementation_ = gfx::kGLImplementationDesktopGL;
    handle_ = pni->nativeResourceForContext("glxcontext", context);
    if (!handle_) {
      implementation_ = gfx::kGLImplementationEGLGLES2;
      handle_ = pni->nativeResourceForContext("eglcontext", context);
    }
    if (!handle_) {
      implementation_ = gfx::kGLImplementationNone;
    }
  } else if (platform.startsWith("ubuntu") {
    handle_ = pni->nativeResourceForContext("eglcontext", context);
    if (handle_) {
      implementation_ = gfx::kGLImplementationEGLGLES2;
    }
  } else {
    DLOG(WARNING) << "Unrecognized platform: " << qPrintable(platform);
  }
}

// static
scoped_refptr<SharedGLContext> SharedGLContext::Create() {
  QOpenGLContext* qcontext = WebContextAdapter::sharedGLContext();
  if (!qcontext) {
    return NULL;
  }

  scoped_refptr<SharedGLContext> context(new SharedGLContext(qcontext));
  if (!context->GetHandle()) {
    DLOG(WARNING) << "Could not determine native handle for shared GL context";
    return NULL;
  }

  return context;
}

} // namespace qt
} // namespace oxide
