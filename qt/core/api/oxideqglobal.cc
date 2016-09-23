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
#include "qt/core/common/oxide_version.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/common/chrome_version.h"

using namespace oxide::qt;

/*!
\headerfile <oxideqglobal.h>
\inmodule OxideQtCore
\ingroup funclists

\title Global functions
*/

namespace {

void RunShutdownCallback(OxideShutdownCallback callback) {
  callback();
}

}

void oxideAddShutdownCallback(OxideShutdownCallback callback) {
  BrowserStartup::AddShutdownCallback(
      base::Bind(&RunShutdownCallback, callback));
}

/*!
\relates <oxideqglobal.h>

Returns the custom directory that Oxide will look in for the NSS database. This
will return an empty string if the application hasn't called oxideSetNSSDbPath,
or this is a build of Oxide that doesn't use NSS.

\sa oxideSetNSSDbPath
*/

QString oxideGetNSSDbPath() {
  base::FilePath path = BrowserStartup::GetInstance()->GetNSSDbPath();
  return QString::fromStdString(path.value());
}

/*!
\relates <oxideqglobal.h>

Call this to specify a custom directory to look in for the NSS database. If this
isn't called, then Oxide will use the NSS shared database.

This must be called before Oxide is started up (in your applications main(),
before using any other Oxide APIs).

This has no effect if the build of Oxide does not use NSS.

This function is mostly only useful for testing purposes.

\sa oxideGetNSSDbPath
*/

void oxideSetNSSDbPath(const QString& path) {
  base::FilePath p(path.toStdString());
  BrowserStartup::GetInstance()->SetNSSDbPath(p);
}

/*!
\relates <oxideqglobal.h>
\deprecated

Return the QThread instance for the internal Chromium IO thread.
*/

QThread* oxideGetIOThread() {
  return GetIOQThread();
}

/*!
\enum OxideProcessModel
\relates <oxideqglobal.h>

\value OxideProcessModelMultiProcess
Multi-process mode. In this mode, web content runs in sandboxed sub-processes
rather than the application process. This mode provides the best level of
security and fault tolerance.

\value OxideProcessModelSingleProcess
Single process mode. In this mode, web content runs in the application process.
Web content is not sandboxed, and crashes that would normally only affect a web
content process in multi-process mode will result in an application crash in
this mode.

\omitvalue OxideProcessModelProcessPerSiteInstance
\omitvalue OxideProcessModelProcessPerView
\omitvalue OxideProcessModelProcessPerSite
\omitvalue OxideProcessModelSitePerProcess
*/

/*!
\relates <oxideqglobal.h>

Returns the current process model. The default (if the application hasn't called
oxideSetProcessModel) will be OxideProcessModelMultiProcess.

\sa oxideSetProcessModel
*/

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

/*!
\relates <oxideqglobal.h>

Set the desired process model. This determines whether Oxide will run web
content in sandboxed sub-processes or the application process.

This must be called before Oxide is started up (in your applications main(),
before using any other Oxide APIs). Calling it afterwards will have no effect.

\sa oxideGetProcessModel
*/

void oxideSetProcessModel(OxideProcessModel model) {
  oxide::ProcessModel m = static_cast<oxide::ProcessModel>(model);
  BrowserStartup::GetInstance()->SetProcessModel(m);
}

/*!
\relates <oxideqglobal.h>
Return the maximum number of web content processes that Oxide will run. If
oxideSetMaxRendererProcessCount has not been called, this will return a
system-dependent default.

\sa oxideSetMaxRendererProcessCount
*/

size_t oxideGetMaxRendererProcessCount() {
  return content::RenderProcessHost::GetMaxRendererProcessCount();
}

/*!
\relates <oxideqglobal.h>

Set the maximum number of web content processes to run. Setting this to 0 will
reset it to the default, which is system dependent.

This is not a hard limit, as there are cases where web content processes will
not be shared (eg, web views in different web contexts, or incognito /
non-incognito web views).

This must be called before Oxide is started up (in your applications main(),
before using any other Oxide APIs). Calling it afterwards will have no effect.

\sa oxideGetMaxRendererProcessCount
*/

void oxideSetMaxRendererProcessCount(size_t count) {
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the renderer process count once Oxide is running";
    return;
  }

  content::RenderProcessHost::SetMaxRendererProcessCount(count);
}

/*!
\relates <oxideqglobal.h>
\since OxideQt 1.15

Return the Chromium version that this Oxide build is based on, in the form
\e{x.x.x.x}.
*/

QString oxideGetChromeVersion() {
  static QString kChromeVersion = QStringLiteral(CHROME_VERSION_STRING);
  return kChromeVersion;
}

/*!
\relates <oxideqglobal.h>
\since OxideQt 1.15

Return the current Oxide version, in the form \e{1.x.x}.
*/

QString oxideGetVersion() {
  static QString kOxideVersion = QStringLiteral(OXIDE_VERSION_STRING);
  return kOxideVersion;
}
