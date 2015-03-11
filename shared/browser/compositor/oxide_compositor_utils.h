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

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "ui/gfx/native_widget_types.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace cc {
class ContextProvider;
}

namespace gpu {
class Mailbox;
}

namespace oxide {

class GLFrameData;

// Utilities for the compositor code
class CompositorUtils {
 public:

  // Return the CompositorUtils singleton
  static CompositorUtils* GetInstance();

  // Initialize the CompositorUtils instance by starting up a compositor thread
  // and starting the GPU service thread if it's not already running.
  // Initialization is not thread-safe - this call should complete before any
  // other threads use this class
  virtual void Initialize() = 0;

  // Shutdown the CompositorUtils instance. This must be called before the GPU
  // service thread is shut down and must be called on the same thread that
  // called Initialize()
  virtual void Shutdown() = 0;

  // Return the task runner for the compositor thread. Can be called on any
  // thread
  virtual scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner() = 0;

  typedef base::Callback<void(scoped_ptr<GLFrameData>)> CreateGLFrameHandleCallback;

  // Create a GPU service-side handle for the underlying texture represented by
  // the provided client-side paramters (|context_provider| and |mailbox|). The
  // result will be returned asynchronously using |callback|, only after the
  // specified |sync_point| has expired. The callback will be called on the
  // |task_runner| provided.
  // This must be called on the same thread that called Initialize() ot the
  // compositor thread. |task_runner| must be for either of these threads as
  // well
  virtual void CreateGLFrameHandle(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) = 0;

  virtual gfx::GLSurfaceHandle GetSharedSurfaceHandle() = 0;

  virtual bool CanUseGpuCompositing() = 0;

 protected:
  virtual ~CompositorUtils();
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_
