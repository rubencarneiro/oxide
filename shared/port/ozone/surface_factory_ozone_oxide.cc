// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "surface_factory_ozone_oxide.h"

#include "ui/gfx/gfx_utils_oxide.h"

namespace ui {

intptr_t SurfaceFactoryOzoneOxide::GetNativeDisplay() {
  return gfx::GetOxideNativeDisplay();
}

bool SurfaceFactoryOzoneOxide::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) {
  return false;
}

SurfaceFactoryOzoneOxide::SurfaceFactoryOzoneOxide() {}

} // namespace ui
