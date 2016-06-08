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

#include "oxide_qt_browser_startup.h"

#include <queue>

#include <QCoreApplication>
#include <QGlobalStatic>
#include <QGuiApplication>
#include <QScreen>
#include <QtDebug>
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
#include <QtGui/private/qopenglcontext_p.h>
#endif

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "ui/gfx/geometry/size.h"

#include "qt/core/app/oxide_qt_platform_delegate.h"
#include "qt/core/gpu/oxide_qt_gl_context_dependent.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_type_conversions.h"
#include "oxide_qt_web_context.h"

using oxide::BrowserProcessMain;

namespace oxide {
namespace qt {

namespace {

Q_GLOBAL_STATIC(BrowserStartup, g_instance)

base::LazyInstance<std::queue<base::Closure>> g_shutdown_callbacks =
    LAZY_INSTANCE_INITIALIZER;

void ShutdownChromium() {
  std::queue<base::Closure>& callbacks = g_shutdown_callbacks.Get();
  while (!callbacks.empty()) {
    base::Closure callback = callbacks.front();
    callbacks.pop();
    callback.Run();
  }

  BrowserProcessMain::GetInstance()->Shutdown();
}

}

BrowserStartup::BrowserStartup()
    : process_model_(oxide::PROCESS_MODEL_UNDEFINED) {}

// static
BrowserStartup* BrowserStartup::GetInstance() {
  return g_instance();
}

BrowserStartup::~BrowserStartup() {}

base::FilePath BrowserStartup::GetNSSDbPath() const {
#if defined(OS_LINUX)
  return nss_db_path_;
#else
  return base::FilePath();
#endif
}

void BrowserStartup::SetNSSDbPath(const base::FilePath& path) {
#if defined(OS_LINUX)
  if (BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the NSS DB directory once Oxide is running";
    return;
  }

  nss_db_path_ = path;
#else
  qWarning() << "NSS not supported on this build";
#endif
}

oxide::ProcessModel BrowserStartup::GetProcessModel() {
  if (process_model_ == oxide::PROCESS_MODEL_UNDEFINED) {
    process_model_ =
        oxide::BrowserProcessMain::GetProcessModelOverrideFromEnv();
    if (process_model_ == oxide::PROCESS_MODEL_UNDEFINED) {
      process_model_ = oxide::PROCESS_MODEL_MULTI_PROCESS;
    }
  }

  return process_model_;
}

void BrowserStartup::SetProcessModel(oxide::ProcessModel model) {
  if (BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the process model once Oxide is running";
    return;
  }

  DCHECK_NE(model, oxide::PROCESS_MODEL_UNDEFINED);

  process_model_ = model;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
void BrowserStartup::SetSharedGLContext(GLContextDependent* context) {
  DCHECK(!BrowserProcessMain::GetInstance()->IsRunning());
  shared_gl_context_ = context;
}
#endif

void BrowserStartup::EnsureChromiumStarted() {
  if (BrowserProcessMain::GetInstance()->IsRunning()) {
    return;
  }

  CHECK(qobject_cast<QGuiApplication *>(QCoreApplication::instance())) <<
      "Your application doesn't have a QGuiApplication. Oxide will not "
      "function without one";

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  shared_gl_context_ = GLContextDependent::Create(qt_gl_global_share_context());
#elif QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  shared_gl_context_ = GLContextDependent::Create(
      QOpenGLContextPrivate::globalShareContext());
#endif

  BrowserProcessMain::StartParams params(
      base::WrapUnique(new PlatformDelegate()));
#if defined(OS_LINUX)
  params.nss_db_path = GetNSSDbPath();
#endif
  params.process_model = GetProcessModel();

  params.gl_implementation = gl::kGLImplementationNone;

  if (shared_gl_context_) {
    params.gl_implementation = shared_gl_context_->implementation();
  } else {
    QString platform = QGuiApplication::platformName();
    if (QGuiApplication::platformNativeInterface()) {
      if (platform == QLatin1String("xcb")) {
        params.gl_implementation = gl::kGLImplementationDesktopGL;
      } else if (platform.startsWith("ubuntu") ||
                 platform == QLatin1String("mirserver") ||
                 platform == QLatin1String("egl")) {
        params.gl_implementation = gl::kGLImplementationEGLGLES2;
      } else {
        LOG(WARNING)
            << "Cannot determine GL implementation to use - "
            << "unrecognized Qt platform: " << qPrintable(platform);
      }
    } else {
      LOG(WARNING)
          << "Unable to use GL - No QPlatformNativeInterface for "
          << "platform: " << qPrintable(platform);
    }
  }

  QScreen* primary_screen = QGuiApplication::primaryScreen();
  params.primary_screen_size =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(primary_screen->size()),
                                          primary_screen);

  oxide::BrowserProcessMain::GetInstance()->Start(std::move(params));

  qAddPostRoutine(ShutdownChromium);
}

// static
void BrowserStartup::AddShutdownCallback(const base::Closure& callback) {
  g_shutdown_callbacks.Get().push(callback);
}

} // namespace qt
} // namespace oxide
