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

#include "legacy_external_touch_editing_menu_controller.h"

#include "base/logging.h"

#include "legacy_external_touch_editing_menu_controller_delegate.h"

namespace oxide {
namespace qt {

LegacyExternalTouchEditingMenuController
    ::LegacyExternalTouchEditingMenuController(
        LegacyExternalTouchEditingMenuControllerDelegate* delegate)
        : delegate_(delegate) {
  DCHECK(!delegate_->controller_);
  delegate_->controller_ = this;
}

void LegacyExternalTouchEditingMenuController::ClearDelegate() {
  DCHECK(delegate_);
  DCHECK_EQ(delegate_->controller_, this);
  delegate_->controller_ = nullptr;
  delegate_ = nullptr;
}

LegacyExternalTouchEditingMenuController
    ::~LegacyExternalTouchEditingMenuController() {
  if (delegate_) {
    ClearDelegate();
  }
}

} // namespace qt
} // namespace oxide
