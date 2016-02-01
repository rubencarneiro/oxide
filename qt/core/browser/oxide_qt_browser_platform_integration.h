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

#ifndef _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_
#define _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_

#include <QObject>
#include <QtGlobal>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"

#include "shared/browser/oxide_browser_platform_integration.h"

QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE;

namespace oxide {
namespace qt {

class BrowserPlatformIntegration : public QObject,
                                   public oxide::BrowserPlatformIntegration {
  Q_OBJECT

 public:
  BrowserPlatformIntegration();
  ~BrowserPlatformIntegration() override;

 private Q_SLOTS:
  void OnApplicationStateChanged();
  void OnClipboardDataChanged();
  void OnScreenGeometryChanged(const QRect& geometry);
  void OnScreenOrientationChanged(Qt::ScreenOrientation orientation);

 private:
  void UpdateApplicationState();
  void UpdateDefaultScreenInfo();

  // oxide::BrowserPlatformIntegration implementation
  bool LaunchURLExternally(const GURL& url) override;
  bool IsTouchSupported() override;
  intptr_t GetNativeDisplay() override;
  blink::WebScreenInfo GetDefaultScreenInfo() override;
  oxide::GLContextDependent* GetGLShareContext() override;
  scoped_ptr<oxide::MessagePump> CreateUIMessagePump() override;
  ui::Clipboard* CreateClipboard() override;
  void BrowserThreadInit(content::BrowserThread::ID id) override;
  scoped_ptr<content::LocationProvider> CreateLocationProvider() override;
  ApplicationState GetApplicationState() override;
  std::string GetApplicationName() override;
  void CreateVibrationManager(
      mojo::InterfaceRequest<device::VibrationManager> request) override;

  // QObject implementation
  bool eventFilter(QObject* watched, QEvent* event) override;

  // QCoreApplication::applicationName() does no locking and QString isn't
  // thread-safe. As GetApplicationName can be called on any thread, we cache
  // the app name here. This protects against the case where we call
  // QCoreApplication::applicationName() off the UI thread whilst the
  // application is in QCoreApplication::setApplicationName. We don't expect
  // this to change, so we don't bother listening for updates
  const std::string application_name_;

  // Whether the application is suspended. If the Qt platform is ubuntu,
  // we detect Qt::ApplicationSuspended synthetically because it doesn't
  // set applicatonState accordingly (we get Qt::ApplicationInactive when
  // the app is unfocused, but that doesn't necessarily mean the app will
  // be suspended)
  // See https://launchpad.net/bugs/1456706
  bool suspended_;

  // The current application state. We track this here because we have 2
  // sources for state changes, and we want to ensure we only notify observers
  // when the state really does change
  ApplicationState state_;

  base::Lock default_screen_info_lock_;
  blink::WebScreenInfo default_screen_info_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPlatformIntegration);
};

QThread* GetIOQThread();

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_PLATFORM_INTEGRATION_H_
