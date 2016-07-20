// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "integration.h"

#include <QCoreApplication>
#include <QScreen>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QVariant>

#if defined(OS_LINUX)
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#endif

#include "backingstore.h"
#include "screen.h"

namespace {
const char* kShimApiKey = "_oxide_mock_qpa_shim_api";
MockPlatformIntegration* g_instance;
}

QPlatformWindow* MockPlatformIntegration::createPlatformWindow(
    QWindow* window) const {
  return new QPlatformWindow(window);
}

QPlatformBackingStore* MockPlatformIntegration::createPlatformBackingStore(
    QWindow* window) const {
  return new MockBackingStore(window);
}

QAbstractEventDispatcher*
MockPlatformIntegration::createEventDispatcher() const {
#if defined(OS_LINUX)
  return createUnixEventDispatcher();
#else
# error "createEventDispatcher is not implemented for this platform"
#endif
}

MockPlatformIntegration::MockPlatformIntegration() {
  Q_ASSERT(!g_instance);
  g_instance = this;

  QCoreApplication::instance()->setProperty(kShimApiKey,
                                            QVariant::fromValue(&shim_));

  QWindowSystemInterface::setSynchronousWindowsSystemEvents(true);

  MockScreen* screen =
      new MockScreen(QRect(0, 0, 540,960), QRect(0, 50, 540, 910), 32,
                     QImage::Format_ARGB32_Premultiplied, 2.f);
  screenAdded(screen, true);
  screens_.push_back(screen);

  screen = new MockScreen(QRect(540, 0, 1920, 1080), QRect(540, 50, 1920, 1030),
                          32, QImage::Format_ARGB32_Premultiplied, 1.f);
  screenAdded(screen, false);
  screens_.push_back(screen);
}

MockPlatformIntegration::~MockPlatformIntegration() {
  while (screens_.size() > 0) {
    MockScreen* screen = screens_.front();
    screens_.pop_front();
    destroyScreen(screen);
  }

  g_instance = nullptr;
}

// static
MockPlatformIntegration* MockPlatformIntegration::instance() {
  return g_instance;
}

void MockPlatformIntegration::overrideScreenGeometry(
    QScreen* screen,
    const QRect& geometry,
    const QRect& available_geometry) {
  static_cast<MockScreen*>(screen->handle())->overrideGeometry(
      geometry, available_geometry);
}

void MockPlatformIntegration::setScreenOrientation(
    QScreen* screen,
    Qt::ScreenOrientation orientation) {
  static_cast<MockScreen*>(screen->handle())->setOrientation(orientation);
}

void MockPlatformIntegration::resetScreens() {
  for (auto screen : screens_) {
    screen->reset();
  }
}
