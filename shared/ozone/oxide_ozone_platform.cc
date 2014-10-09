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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "ui/display/types/native_display_delegate.h"
#include "ui/ozone/public/gpu_platform_support.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window.h"

#include "oxide_ozone_surface_factory.h"

namespace oxide {

class OzonePlatform : public ui::OzonePlatform {
 public:
  OzonePlatform()
      : gpu_platform_support_host_(ui::CreateStubGpuPlatformSupportHost()),
        gpu_platform_support_(ui::CreateStubGpuPlatformSupport()) {}

  virtual ~OzonePlatform() {}

  ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() FINAL {
    return &surface_factory_;
  }

  ui::CursorFactoryOzone* GetCursorFactoryOzone() FINAL {
    return NULL;
  }

  ui::GpuPlatformSupport* GetGpuPlatformSupport() FINAL {
    return gpu_platform_support_.get();
  }

  ui::GpuPlatformSupportHost* GetGpuPlatformSupportHost() FINAL {
    return gpu_platform_support_host_.get();
  }

  scoped_ptr<ui::PlatformWindow> CreatePlatformWindow(
      ui::PlatformWindowDelegate* delegate,
      const gfx::Rect& bounds) FINAL {
    return scoped_ptr<ui::PlatformWindow>();
  }

  scoped_ptr<ui::NativeDisplayDelegate> CreateNativeDisplayDelegate() FINAL {
    return scoped_ptr<ui::NativeDisplayDelegate>();
  }

 private:
  void InitializeUI() OVERRIDE {}
  void InitializeGPU() OVERRIDE {}

  OzoneSurfaceFactory surface_factory_;
  scoped_ptr<ui::GpuPlatformSupportHost> gpu_platform_support_host_;
  scoped_ptr<ui::GpuPlatformSupport> gpu_platform_support_;
};

} // namespace oxide

namespace ui {

OzonePlatform* CreateOzonePlatformOxide() {
  return new oxide::OzonePlatform();
}

} // namespace ui
