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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
class MainFunctionParams;
}

namespace oxide {

class ContentMainDelegate;
class SharedGLContext;

// This class basically encapsulates the process-wide bits that would
// normally be kept alive for the life of the process on the stack in
// Chrome (which is not possible in a public API)
class BrowserProcessMain {
 public:
  virtual ~BrowserProcessMain();

  // Creates the BrowserProcessMain singleton and starts the
  // browser process components
  static void Start(
      scoped_refptr<oxide::SharedGLContext> shared_gl_context,
      scoped_ptr<ContentMainDelegate> delegate);

  // Quit the browser process components and delete the
  // BrowserProcessMain singleton
  static void Shutdown();

  // Returns true if the browser process components are running
  static bool IsRunning();

  // Return the BrowserProcessMain singleton
  static BrowserProcessMain* instance();

  virtual SharedGLContext* GetSharedGLContext() const = 0;

 protected:
  BrowserProcessMain();

 private:
  // For RunBrowserMain() / ShutdownBrowserMain()
  friend class oxide::ContentMainDelegate;

  virtual int RunBrowserMain(
      const content::MainFunctionParams& main_function_params) = 0;
  virtual void ShutdownBrowserMain() = 0;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessMain);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_H_
