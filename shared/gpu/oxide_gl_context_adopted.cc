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

#include "oxide_gl_context_adopted.h"

#include "base/logging.h"

namespace oxide {

void GLContextAdopted::OnSetSwapInterval(int interval) {}

GLContextAdopted::GLContextAdopted(GLContextAdopted* other)
    : gfx::GLContext(nullptr),
      handle_(other->GetHandle()),
      was_allocated_using_robustness_extension_(
        other->WasAllocatedUsingRobustnessExtension()) {}

GLContextAdopted::GLContextAdopted(
    void* handle,
    bool was_allocated_using_robustness_extension)
    : gfx::GLContext(nullptr),
      handle_(handle),
      was_allocated_using_robustness_extension_(
        was_allocated_using_robustness_extension) {}

GLContextAdopted::~GLContextAdopted() {}

// static
scoped_refptr<GLContextAdopted> GLContextAdopted::CloneFrom(
    GLContextAdopted* other) {
  return make_scoped_refptr(new GLContextAdopted(other));
}

bool GLContextAdopted::Initialize(gfx::GLSurface* compatible_surface,
                                  gfx::GpuPreference gpu_preference) {
  return true;
}

void GLContextAdopted::Destroy() {}

bool GLContextAdopted::MakeCurrent(gfx::GLSurface* surface) {
  NOTREACHED();
  return false;
}

void GLContextAdopted::ReleaseCurrent(gfx::GLSurface* surface) {}

bool GLContextAdopted::IsCurrent(gfx::GLSurface* surface) {
  return false;
}

void* GLContextAdopted::GetHandle() {
  return handle_;
}

bool GLContextAdopted::WasAllocatedUsingRobustnessExtension() {
  return was_allocated_using_robustness_extension_;
}

} // namespace oxide
