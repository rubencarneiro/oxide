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

#include <QGuiApplication>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "content/public/test/unittest_test_suite.h"
#include "mojo/edk/embedder/embedder.h"

#include "shared/test/oxide_test_suite.h"

namespace {
const char kQtPluginPath[] = "qt-plugin-path";
}

// QCoreApplication adds the application directory to
// QCoreApplication::libraryPaths(), but in an unstripped static build we get
// crashes whenever a QFactoryLoader instance enumerates it (it has problems
// with libOxideQtCore.so.0). This just works around that by removing it from
// the directory list after QCoreApplication is constructed.
//
// Note, this prevents us from using -platformpluginpath, as the platform
// plugin is instantiated before we have a chance to remove the application
// directory from the directory list, and this flag causes Qt to look in
// <appdir> for platform plugins (rather than <appdir>/platforms)

void RemoveApplicationDirFromLibraryPaths() {
  QCoreApplication::removeLibraryPath(QCoreApplication::applicationDirPath());
}

Q_COREAPP_STARTUP_FUNCTION(RemoveApplicationDirFromLibraryPaths)

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);
  {
    const base::CommandLine& command_line =
        *base::CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(kQtPluginPath)) {
      std::string path = command_line.GetSwitchValueASCII(kQtPluginPath);
      LOG(INFO) << "Qt plugin path: " << path;
      QCoreApplication::addLibraryPath(QString::fromStdString(path));
    }
  }
  base::CommandLine::Reset();

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QGuiApplication app(argc, argv);
  content::UnitTestTestSuite test_suite(new oxide::TestSuite(argc, argv));
  mojo::edk::Init();
  return base::LaunchUnitTests(
      argc, argv, base::Bind(&content::UnitTestTestSuite::Run,
                             base::Unretained(&test_suite)));
}
