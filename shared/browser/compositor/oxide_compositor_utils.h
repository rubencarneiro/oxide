// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace cc {
class ContextProvider;
class TaskGraphRunner;
}

namespace gpu {
class Mailbox;
}

namespace oxide {

// Utilities for the compositor code
class CompositorUtils {
 public:

  // Return the CompositorUtils singleton
  static CompositorUtils* GetInstance();

  // Initialize the CompositorUtils instance by starting up a compositor thread
  // and starting the GPU service thread if it's not already running.
  // Initialization is not thread-safe - this call should complete before any
  // other threads use this class
  virtual void Initialize(bool has_share_context) = 0;

  // Shutdown the CompositorUtils instance. This must be called before the GPU
  // service thread is shut down and must be called on the same thread that
  // called Initialize()
  virtual void Shutdown() = 0;

  // Return the task runner for the compositor thread. Can be called on any
  // thread
  virtual scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner() = 0;

  typedef base::Callback<void(GLuint)> GetTextureFromMailboxCallback;

  // Asynchronously get the real texture from the GPU service thread, using
  // the specified |context_provider| and |mailbox| after |sync_point| has
  // expired.
  // This must be called on the same thread that called Initialize() ot the
  // compositor thread. |task_runner| must be for either of these threads as
  // well
  virtual void GetTextureFromMailbox(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const GetTextureFromMailboxCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) = 0;

  typedef base::Callback<void(EGLImageKHR)> CreateEGLImageFromMailboxCallback;

  // Asynchronously create an EGLImage backed by the texture represented by
  // the specified |mailbox| on the GPU service thread after |sync_point|
  // has expired.
  // This must be called on the same thread that called Initialize() ot the
  // compositor thread. |task_runner| must be for either of these threads as
  // well
  virtual void CreateEGLImageFromMailbox(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateEGLImageFromMailboxCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) = 0;

  virtual bool CanUseGpuCompositing() const = 0;

  virtual CompositingMode GetCompositingMode() const = 0;

  virtual cc::TaskGraphRunner* GetTaskGraphRunner() const = 0;

 protected:
  virtual ~CompositorUtils();
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_
