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
#include "platformnativeinterface.h"
#include "screen.h"

namespace {
const char* kShimApiKey = "_oxide_mock_qpa_shim_api";
MockPlatformIntegration* g_instance;
}

void MockPlatformIntegration::freeScreens() {
  while (screens_.size() > 0) {
    MockScreen* screen = screens_.front();
    screens_.pop_front();
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    destroyScreen(screen);
#else
    delete screen;
#endif
  }
}

void MockPlatformIntegration::initializeScreens() {
  Q_ASSERT(screens_.isEmpty());

  MockScreen* screen =
      new MockScreen(0, QRect(0, 0, 1080, 1920), QRect(0, 50, 1080, 1870), 32,
                     QImage::Format_ARGB32_Premultiplied, 2.f, 1);
  screens_.push_back(screen);
  screenAdded(screen);

  screen =
      new MockScreen(1, QRect(1080, 0, 1920, 1080), QRect(0, 25, 1920, 1055),
                     32, QImage::Format_ARGB32_Premultiplied, 1.f, 3);
  screens_.push_back(screen);
  screenAdded(screen);

  screen =
      new MockScreen(2, QRect(3000, 0, 3840, 2160), QRect(0, 50, 3840, 2110),
                     32, QImage::Format_ARGB32_Premultiplied, 2.f, 4);
  screens_.push_back(screen);
  screenAdded(screen);
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

QPlatformNativeInterface* MockPlatformIntegration::nativeInterface() const {
  return native_interface_.data();
}

MockPlatformIntegration::MockPlatformIntegration()
    : native_interface_(new MockPlatformNativeInterface()) {
  Q_ASSERT(!g_instance);
  g_instance = this;

  QCoreApplication::instance()->setProperty(kShimApiKey,
                                            QVariant::fromValue(&shim_));

  QWindowSystemInterface::setSynchronousWindowsSystemEvents(true);

  initializeScreens();
}

MockPlatformIntegration::~MockPlatformIntegration() {
  freeScreens();
  g_instance = nullptr;
}

// static
MockPlatformIntegration* MockPlatformIntegration::instance() {
  return g_instance;
}

void MockPlatformIntegration::resetScreens() {
  freeScreens();
  initializeScreens();
}

void MockPlatformIntegration::screenPropertyChanged(
    QPlatformScreen* screen,
    const QString& property_name) {
  Q_EMIT native_interface_->screenPropertyChanged(screen, property_name);
}
