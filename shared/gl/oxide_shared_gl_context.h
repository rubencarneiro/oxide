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

#ifndef _OXIDE_SHARED_SHARED_GL_CONTEXT_H_
#define _OXIDE_SHARED_SHARED_GL_CONTEXT_H_

#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"

namespace oxide {

class GLShareGroup FINAL : public gfx::GLShareGroup {
 public:
  GLShareGroup();

  gfx::GLContext* GetContext() FINAL { return context_; }
  void SetContext(gfx::GLContext* context) {
    context_ = context;
  }

 private:
  ~GLShareGroup();

  gfx::GLContext* context_;
};

class SharedGLContext : public gfx::GLContext {
 public:
  SharedGLContext(oxide::GLShareGroup* share_group);
  virtual ~SharedGLContext();

  static SharedGLContext* FromGfx(gfx::GLContext* context);

  void* GetHandle() = 0;
  virtual gfx::GLImplementation GetImplementation() = 0;

  bool Initialize(gfx::GLSurface* compatible_surface,
                  gfx::GpuPreference gpu_preference) FINAL;
  void Destroy() FINAL;
  bool MakeCurrent(gfx::GLSurface* surface) FINAL;
  void ReleaseCurrent(gfx::GLSurface* surface) FINAL;
  bool IsCurrent(gfx::GLSurface* surface) FINAL;
  void SetSwapInterval(int interval) FINAL;
};

} // namespace oxide

#endif // _OXIDE_SHARED_SHARED_GL_CONTEXT_H_
