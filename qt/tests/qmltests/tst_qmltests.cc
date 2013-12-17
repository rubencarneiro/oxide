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

#include <QGuiApplication>
#include <QtQuickTest/quicktest.h>
#include <QtQuickVersion>
#if QTQUICK_VERSION >= 0x050200
#include <QtQuick/private/qsgcontext_p.h>
#endif

int main(int argc, char** argv) {
  QGuiApplication app(argc, argv);

#if QTQUICK_VERSION >= 0x050200
  QOpenGLContext context;
  context.create();
  QSGContext::setSharedOpenGLContext(&context);
#endif

  return quick_test_main(argc, argv, "qmltests", QUICK_TEST_SOURCE_DIR);
}
