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

#ifndef _OXIDE_SHARED_BROWSER_GL_CONTEXT_DEPENDENT_H_
#define _OXIDE_SHARED_BROWSER_GL_CONTEXT_DEPENDENT_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "ui/gl/gl_context.h"

namespace gfx {
class GLShareGroup;
}

namespace oxide {

class GLContextDependent : public gfx::GLContext {
 public:
  GLContextDependent(void* handle,
                     bool was_allocated_using_robustness_extension);
  virtual ~GLContextDependent();

  static scoped_refptr<GLContextDependent> CloneFrom(
      GLContextDependent* other);

  // gfx::GLContext implementation
  bool Initialize(gfx::GLSurface* compatible_surface,
                  gfx::GpuPreference gpu_preference) override;
  void Destroy() override;
  bool MakeCurrent(gfx::GLSurface* surface) override;
  void ReleaseCurrent(gfx::GLSurface* surface) override;
  bool IsCurrent(gfx::GLSurface* surface) override;
  void* GetHandle() override;
  bool WasAllocatedUsingRobustnessExtension() override;

 private:
  void OnSetSwapInterval(int interval) override;

  GLContextDependent(GLContextDependent* other);

  void* handle_;
  bool was_allocated_using_robustness_extension_;

  DISALLOW_COPY_AND_ASSIGN(GLContextDependent);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GL_CONTEXT_ADOPTED_H_
