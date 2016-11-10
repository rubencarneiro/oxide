// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "screen_form_factor.h"

#include "base/logging.h"
#include "ui/display/display.h"

#include "qt/core/browser/qt_screen.h"
#include "shared/browser/display_form_factor.h"

#include "macros.h"

namespace oxide {
namespace qt {

ScreenFormFactor GetScreenFormFactor(QScreen* screen) {
  STATIC_ASSERT_MATCHING_ENUM(ScreenFormFactor::Monitor,
                              oxide::DisplayFormFactor::Monitor)
  STATIC_ASSERT_MATCHING_ENUM(ScreenFormFactor::Mobile,
                              oxide::DisplayFormFactor::Mobile)
  STATIC_ASSERT_MATCHING_ENUM(ScreenFormFactor::Television,
                              oxide::DisplayFormFactor::Television)

  display::Display display = Screen::GetInstance()->DisplayFromQScreen(screen);
  DCHECK(display.is_valid());
  return static_cast<ScreenFormFactor>(
      Screen::GetInstance()->GetDisplayFormFactor(display));
}

} // namespace qt
} // namespace oxide
