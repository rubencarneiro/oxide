// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_STARTUP_H_
#define _OXIDE_QT_CORE_BROWSER_STARTUP_H_

#include <QtGlobal>

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"

#include "shared/browser/oxide_browser_process_main.h"

namespace oxide {
namespace qt {

class GLContextDependent;

class BrowserStartup final {
 public:
  static BrowserStartup* GetInstance();

  // These should be private, but Q_GLOBAL_STATIC doesn't allow this
  BrowserStartup();
  ~BrowserStartup();

  base::FilePath GetNSSDbPath() const;
  void SetNSSDbPath(const base::FilePath& path);

  oxide::ProcessModel GetProcessModel();
  void SetProcessModel(oxide::ProcessModel model);

  GLContextDependent* shared_gl_context() const {
    return shared_gl_context_.get();
  }
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
  void SetSharedGLContext(GLContextDependent* context);
#endif

  bool DidSelectProcessModelFromEnv() const;

  void EnsureChromiumStarted();

 private:
#if defined(USE_NSS_CERTS)
  base::FilePath nss_db_path_;
#endif

  bool process_model_is_from_env_;
  oxide::ProcessModel process_model_;

  scoped_refptr<GLContextDependent> shared_gl_context_;

  DISALLOW_COPY_AND_ASSIGN(BrowserStartup);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_STARTUP_H_
