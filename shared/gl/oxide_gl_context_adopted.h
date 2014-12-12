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

#ifndef _OXIDE_SHARED_BROWSER_GL_CONTEXT_ADOPTED_H_
#define _OXIDE_SHARED_BROWSER_GL_CONTEXT_ADOPTED_H_

#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"

namespace gfx {
class GLShareGroup;
}

namespace oxide {

class GLContextAdopted : public gfx::GLContext {
 public:
  virtual ~GLContextAdopted();

  virtual gfx::GLImplementation GetImplementation() const = 0;

  bool Initialize(gfx::GLSurface* compatible_surface,
                  gfx::GpuPreference gpu_preference) final;
  void Destroy() final;
  bool MakeCurrent(gfx::GLSurface* surface) final;
  void ReleaseCurrent(gfx::GLSurface* surface) final;
  bool IsCurrent(gfx::GLSurface* surface) final;

 protected:
  GLContextAdopted(gfx::GLShareGroup* share_group);

 private:
  void OnSetSwapInterval(int interval) final;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GL_CONTEXT_ADOPTED_H_
