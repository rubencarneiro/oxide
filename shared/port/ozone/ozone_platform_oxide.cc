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

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "ui/display/types/native_display_delegate.h"
#include "ui/ozone/common/stub_client_native_pixmap_factory.h"
#include "ui/ozone/public/gpu_platform_support.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/ozone/public/system_input_injector.h"
#include "ui/platform_window/platform_window.h"

#include "surface_factory_ozone_oxide.h"

namespace ui {

class OzonePlatformOxide : public OzonePlatform {
 public:
  OzonePlatformOxide()
      : gpu_platform_support_host_(CreateStubGpuPlatformSupportHost()),
        gpu_platform_support_(CreateStubGpuPlatformSupport()) {}

  virtual ~OzonePlatformOxide() {}

  SurfaceFactoryOzone* GetSurfaceFactoryOzone() override {
    return &surface_factory_;
  }

  OverlayManagerOzone* GetOverlayManager() override {
    return nullptr;
  }

  CursorFactoryOzone* GetCursorFactoryOzone() override {
    return nullptr;
  }

  ui::InputController* GetInputController() override {
    return nullptr;
  }

  GpuPlatformSupport* GetGpuPlatformSupport() override {
    return gpu_platform_support_.get();
  }

  GpuPlatformSupportHost* GetGpuPlatformSupportHost() override {
    return gpu_platform_support_host_.get();
  }

  std::unique_ptr<SystemInputInjector> CreateSystemInputInjector() override {
    return nullptr;
  }

  std::unique_ptr<PlatformWindow> CreatePlatformWindow(
      PlatformWindowDelegate* delegate,
      const gfx::Rect& bounds) override {
    return nullptr;
  }

  std::unique_ptr<NativeDisplayDelegate> CreateNativeDisplayDelegate() override {
    return nullptr;
  }

 private:
  void InitializeUI() override {}
  void InitializeGPU() override {}

  SurfaceFactoryOzoneOxide surface_factory_;
  std::unique_ptr<GpuPlatformSupportHost> gpu_platform_support_host_;
  std::unique_ptr<GpuPlatformSupport> gpu_platform_support_;
};

OzonePlatform* CreateOzonePlatformOxide() {
  return new OzonePlatformOxide();
}

ClientNativePixmapFactory* CreateClientNativePixmapFactoryOxide() {
  return CreateStubClientNativePixmapFactory();
}

} // namespace ui
