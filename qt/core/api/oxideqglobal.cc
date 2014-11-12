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

#include "base/files/file_path.h"
#include "content/public/browser/render_process_host.h"

#include "qt/core/browser/oxide_qt_browser_platform_integration.h"
#include "qt/core/browser/oxide_qt_browser_startup.h"
#include "shared/browser/oxide_browser_process_main.h"

using namespace oxide::qt;

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
  COMPILE_ASSERT(
      OxideProcessModelMultiProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_MULTI_PROCESS),
      process_model_enums_multi_process_doesnt_match);
  COMPILE_ASSERT(
      OxideProcessModelSingleProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_SINGLE_PROCESS),
      process_model_enums_single_process_doesnt_match);
  COMPILE_ASSERT(
      OxideProcessModelProcessPerSiteInstance ==
        static_cast<OxideProcessModel>(
          oxide::PROCESS_MODEL_PROCESS_PER_SITE_INSTANCE),
      process_model_enums_process_per_site_instance_doesnt_match);
  COMPILE_ASSERT(
      OxideProcessModelProcessPerView ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_PROCESS_PER_VIEW),
      process_model_enums_process_per_view_doesnt_match);
  COMPILE_ASSERT(
      OxideProcessModelProcessPerSite ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_PROCESS_PER_SITE),
      process_model_enums_process_per_site_doesnt_match);
  COMPILE_ASSERT(
      OxideProcessModelSitePerProcess ==
        static_cast<OxideProcessModel>(oxide::PROCESS_MODEL_SITE_PER_PROCESS),
      process_model_enums_site_per_process_doesnt_match);

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
