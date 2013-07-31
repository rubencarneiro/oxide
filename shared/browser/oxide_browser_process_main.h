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
class IOThreadDelegate;

// This class basically encapsulates the process-wide bits that would
// normally be kept alive for the life of the process on the stack in
// Chrome (which is not possible in a public API). Use BrowserProcessHandle
// to hold a reference to this, which also handles setting the
// ContentMainDelegate to that specified by the implementation
class BrowserProcessMain FINAL : public base::RefCounted<BrowserProcessMain> {
 public:

  // Return the IO thread delegate, which is a container of objects
  // whose lifetime is tied to the IO thread
  static IOThreadDelegate* io_thread_delegate();

  // Returns true of the browser process components have been started
  static bool Exists();

 private:
  friend class BrowserProcessHandle;
  friend class base::RefCounted<BrowserProcessMain>;
  friend class BrowserMainParts;
  friend class ContentMainDelegate;

  BrowserProcessMain();
  ~BrowserProcessMain();

  static BrowserProcessMain* GetInstance();

  static void Create();
  bool Init();

  static int RunBrowserProcess(
      const content::MainFunctionParams& main_function_params);
  static void ShutdownBrowserProcess();

  static void PreCreateThreads();

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
