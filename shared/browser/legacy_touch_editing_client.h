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

#ifndef _OXIDE_SHARED_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_H_

#include "ui/touch_selection/touch_selection_controller.h"

namespace gfx {
class RectF;
}

namespace oxide {

class LegacyTouchEditingController;

// This interface exists purely to support the deprecated
// OxideQQuickTouchSelectionController API
class LegacyTouchEditingClient {
 public:
  virtual ~LegacyTouchEditingClient() = default;

  virtual void StatusChanged(ui::TouchSelectionController::ActiveStatus status,
                             const gfx::RectF& bounds,
                             bool handle_drag_in_progress) = 0;

  virtual void InsertionHandleTapped() = 0;

  virtual void ContextMenuIntercepted() = 0;

 protected:
  LegacyTouchEditingClient() = default;

  LegacyTouchEditingController* controller() const { return controller_; }

 private:
  friend class LegacyTouchEditingController;

  LegacyTouchEditingController* controller_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(LegacyTouchEditingClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_LEGACY_TOUCH_EDITING_CLIENT_H_
