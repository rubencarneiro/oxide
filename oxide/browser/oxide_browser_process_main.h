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

#ifndef _OXIDE_BROWSER_BROWSER_PROCESS_MAIN_H_
#define _OXIDE_BROWSER_BROWSER_PROCESS_MAIN_H_

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

typedef ContentMainDelegate* (ContentMainDelegateFactory)();

class BrowserProcessMain FINAL : public base::RefCounted<BrowserProcessMain> {
 public:
  static IOThreadDelegate* io_thread_delegate();

  static bool Exists();

 private:
  friend class BrowserProcessHandleBase;
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
  static void InitContentMainDelegateFactory(
      ContentMainDelegateFactory* factory);

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

#endif // _OXIDE_BROWSER_BROWSER_PROCESS_H_
