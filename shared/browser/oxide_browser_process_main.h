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

#include <memory>
#include <string>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gl/gl_implementation.h"

#include "shared/common/oxide_shared_export.h"

#if defined(OS_LINUX)
namespace base {
class FilePath;
}
#endif

namespace oxide {

class PlatformDelegate;

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
class OXIDE_SHARED_EXPORT BrowserProcessMain {
 public:

  struct StartParams {
    StartParams(std::unique_ptr<PlatformDelegate> delegate);
    ~StartParams();

    StartParams(StartParams&& other);

    std::unique_ptr<PlatformDelegate> delegate;
#if defined(OS_LINUX)
    base::FilePath nss_db_path;
#endif
    gl::GLImplementation gl_implementation;
    ProcessModel process_model;
    gfx::Size primary_screen_size;
    std::string argv0;
  };

  virtual ~BrowserProcessMain();

  static ProcessModel GetProcessModelOverrideFromEnv();

  // Return the BrowserProcessMain singleton
  static BrowserProcessMain* GetInstance();

  // Creates the BrowserProcessMain singleton and starts the
  // browser process components
  virtual void Start(StartParams params) = 0;

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
