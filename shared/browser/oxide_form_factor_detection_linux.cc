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

#include "oxide_form_factor_detection.h"

#include <algorithm>

#include "ui/gfx/geometry/size.h"

#include "shared/common/oxide_form_factor.h"

#if defined(ENABLE_HYBRIS)
#include "hybris_utils.h"
#endif

namespace oxide {

FormFactor DetectFormFactorHintImpl(const gfx::Size& primary_screen_size) {
#if defined(ENABLE_HYBRIS)
  if (HybrisUtils::GetInstance()->HasDeviceProperties()) {
    // Ubuntu on phones and tablets currently uses an Android kernel and EGL
    // stack. If we detect these, assume we are a phone or tablet. The screen
    // size check here is basically the same as Chrome for Android, where
    // a minimum DIP width of less than 600 is a phone
    if (std::min(primary_screen_size.width(),
                 primary_screen_size.height()) >= 600) {
      return FORM_FACTOR_TABLET;
    }

    return FORM_FACTOR_PHONE;
  }
#endif

  // If this is not an Ubuntu phone or tablet, assume desktop linux for now.
  // We could be cleverer here, eg, using /sys/class/dmi/id/chassis_type
  // or something like that. But this is good enough for now

  return FORM_FACTOR_DESKTOP;
}

} // namespace oxide
