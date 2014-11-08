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

#include "oxideqglobal.h"

#include <QGlobalStatic>
#include <QtDebug>

#include "qt/core/browser/oxide_qt_browser_platform_integration.h"
#include "shared/browser/oxide_browser_process_main.h"

Q_GLOBAL_STATIC(QString, g_nss_db_path)

QString oxideGetNSSDbPath() {
  return *g_nss_db_path;
}

bool oxideSetNSSDbPath(const QString& path) {
#if defined(USE_NSS)
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the NSS DB directory once Oxide is running";
    return false;
  }

  *g_nss_db_path = path;
  return true;
#else
  qWarning() << "NSS not supported on this build";
  return false;
#endif
}

QThread* oxideGetIOThread() {
  return oxide::qt::GetIOQThread();
}
