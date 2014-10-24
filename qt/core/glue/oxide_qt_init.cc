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

#include "oxide_qt_init.h"

#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>

#include "base/logging.h"

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/app/oxide_qt_content_main_delegate.h"
#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {
namespace qt {

namespace {

QOpenGLContext* g_shared_gl_context;

void ShutdownChromium() {
  oxide::BrowserProcessMain::GetInstance()->Shutdown();
}

}

QOpenGLContext* GetSharedGLContext() {
  return g_shared_gl_context;
}

void SetSharedGLContext(QOpenGLContext* context) {
  CHECK(!oxide::BrowserProcessMain::GetInstance()->IsRunning()) <<
      "SetSharedGLContext must be called before the browser components are "
      "started!";

  g_shared_gl_context = context;
}

void EnsureChromiumStarted() {
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    return;
  }

  CHECK(qobject_cast<QGuiApplication *>(QCoreApplication::instance())) <<
      "Your application doesn't have a QGuiApplication. Oxide will not "
      "function without one";

  QString nss_db_path(oxideGetNSSDbPath());
  if (!nss_db_path.isEmpty()) {
    nss_db_path = QDir(nss_db_path).absolutePath();
  }

  scoped_ptr<ContentMainDelegate> delegate(
      ContentMainDelegate::CreateForBrowser(
        base::FilePath(nss_db_path.toStdString())));
  oxide::BrowserProcessMain::GetInstance()->Start(delegate.Pass());

  qAddPostRoutine(ShutdownChromium);
}

} // namespace qt
} // namespace oxide
