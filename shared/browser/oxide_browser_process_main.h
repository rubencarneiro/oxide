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
#include "base/memory/scoped_ptr.h"

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
class MainFunctionParams;
}

namespace oxide {

class ContentMainDelegate;
class IOThreadDelegate;

// This class basically encapsulates the process-wide bits that would
// normally be kept alive for the life of the process on the stack in
// Chrome (which is not possible in a public API)
class BrowserProcessMain FINAL {
 public:
  enum Flags {
    ENABLE_VIEWPORT = 1 << 0,
    ENABLE_OVERLAY_SCROLLBARS = 1 << 1,
    ENABLE_PINCH = 1 << 2
  };

  ~BrowserProcessMain();

  // Start the browser process components if they haven't already
  // been started
  static bool Run(int flags);
  static void Quit();

  static int GetFlags();

  // Returns true of the browser process components have been started
  static bool IsRunning();

  // Return the IO thread delegate, which is a container of objects
  // whose lifetime is tied to the IO thread
  static IOThreadDelegate* io_thread_delegate();

  // Ensure that the IO thread delegate is created
  static void CreateIOThreadDelegate();

 private:
  // For RunBrowserMain() / ShutdownBrowserMain()
  friend class oxide::ContentMainDelegate;

  BrowserProcessMain(int flags);

  static int RunBrowserMain(
      const content::MainFunctionParams& main_function_params);
  static void ShutdownBrowserMain();

  bool Init();
  void Shutdown();

  bool did_shutdown_;

  int flags_;

  // XXX: Don't change the order of these unless you know what you are
  //      doing. It's important that ContentMainDelegate outlives
  //      ContentMainRunner
  scoped_ptr<ContentMainDelegate> main_delegate_;
  scoped_ptr<content::ContentMainRunner> main_runner_;
  scoped_ptr<content::BrowserMainRunner> browser_main_runner_;

  scoped_ptr<IOThreadDelegate> io_thread_delegate_;

  DISALLOW_COPY_AND_ASSIGN(BrowserProcessMain);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_PROCESS_H_
