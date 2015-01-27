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

#include "oxide_qt_browser_startup.h"

#include <QCoreApplication>
#include <QGlobalStatic>
#include <QGuiApplication>
#include <QtDebug>
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
#include <QtGui/private/qopenglcontext_p.h>
#endif

#include "base/logging.h"

#include "qt/core/app/oxide_qt_platform_delegate.h"
#include "qt/core/gpu/oxide_qt_gl_context_adopted.h"

#include "oxide_qt_web_context.h"

namespace oxide {
namespace qt {

namespace {

Q_GLOBAL_STATIC(BrowserStartup, g_instance)

void ShutdownChromium() {
  WebContext::DestroyDefault();
  oxide::BrowserProcessMain::GetInstance()->Shutdown();
}

}

BrowserStartup::BrowserStartup()
    : process_model_is_from_env_(false),
      process_model_(oxide::PROCESS_MODEL_UNDEFINED) {}

// static
BrowserStartup* BrowserStartup::GetInstance() {
  return g_instance();
}

BrowserStartup::~BrowserStartup() {}

base::FilePath BrowserStartup::GetNSSDbPath() const {
#if defined(USE_NSS)
  return nss_db_path_;
#else
  return base::FilePath();
#endif
}

void BrowserStartup::SetNSSDbPath(const base::FilePath& path) {
#if defined(USE_NSS)
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
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
    } else {
      process_model_is_from_env_ = true;
    }
  }

  return process_model_;
}

void BrowserStartup::SetProcessModel(oxide::ProcessModel model) {
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    qWarning() << "Cannot set the process model once Oxide is running";
    return;
  }

  DCHECK_NE(model, oxide::PROCESS_MODEL_UNDEFINED);

  static bool did_warn = false;
  if (!did_warn) {
    did_warn = true;
    qWarning()
        << "Changing the Oxide process model is currently experimental "
        << "and untested, and might break your application in unexpected "
        << "ways. This interface is only exposed for development purposes "
        << "at the moment, although it will eventually be suitable for "
        << "public use";
  }

  process_model_is_from_env_ = false;
  process_model_ = model;
}

#if defined(ENABLE_COMPOSITING) && QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
void BrowserStartup::SetSharedGLContext(GLContextAdopted* context) {
  DCHECK(!oxide::BrowserProcessMain::GetInstance()->IsRunning());
  shared_gl_context_ = context;
}
#endif

bool BrowserStartup::DidSelectProcessModelFromEnv() const {
  return process_model_is_from_env_;
}

void BrowserStartup::EnsureChromiumStarted() {
  if (oxide::BrowserProcessMain::GetInstance()->IsRunning()) {
    return;
  }

  CHECK(qobject_cast<QGuiApplication *>(QCoreApplication::instance())) <<
      "Your application doesn't have a QGuiApplication. Oxide will not "
      "function without one";

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  shared_gl_context_ = GLContextAdopted::Create(qt_gl_global_share_context());
#elif QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  shared_gl_context_ = GLContextAdopted::Create(
      QOpenGLContextPrivate::globalShareContext());
#endif

  scoped_ptr<PlatformDelegate> delegate(
      new PlatformDelegate(shared_gl_context_.get()));

  gfx::GLImplementation gl_impl = gfx::kGLImplementationNone;

  if (shared_gl_context_) {
    gl_impl = shared_gl_context_->implementation();
  } else {
    QString platform = QGuiApplication::platformName();
    if (QGuiApplication::platformNativeInterface()) {
      if (platform == QLatin1String("xcb")) {
        gl_impl = gfx::kGLImplementationDesktopGL;
      } else if (platform.startsWith("ubuntu")) {
        gl_impl = gfx::kGLImplementationEGLGLES2;
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

  oxide::BrowserProcessMain::GetInstance()->Start(
      delegate.Pass(),
#if defined(USE_NSS)
      GetNSSDbPath(),
#endif
      gl_impl,
      GetProcessModel());

  qAddPostRoutine(ShutdownChromium);
}

} // namespace qt
} // namespace oxide
