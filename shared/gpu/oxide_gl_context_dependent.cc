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

#include "oxide_gl_context_dependent.h"

#include "base/logging.h"
#include "ui/gl/gpu_timing.h"

namespace oxide {

void GLContextDependent::OnSetSwapInterval(int interval) {}

GLContextDependent::GLContextDependent(GLContextDependent* other)
    : gfx::GLContext(nullptr),
      handle_(other->GetHandle()),
      was_allocated_using_robustness_extension_(
        other->WasAllocatedUsingRobustnessExtension()) {}

GLContextDependent::GLContextDependent(
    void* handle,
    bool was_allocated_using_robustness_extension)
    : gfx::GLContext(nullptr),
      handle_(handle),
      was_allocated_using_robustness_extension_(
        was_allocated_using_robustness_extension) {}

GLContextDependent::~GLContextDependent() {}

// static
scoped_refptr<GLContextDependent> GLContextDependent::CloneFrom(
    GLContextDependent* other) {
  return make_scoped_refptr(new GLContextDependent(other));
}

bool GLContextDependent::Initialize(gfx::GLSurface* compatible_surface,
                                    gfx::GpuPreference gpu_preference) {
  return true;
}

void GLContextDependent::Destroy() {}

bool GLContextDependent::MakeCurrent(gfx::GLSurface* surface) {
  NOTREACHED();
  return false;
}

void GLContextDependent::ReleaseCurrent(gfx::GLSurface* surface) {}

bool GLContextDependent::IsCurrent(gfx::GLSurface* surface) {
  return false;
}

void* GLContextDependent::GetHandle() {
  return handle_;
}

bool GLContextDependent::WasAllocatedUsingRobustnessExtension() {
  return was_allocated_using_robustness_extension_;
}

scoped_refptr<gfx::GPUTimingClient> GLContextDependent::CreateGPUTimingClient() {
  return nullptr;
}

} // namespace oxide
