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

#ifndef _OXIDE_SHARED_BROWSER_GL_CONTEXT_DEPENDENT_H_
#define _OXIDE_SHARED_BROWSER_GL_CONTEXT_DEPENDENT_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "ui/gl/gl_context.h"

#include "shared/common/oxide_shared_export.h"

namespace gfx {
class GLShareGroup;
}

namespace oxide {

class OXIDE_SHARED_EXPORT GLContextDependent : public gl::GLContext {
 public:
  GLContextDependent(void* handle,
                     bool was_allocated_using_robustness_extension);
  virtual ~GLContextDependent();

  static scoped_refptr<GLContextDependent> CloneFrom(
      GLContextDependent* other);

  // gfx::GLContext implementation
  bool Initialize(gl::GLSurface* compatible_surface,
                  gl::GpuPreference gpu_preference) override;
  bool MakeCurrent(gl::GLSurface* surface) override;
  void ReleaseCurrent(gl::GLSurface* surface) override;
  bool IsCurrent(gl::GLSurface* surface) override;
  void* GetHandle() override;
  bool WasAllocatedUsingRobustnessExtension() override;
  scoped_refptr<gl::GPUTimingClient> CreateGPUTimingClient() override;

 private:
  void OnSetSwapInterval(int interval) override;

  GLContextDependent(GLContextDependent* other);

  void* handle_;
  bool was_allocated_using_robustness_extension_;

  DISALLOW_COPY_AND_ASSIGN(GLContextDependent);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GL_CONTEXT_ADOPTED_H_
