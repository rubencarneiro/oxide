// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

// This file contains a set of shims that allows GPU-client code to access
// GPU-service functionality without having to pull in GPU-service headers,
// which may pull in gl_bindings.h which pulls in other headers that conflict
// with headers normally used by client code

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_GPU_SHIMS_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_GPU_SHIMS_H_

#include <stdint.h>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "gpu/command_buffer/common/command_buffer_id.h"

typedef unsigned int GLuint;

typedef unsigned int EGLBoolean;
typedef void* EGLDisplay;
typedef void* EGLImageKHR;

namespace base {
class SingleThreadTaskRunner;
}

namespace gpu {
class Mailbox;
class SyncToken;
}

namespace oxide {

class EGL {
 public:
  static EGLBoolean DestroyImageKHR(EGLDisplay dpy,
                                    EGLImageKHR image);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(EGL);
};

class GpuUtils {
 public:
  static EGLDisplay GetHardwareEGLDisplay();

  static scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner();

  static bool IsSyncPointRetired(const gpu::SyncToken& sync_token);
  static bool WaitForSyncPoint(const gpu::SyncToken& sync_token,
                               const base::Closure& callback);

  static bool CanUseEGLImage();

  static GLuint GetTextureFromMailbox(gpu::CommandBufferId command_buffer,
                                      const gpu::Mailbox& mailbox);

  static EGLImageKHR CreateEGLImageFromMailbox(
      gpu::CommandBufferId command_buffer,
      const gpu::Mailbox& mailbox);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(GpuUtils);
};


} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_GPU_SHIMS_H_
