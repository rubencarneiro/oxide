// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "oxide_form_factor.h"

#include <algorithm>

#include "base/logging.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"

#include "oxide_android_properties.h"
#include "oxide_browser_platform_integration.h"

namespace oxide {

FormFactor GetFormFactorHint() {
  static bool initialized = false;
  static FormFactor form_factor = FORM_FACTOR_DESKTOP;

  if (initialized) {
    return form_factor;
  }

  const char* force = getenv("OXIDE_FORCE_FORM_FACTOR");
  if (force) {
    initialized = true;
    if (!strcmp(force, "desktop")) {
      form_factor = FORM_FACTOR_DESKTOP;
    } else if (!strcmp(force, "tablet")) {
      form_factor = FORM_FACTOR_TABLET;
    } else if (!strcmp(force, "phone")) {
      form_factor = FORM_FACTOR_PHONE;
    } else {
      LOG(ERROR) << "Unrecognized value for OXIDE_FORCE_FORM_FACTOR";
      initialized = false;
    }
  }

  if (initialized) {
    return form_factor;
  }

  initialized = true;

  if (AndroidProperties::GetInstance()->Available()) {
    // Ubuntu on phones and tablets currently uses an Android kernel and EGL
    // stack. If we detect these, assume we are a phone or tablet. The screen
    // size check here is basically the same as Chrome for Android, where
    // a minimum DIP width of less than 600 is a phone
    blink::WebScreenInfo screen(
        BrowserPlatformIntegration::GetInstance()->GetDefaultScreenInfo());
    if (std::min(screen.rect.width / screen.deviceScaleFactor,
                 screen.rect.height / screen.deviceScaleFactor) >= 600) {
      form_factor = FORM_FACTOR_TABLET;
    } else {
      form_factor = FORM_FACTOR_PHONE;
    }
  } else {
    // If this is not an Ubuntu phone or tablet, assume desktop linux for now.
    // We could be cleverer here, eg, using /sys/class/dmi/id/chassis_type
    // or something like that. But this is good enough for now
  }

  return form_factor;
}

} // namespace oxide
