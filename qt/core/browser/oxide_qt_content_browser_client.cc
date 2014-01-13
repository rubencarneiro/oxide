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

#include "oxide_qt_content_browser_client.h"

#include <QGuiApplication>
#include <QString>
#include <QtGui/qpa/qplatformnativeinterface.h>
#if defined(USE_X11)
#include <X11/Xlib.h>
#undef None
#endif

#include "base/logging.h"

#include "shared/browser/oxide_shared_gl_context.h"

#include "qt/core/glue/oxide_qt_web_context_adapter.h"

#include "oxide_qt_message_pump.h"
#include "oxide_qt_render_widget_host_view.h"

namespace oxide {
namespace qt {

namespace {

class SharedGLContext : public oxide::SharedGLContext {
 public:
  SharedGLContext(QOpenGLContext* context, oxide::GLShareGroup* share_group) :
      oxide::SharedGLContext(share_group),
      handle_(NULL) {
    QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
    QString platform = QGuiApplication::platformName();
    if (platform == "xcb") {
      // QXcbNativeInterface creates a GLXContext if GLX is enabled, else
      // it creates an EGLContext is EGL is enabled, so this should be safe
      // XXX: Check this matches the GL implementation selected by Chrome?
      handle_ = pni->nativeResourceForContext("glxcontext", context);
      if (!handle_) {
        handle_ = pni->nativeResourceForContext("eglcontext", context);
      }
    } else {
      DLOG(WARNING) << "Unsupported platform: " << qPrintable(platform);
    }
  }

  void* GetHandle() OVERRIDE { return handle_; }

 private:
  void* handle_;
};

} // namespace

void ContentBrowserClient::GetDefaultScreenInfoImpl(
    blink::WebScreenInfo* result) {
  RenderWidgetHostView::GetScreenInfo(NULL, result);
}

scoped_refptr<gfx::GLContext> ContentBrowserClient::CreateSharedGLContext(
    oxide::GLShareGroup* share_group) {
  QOpenGLContext* qcontext = WebContextAdapter::sharedGLContext();
  if (!qcontext) {
    return NULL;
  }

  scoped_refptr<gfx::GLContext> context =
      new SharedGLContext(qcontext, share_group);
  if (!context->GetHandle()) {
    DLOG(WARNING) << "No native handle for shared GL context. "
                  << "Compositing will not work";
    context = NULL;
  }

  return context;
}

base::MessagePump* ContentBrowserClient::CreateMessagePumpForUI() {
  return new MessagePump();
}

#if defined (USE_X11)
Display* ContentBrowserClient::GetDefaultXDisplay() {
  static Display* display = NULL;
  if (!display) {
    QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
    display = static_cast<Display *>(
        pni->nativeResourceForScreen("display",
                                     QGuiApplication::primaryScreen()));
  }

  if (!display) {
    display = XOpenDisplay(NULL);
  }

  return display;
}
#endif

} // namespace qt
} // namespace oxide
