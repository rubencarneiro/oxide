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

#include <QFileInfo>
#include <QGuiApplication>
#include <QList>

#include "base/bind.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "content/public/test/unittest_test_suite.h"
#include "mojo/edk/embedder/embedder.h"

#include "shared/test/oxide_test_suite.h"

// QCoreApplication adds the application directory to
// QCoreApplication::libraryPaths(), but in an unstripped static build we get
// crashes whenever a QFactoryLoader instance enumerates it (it has problems
// with libOxideQtCore.so.0).
// QCoreApplication adds the application directory whenever
// QCoreApplication::libraryPaths() is accessed if the instance() exists. It
// also adds it at the end of the constructor (after QGuiApplication has loaded
// the platform plugin).
//
// We need the application directory to not exist when the platform plugin is
// loaded. So, what we do is:
// 1) Access QCoreApplication::libraryPaths() before QCoreApplication is
//    instantiated. This ensures we don't look in the application directory
//    when the platform plugin is created
// 2) Register a callback that runs at the end of QCoreApplication
//    initialization to remove the application directory

void RemoveApplicationDirFromLibraryPaths() {
  QStringList paths = QCoreApplication::libraryPaths();
  QFileInfo appdir = QCoreApplication::applicationDirPath();

  auto it = std::find_if(paths.begin(), paths.end(),
                         [&appdir] (const QString& path) {
    return QFileInfo(path) == appdir;
  });

  if (it == paths.end()) {
    return;
  }

  paths.erase(it);

  QCoreApplication::setLibraryPaths(paths);
}

Q_COREAPP_STARTUP_FUNCTION(RemoveApplicationDirFromLibraryPaths)

int main(int argc, char** argv) {
  QCoreApplication::libraryPaths();

  QGuiApplication app(argc, argv);
  content::UnitTestTestSuite test_suite(new oxide::TestSuite(argc, argv));
  mojo::edk::Init();
  return base::LaunchUnitTests(
      argc, argv, base::Bind(&content::UnitTestTestSuite::Run,
                             base::Unretained(&test_suite)));
}
