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

#include "oxide_shared_gl_context.h"

#include "ui/gl/gl_share_group.h"

#include "base/logging.h"

namespace oxide {

namespace {

class GLShareGroup final : public gfx::GLShareGroup {
 public:
  GLShareGroup(SharedGLContext* context) :
      context_(context) {}

  gfx::GLContext* GetContext() final { return context_; }
  void ClearContext() { context_ = NULL; }

 private:
  ~GLShareGroup() {
    DCHECK(!context_);
  }

  SharedGLContext* context_;
};

} // namespace

SharedGLContext::SharedGLContext() :
    gfx::GLContext(new GLShareGroup(this)) {}

SharedGLContext::~SharedGLContext() {
  oxide::GLShareGroup* sg = static_cast<oxide::GLShareGroup *>(share_group());
  DCHECK_EQ(sg->GetContext(), this);
  sg->ClearContext();
}

bool SharedGLContext::Initialize(gfx::GLSurface* compatible_surface,
                                 gfx::GpuPreference gpu_preference) {
  return true;
}

void SharedGLContext::Destroy() {}

bool SharedGLContext::MakeCurrent(gfx::GLSurface* surface) {
  NOTREACHED();
  return false;
}

void SharedGLContext::ReleaseCurrent(gfx::GLSurface* surface) {}

bool SharedGLContext::IsCurrent(gfx::GLSurface* surface) {
  return false;
}

void SharedGLContext::SetSwapInterval(int interval) {}

} // namespace oxide
