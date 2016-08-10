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

#ifndef _OXIDE_QT_TESTS_MOCK_QPA_INTEGRATION_H_
#define _OXIDE_QT_TESTS_MOCK_QPA_INTEGRATION_H_

#include <QtGlobal>
#include <Qt>
#include <QtGui/qpa/qplatformintegration.h>

#include "qt/tests/mock/qpa/shimapi.h"

QT_BEGIN_NAMESPACE
class QRect;
class QPlatformScreen;
class QScreen;
class QString;
QT_END_NAMESPACE

class MockPlatformNativeInterface;
class MockScreen;

class MockPlatformIntegration : public QPlatformIntegration {
 public:
  MockPlatformIntegration();
  ~MockPlatformIntegration() override;

  static MockPlatformIntegration* instance();

  QList<MockScreen*> screens() const { return screens_; }

  void resetScreens();
  void screenPropertyChanged(QPlatformScreen* screen,
                             const QString& property_name);

 private:
  void freeScreens();
  void initializeScreens();

  // QPlatformIntegration implementation
  QPlatformWindow* createPlatformWindow(QWindow* window) const override;
  QPlatformBackingStore* createPlatformBackingStore(
      QWindow* window) const override;
  QAbstractEventDispatcher* createEventDispatcher() const override;
  QPlatformNativeInterface* nativeInterface() const override;

  QList<MockScreen*> screens_;

  QScopedPointer<MockPlatformNativeInterface> native_interface_;

  ShimApi shim_;
};

#endif // _OXIDE_QT_TESTS_MOCK_QPA_INTEGRATION_H_
