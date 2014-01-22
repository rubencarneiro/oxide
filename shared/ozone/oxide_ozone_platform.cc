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
#include "ui/ozone/ozone_platform.h"

#include "oxide_ozone_surface_factory.h"

namespace oxide {

class OzonePlatform : public ui::OzonePlatform {
 public:
  OzonePlatform() {}
  virtual ~OzonePlatform() {}

  gfx::SurfaceFactoryOzone* GetSurfaceFactoryOzone() OVERRIDE {
    return &surface_factory_;
  }

  ui::EventFactoryOzone* GetEventFactoryOzone() OVERRIDE {
    return NULL;
  }

  ui::InputMethodContextFactoryOzone*
  GetInputMethodContextFactoryOzone() OVERRIDE {
    return NULL;
  }

 private:
  OzoneSurfaceFactory surface_factory_;
};

} // namespace oxide

namespace ui {

OzonePlatform* CreateOzonePlatformOxide() {
  return new oxide::OzonePlatform();
}

} // namespace ui
