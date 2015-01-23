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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace base {
class MessageLoop;
}

namespace gfx {
class Screen;
}

namespace gpu {
class GpuInfoCollectorOxideLinux;
}

namespace oxide {

class IOThread;

class BrowserMainParts final : public content::BrowserMainParts {
 public:
  BrowserMainParts();
  ~BrowserMainParts();

 private:
  // content::BrowserMainParts implementation
  virtual void PreEarlyInitialization() final;
  virtual int PreCreateThreads() final;
  virtual void PreMainMessageLoopRun() final;
  virtual bool MainMessageLoopRun(int* result_code) final;
  virtual void PostMainMessageLoopRun() final;
  virtual void PostDestroyThreads() final;

  scoped_ptr<gpu::GpuInfoCollectorOxideLinux> gpu_info_collector_;
  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_ptr<IOThread> io_thread_;
  scoped_ptr<gfx::Screen> primary_screen_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMainParts);
};

} // namespace oxide

#endif //_OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
