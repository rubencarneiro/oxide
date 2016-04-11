// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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
#include "oxideqglobal_p.h"

#include <QGlobalStatic>
#include <QtDebug>

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "content/public/browser/render_process_host.h"

#include "qt/core/browser/oxide_qt_browser_platform_integration.h"
#include "qt/core/browser/oxide_qt_browser_startup.h"
#include "qt/oxide_version.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/common/chrome_version.h"

using namespace oxide::qt;

namespace {

void RunShutdownCallback(OxideShutdownCallback callback) {
  callback();
}

const QString kChromeVersion = QStringLiteral(CHROME_VERSION_STRING);
const QString kOxideVersion = QStringLiteral(OXIDE_VERSION_STRING);

}

void oxideAddShutdownCallback(OxideShutdownCallback callback) {
  BrowserStartup::AddShutdownCallback(
      base::Bind(&RunShutdownCallback, callback));
}

QString oxideGetNSSDbPath() {
  base::FilePath path = BrowserStartup::GetInstance()->GetNSSDbPath();
  return QString::fromStdString(path.value());
}

void oxideSetNSSDbPath(const QString& path) {
  base::FilePath p(path.toStdString());
  BrowserStartup::GetInstance()->SetNSSDbPath(p);
}

QThread* oxideGetIOThread() {
  return GetIOQThread();
}

OxideProcessModel oxideGetProcessModel() {
  static_assert(
      OxideProcessModelMultiProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_MULTI_PROCESS),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelMultiProcess");
  static_assert(
      OxideProcessModelSingleProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_SINGLE_PROCESS),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelSingleProcess");
  static_assert(
      OxideProcessModelProcessPerSiteInstance ==
        static_cast<OxideProcessModel>(
          oxide::PROCESS_MODEL_PROCESS_PER_SITE_INSTANCE),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelProcessPerSiteInstance");
  static_assert(
      OxideProcessModelProcessPerView ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_PROCESS_PER_VIEW),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelProcessPerView");
  static_assert(
      OxideProcessModelProcessPerSite ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_PROCESS_PER_SITE),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelProcessPerSite");
  static_assert(
      OxideProcessModelSitePerProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_SITE_PER_PROCESS),
      "OxideProcessModel and oxide::ProcessModel enums don't match: "
      "OxideProcessModelSitePerProcess");

  oxide::ProcessModel model = BrowserStartup::GetInstance()->GetProcessModel();
  return static_cast<OxideProcessModel>(model);
}

void oxideSetProcessModel(OxideProcessModel model) {
  oxide::ProcessModel m = static_cast<oxide::ProcessModel>(model);
  BrowserStartup::GetInstance()->SetProcessModel(m);
}

size_t oxideGetMaxRendererProcessCount() {
  return content::RenderProcessHost::GetMaxRendererProcessCount();
}

void oxideSetMaxRendererProcessCount(size_t count) {
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the renderer process count once Oxide is running";
    return;
  }

  content::RenderProcessHost::SetMaxRendererProcessCount(count);
}

QString oxideGetChromeVersion() {
  return kChromeVersion;
}

QString oxideGetOxideVersion() {
  return kOxideVersion;
}
