// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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
#include "base/memory/ref_counted.h"
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

class GLContextDependent;
class IOThread;
class LifecycleObserver;
class RenderProcessInitializer;

class BrowserMainParts : public content::BrowserMainParts {
 public:
  BrowserMainParts();
  ~BrowserMainParts();

 private:
  // content::BrowserMainParts implementation
  void PreEarlyInitialization() override;
  int PreCreateThreads() override;
  void PreMainMessageLoopRun() override;
  bool MainMessageLoopRun(int* result_code) override;
  void PostMainMessageLoopRun() override;
  void PostDestroyThreads() override;

  // This is a bit odd - gl_share_group_ is consumed by the GPU thread,
  // but neither gfx::GLContext nor gfx::GLShareGroup are thread-safe.
  // Therefore, it's only safe to drop this reference once the GPU thread
  // has been shut-down
  scoped_refptr<GLContextDependent> gl_share_context_;

  scoped_ptr<gpu::GpuInfoCollectorOxideLinux> gpu_info_collector_;
  scoped_ptr<base::MessageLoop> main_message_loop_;
  scoped_ptr<IOThread> io_thread_;
  scoped_ptr<gfx::Screen> primary_screen_;

  scoped_ptr<LifecycleObserver> lifecycle_observer_;

  scoped_ptr<RenderProcessInitializer> render_process_initializer_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMainParts);
};

} // namespace oxide

#endif //_OXIDE_SHARED_BROWSER_BROWSER_MAIN_PARTS_H_
