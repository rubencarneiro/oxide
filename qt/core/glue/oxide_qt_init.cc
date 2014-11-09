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

#include "base/files/file_path.h"
#include "base/logging.h"

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/app/oxide_qt_platform_delegate.h"
#include "shared/base/oxide_enum_flags.h"
#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {
namespace qt {

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(oxide::SupportedGLImplFlags)

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

  scoped_ptr<PlatformDelegate> delegate(new PlatformDelegate());

  oxide::SupportedGLImplFlags supported_gl_impls =
      oxide::SUPPORTED_GL_IMPL_NONE;
  if (QGuiApplication::platformNativeInterface()) {
    QString platform = QGuiApplication::platformName();
    if (platform == QLatin1String("xcb")) {
      supported_gl_impls |= oxide::SUPPORTED_GL_IMPL_DESKTOP_GL;
      supported_gl_impls |= oxide::SUPPORTED_GL_IMPL_EGL_GLES2;
    } else if (platform.startsWith("ubuntu")) {
      supported_gl_impls |= oxide::SUPPORTED_GL_IMPL_EGL_GLES2;
    } else {
      LOG(WARNING) << "Unrecognized Qt platform: " << qPrintable(platform);
    }
  } else {
    LOG(WARNING)
        << "Unable to determine native display handle on Qt platform: "
        << qPrintable(QGuiApplication::platformName());
  }

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

  oxide::BrowserProcessMain::GetInstance()->Start(
      delegate.Pass(),
#if defined(USE_NSS)
      base::FilePath(nss_db_path.toStdString()),
#endif
      supported_gl_impls,
      static_cast<oxide::ProcessModel>(oxideGetProcessModel()));

  qAddPostRoutine(ShutdownChromium);
}

} // namespace qt
} // namespace oxide
