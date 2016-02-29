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

#include "base/logging.h"

namespace oxide {

FormFactor DetectFormFactorHintImpl(const gfx::Size& primary_screen_size);

FormFactor DetectFormFactorHint(const gfx::Size& primary_screen_size) {
  const char* force = getenv("OXIDE_FORCE_FORM_FACTOR");
  if (force) {
    if (!strcmp(force, "desktop")) {
      return FORM_FACTOR_DESKTOP;
    } else if (!strcmp(force, "tablet")) {
      return FORM_FACTOR_TABLET;
    } else if (!strcmp(force, "phone")) {
      return FORM_FACTOR_PHONE;
    } else {
      LOG(ERROR) << "Unrecognized value for OXIDE_FORCE_FORM_FACTOR";
    }
  }

  return DetectFormFactorHintImpl(primary_screen_size);
}

} // namespace oxide
