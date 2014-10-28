// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_

#include <queue>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "ui/gfx/native_widget_types.h"

namespace base {
class SingleThreadTaskRunner;
class TaskRunner;
class Thread;
}

namespace cc {
class OutputSurface;
}

namespace gfx {
class Size;
}

namespace gpu {
class Mailbox;
}

template <typename T> struct DefaultSingletonTraits;

namespace oxide {

class GLFrameData;

class CompositorUtils final : public base::MessageLoop::TaskObserver {
 public:
  static CompositorUtils* GetInstance();

  void Initialize();
  void Destroy();

  scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner();

  typedef base::Callback<void(scoped_ptr<GLFrameData>)> CreateGLFrameHandleCallback;
  void CreateGLFrameHandle(cc::OutputSurface* output_surface,
                           const gpu::Mailbox& mailbox,
                           uint32 sync_point,
                           const CreateGLFrameHandleCallback& callback,
                           scoped_refptr<base::TaskRunner> task_runner);

  gfx::GLSurfaceHandle GetSharedSurfaceHandle();

 private:
  friend struct DefaultSingletonTraits<CompositorUtils>;

  CompositorUtils();
  ~CompositorUtils();

  void InitializeOnGpuThread();

  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) final;
  void DidProcessTask(const base::PendingTask& pending_task) final;

  int32 client_id_;

  scoped_ptr<base::Thread> compositor_thread_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::Lock fetch_texture_resources_lock_;
  bool fetch_texture_resources_pending_;
  bool gpu_thread_is_processing_task_;

  class FetchTextureResourcesTask;
  std::queue<scoped_refptr<FetchTextureResourcesTask> > fetch_texture_resources_queue_;

  bool can_use_gpu_;

  DISALLOW_COPY_AND_ASSIGN(CompositorUtils);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_
