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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_MAIN_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_MAIN_H_

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#if defined(USE_NSS)
namespace base {
class FilePath;
}
#endif

namespace oxide {

class PlatformDelegate;

enum SupportedGLImplFlags {
  SUPPORTED_GL_IMPL_NONE = 0,
  SUPPORTED_GL_IMPL_DESKTOP_GL = 1 << 0,
  SUPPORTED_GL_IMPL_EGL_GLES2 = 1 << 1
};

enum ProcessModel {
  PROCESS_MODEL_MULTI_PROCESS,
  PROCESS_MODEL_SINGLE_PROCESS,
  PROCESS_MODEL_PROCESS_PER_SITE_INSTANCE,
  PROCESS_MODEL_PROCESS_PER_VIEW,
  PROCESS_MODEL_PROCESS_PER_SITE,
  PROCESS_MODEL_SITE_PER_PROCESS,

  PROCESS_MODEL_UNDEFINED = 1000
};

// This class basically encapsulates the process-wide bits that would
// normally be kept alive for the life of the process on the stack in
// Chrome (which is not possible in a public API)
class BrowserProcessMain {
 public:
  virtual ~BrowserProcessMain();

  static ProcessModel GetProcessModelOverrideFromEnv();

  // Return the BrowserProcessMain singleton
  static BrowserProcessMain* GetInstance();

  // Creates the BrowserProcessMain singleton and starts the
  // browser process components
  virtual void Start(scoped_ptr<PlatformDelegate> delegate,
#if defined(USE_NSS)
                     const base::FilePath& nss_db_path,
#endif
                     SupportedGLImplFlags supported_gl_flags,
                     ProcessModel process_model) = 0;

  // Quit the browser process components and delete the
  // BrowserProcessMain singleton
  virtual void Shutdown() = 0;

  // Returns true if the browser process components are running
  virtual bool IsRunning() const = 0;

  virtual ProcessModel GetProcessModel() const = 0;

 protected:
  BrowserProcessMain();

 private:
  DISALLOW_COPY_AND_ASSIGN(BrowserProcessMain);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_H_
