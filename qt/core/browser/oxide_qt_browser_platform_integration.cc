// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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
#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QScreen>
#include <QString>
#include <QThread>
#include <QTouchDevice>
#include <QUrl>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "base/lazy_instance.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

#include "qt/core/glue/oxide_qt_init.h"
#include "qt/core/gpu/oxide_qt_gl_context_dependent.h"

#include "oxide_qt_browser_startup.h"
#include "oxide_qt_browser_thread_q_event_dispatcher.h"
#include "oxide_qt_clipboard.h"
#include "oxide_qt_location_provider.h"
#include "oxide_qt_message_pump.h"
#include "oxide_qt_screen_utils.h"

namespace oxide {
namespace qt {

namespace {
base::LazyInstance<QPointer<QThread> > g_io_thread;

void LaunchURLExternallyOnUIThread(const GURL& url) {
  QDesktopServices::openUrl(QUrl(QString::fromStdString(url.spec())));
}

oxide::BrowserPlatformIntegration::ApplicationState
CalculateApplicationState(bool suspended) {
  switch (qApp->applicationState()) {
    case Qt::ApplicationSuspended:
      return oxide::BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED;
    case Qt::ApplicationHidden:
      return oxide::BrowserPlatformIntegration::APPLICATION_STATE_INACTIVE;
    case Qt::ApplicationInactive:
      if (suspended) {
        return oxide::BrowserPlatformIntegration::APPLICATION_STATE_SUSPENDED;
      }
      return oxide::BrowserPlatformIntegration::APPLICATION_STATE_INACTIVE;
    case Qt::ApplicationActive:
      return oxide::BrowserPlatformIntegration::APPLICATION_STATE_ACTIVE;
    default:
      NOTREACHED();
      return oxide::BrowserPlatformIntegration::APPLICATION_STATE_ACTIVE;
  }
}

}

void BrowserPlatformIntegration::OnApplicationStateChanged() {
  UpdateApplicationState();
}

void BrowserPlatformIntegration::OnScreenGeometryChanged(
    const QRect& geometry) {
  UpdateDefaultScreenInfo();
}

void BrowserPlatformIntegration::OnScreenOrientationChanged(
    Qt::ScreenOrientation orientation) {
  UpdateDefaultScreenInfo();
}

void BrowserPlatformIntegration::UpdateApplicationState() {
  ApplicationState state = CalculateApplicationState(suspended_);
  if (state == state_) {
    return;
  }

  state_ = state;

  NotifyApplicationStateChanged();
}

void BrowserPlatformIntegration::UpdateDefaultScreenInfo() {
  base::AutoLock lock(default_screen_info_lock_);
  default_screen_info_ =
      GetWebScreenInfoFromQScreen(QGuiApplication::primaryScreen());
}

bool BrowserPlatformIntegration::LaunchURLExternally(const GURL& url) {
  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&LaunchURLExternallyOnUIThread, url));
  } else {
    LaunchURLExternallyOnUIThread(url);
  }

  return true;
}

bool BrowserPlatformIntegration::IsTouchSupported() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  // XXX: Is there a way to get notified if a touch device is added?
  return QTouchDevice::devices().size() > 0;
}

intptr_t BrowserPlatformIntegration::GetNativeDisplay() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  if (!pni) {
    return 0;
  }

  return reinterpret_cast<intptr_t>(
      pni->nativeResourceForScreen("display",
                                   QGuiApplication::primaryScreen()));
}

blink::WebScreenInfo BrowserPlatformIntegration::GetDefaultScreenInfo() {
  base::AutoLock lock(default_screen_info_lock_);
  return default_screen_info_;
}

oxide::GLContextDependent* BrowserPlatformIntegration::GetGLShareContext() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return BrowserStartup::GetInstance()->shared_gl_context();
}

scoped_ptr<oxide::MessagePump>
BrowserPlatformIntegration::CreateUIMessagePump() {
  return make_scoped_ptr(new MessagePump());
}

ui::Clipboard* BrowserPlatformIntegration::CreateClipboard() {
  return new Clipboard();
}

void BrowserPlatformIntegration::BrowserThreadInit(
    content::BrowserThread::ID id) {
  if (id != content::BrowserThread::IO) {
    return;
  }

  QThread* thread = QThread::currentThread();
  thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::ThreadTaskRunnerHandle::Get()));
  g_io_thread.Get() = thread;
}

scoped_ptr<content::LocationProvider>
BrowserPlatformIntegration::CreateLocationProvider() {
  // Give the geolocation thread a Qt event dispatcher, so that we can use
  // Queued signals / slots between it and the IO thread
  QThread* thread = QThread::currentThread();
  if (!thread->eventDispatcher()) {
    thread->setEventDispatcher(
      new BrowserThreadQEventDispatcher(base::ThreadTaskRunnerHandle::Get()));
  }

  return make_scoped_ptr(new LocationProvider());
}

oxide::BrowserPlatformIntegration::ApplicationState
BrowserPlatformIntegration::GetApplicationState() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return state_;
}

std::string BrowserPlatformIntegration::GetApplicationName() {
  return application_name_;
}

bool BrowserPlatformIntegration::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::ApplicationActivate ||
      event->type() == QEvent::ApplicationDeactivate) {
    suspended_ = event->type() == QEvent::ApplicationDeactivate;
    UpdateApplicationState();
  }

  return QObject::eventFilter(watched, event);
}

BrowserPlatformIntegration::BrowserPlatformIntegration()
    : application_name_(qApp->applicationName().toStdString()),
      suspended_(false),
      state_(CalculateApplicationState(false)) {
  QScreen* primary_screen = QGuiApplication::primaryScreen();
  primary_screen->setOrientationUpdateMask(Qt::LandscapeOrientation |
                                           Qt::PortraitOrientation |
                                           Qt::InvertedLandscapeOrientation |
                                           Qt::InvertedPortraitOrientation);
  connect(primary_screen, SIGNAL(virtualGeometryChanged(const QRect&)),
          SLOT(OnScreenGeometryChanged(const QRect&)));
  connect(primary_screen, SIGNAL(geometryChanged(const QRect&)),
          SLOT(OnScreenGeometryChanged(const QRect&)));
  connect(primary_screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)),
          SLOT(OnScreenOrientationChanged(Qt::ScreenOrientation)));
  connect(primary_screen,
          SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)),
          SLOT(OnScreenOrientationChanged(Qt::ScreenOrientation)));

  UpdateDefaultScreenInfo();

  connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
          SLOT(OnApplicationStateChanged()));
  if (QGuiApplication::platformName().startsWith("ubuntu")) {
    qApp->installEventFilter(this);
  }

}

BrowserPlatformIntegration::~BrowserPlatformIntegration() {
  qApp->disconnect(this);
  qApp->removeEventFilter(this);
}

QThread* GetIOQThread() {
  return g_io_thread.Get();
}

} // namespace qt
} // namespace oxide
