// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_qt_browser_platform_integration.h"

#include <QDesktopServices>
#include <QGuiApplication>
#include <QPointer>
#include <QString>
#include <QThread>
#include <QTouchDevice>
#include <QUrl>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "base/lazy_instance.h"
#include "base/message_loop/message_loop_proxy.h"
#include "url/gurl.h"

#include "qt/core/base/oxide_qt_screen_utils.h"
#include "qt/core/gl/oxide_qt_gl_context_adopted.h"
#include "qt/core/glue/oxide_qt_init.h"

#include "oxide_qt_browser_thread_q_event_dispatcher.h"
#include "oxide_qt_location_provider.h"
#include "oxide_qt_message_pump.h"

namespace oxide {
namespace qt {

namespace {
base::LazyInstance<QPointer<QThread> > g_io_thread;
}

BrowserPlatformIntegration::BrowserPlatformIntegration(
    GLContextAdopted* gl_share_context)
    : QObject()
    , gl_share_context_(gl_share_context) {
  QObject::connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
                   this, SLOT(onApplicationStateChanged(Qt::ApplicationState)));
}

BrowserPlatformIntegration::~BrowserPlatformIntegration() {}

void BrowserPlatformIntegration::onApplicationStateChanged(
    Qt::ApplicationState state) {
  BrowserPlatformIntegration::ApplicationState oxide_state =
      (state == Qt::ApplicationActive) ?
          BrowserPlatformIntegration::APPLICATION_STATE_ACTIVE :
          BrowserPlatformIntegration::APPLICATION_STATE_INACTIVE;
  NotifyApplicationStateChanged(oxide_state);
}

bool BrowserPlatformIntegration::LaunchURLExternally(const GURL& url) {
  return QDesktopServices::openUrl(QUrl(QString::fromStdString(url.spec())));
}

bool BrowserPlatformIntegration::IsTouchSupported() {
  // XXX: Is there a way to get notified if a touch device is added?
  return QTouchDevice::devices().size() > 0;
}

intptr_t BrowserPlatformIntegration::GetNativeDisplay() {
  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  if (!pni) {
    return 0;
  }

  return reinterpret_cast<intptr_t>(
      pni->nativeResourceForScreen("display",
                                   QGuiApplication::primaryScreen()));
}

blink::WebScreenInfo BrowserPlatformIntegration::GetDefaultScreenInfo() {
  return GetWebScreenInfoFromQScreen(QGuiApplication::primaryScreen());
}

oxide::GLContextAdopted* BrowserPlatformIntegration::GetGLShareContext() {
  return gl_share_context_.get();
}

scoped_ptr<oxide::MessagePump>
BrowserPlatformIntegration::CreateUIMessagePump() {
  return make_scoped_ptr(new MessagePump());
}

void BrowserPlatformIntegration::BrowserThreadInit(
    content::BrowserThread::ID id) {
  if (id != content::BrowserThread::IO) {
    return;
  }

  QThread* thread = QThread::currentThread();
  thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::MessageLoopProxy::current()));
  g_io_thread.Get() = thread;
}

content::LocationProvider*
BrowserPlatformIntegration::CreateLocationProvider() {
  // Give the geolocation thread a Qt event dispatcher, so that we can use
  // Queued signals / slots between it and the IO thread
  QThread* thread = QThread::currentThread();
  if (!thread->eventDispatcher()) {
    thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::MessageLoopProxy::current()));
  }

  return new LocationProvider();
}

QThread* GetIOQThread() {
  return g_io_thread.Get();
}

} // namespace qt
} // namespace oxide
