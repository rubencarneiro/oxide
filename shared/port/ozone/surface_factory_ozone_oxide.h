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

#ifndef _OXIDE_SHARED_PORT_OZONE_SURFACE_FACTORY_OXIDE_H_
#define _OXIDE_SHARED_PORT_OZONE_SURFACE_FACTORY_OXIDE_H_

#include "ui/ozone/public/surface_factory_ozone.h"

namespace ui {

class SurfaceFactoryOzoneOxide : public SurfaceFactoryOzone {
 public:
  SurfaceFactoryOzoneOxide();

 private:
  intptr_t GetNativeDisplay() override;
  bool LoadEGLGLES2Bindings(
      AddGLLibraryCallback add_gl_library,
      SetGLGetProcAddressProcCallback set_gl_get_proc_address) override;
  const int32_t* GetEGLSurfaceProperties(const int32_t* desired_list) override;
};

} // namespace ui

#endif // _OXIDE_SHARED_PORT_OZONE_SURFACE_FACTORY_OXIDE_H_
