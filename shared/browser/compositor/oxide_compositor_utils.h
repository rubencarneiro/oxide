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
class TaskRunner;
}

namespace cc {
class ContextProvider;
}

namespace gpu {
class Mailbox;
}

namespace oxide {

class GLFrameData;

class CompositorUtils {
 public:
  static CompositorUtils* GetInstance();

  virtual void Initialize() = 0;
  virtual void Shutdown() = 0;

  virtual scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner() = 0;

  typedef base::Callback<void(scoped_ptr<GLFrameData>)> CreateGLFrameHandleCallback;
  virtual void CreateGLFrameHandle(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::TaskRunner> task_runner) = 0;

  virtual gfx::GLSurfaceHandle GetSharedSurfaceHandle() = 0;

  virtual bool CanUseGpuCompositing() = 0;

 protected:
  virtual ~CompositorUtils();
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_UTILS_H_
