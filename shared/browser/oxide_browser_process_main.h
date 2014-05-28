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
class BrowserProcessMain FINAL {
 public:
  ~BrowserProcessMain();

  // Start the browser process components if they haven't already
  // been started. Cannot be called after Quit()
  static bool StartIfNotRunning(
      scoped_refptr<oxide::SharedGLContext> shared_gl_context);

  // Quit the browser process components if they are running
  static void ShutdownIfRunning();

  // Returns true if BrowserProcessMain exists
  static bool Exists();

  // Return the BrowserProcessMain singleton
  static BrowserProcessMain* instance();

  oxide::SharedGLContext* shared_gl_context() const {
    return shared_gl_context_.get();
  }

 private:
  // For RunBrowserMain() / ShutdownBrowserMain()
  friend class oxide::ContentMainDelegate;

  BrowserProcessMain(
      scoped_refptr<oxide::SharedGLContext> shared_gl_context);

  int RunBrowserMain(
      const content::MainFunctionParams& main_function_params);
  void ShutdownBrowserMain();

  bool Init();
  void Shutdown();

  bool did_shutdown_;

  scoped_refptr<oxide::SharedGLContext> shared_gl_context_;

  // XXX: Don't change the order of these unless you know what you are
  //      doing. It's important that ContentMainDelegate outlives
  //      ContentMainRunner
  scoped_ptr<ContentMainDelegate> main_delegate_;
  scoped_ptr<content::ContentMainRunner> main_runner_;
  scoped_ptr<content::BrowserMainRunner> browser_main_runner_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessMain);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_H_
