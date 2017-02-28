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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_H_

#include "ui/touch_selection/selection_event_type.h"

namespace content {
class ContextMenuParams;
}

namespace gfx {
class Rect;
class RectF;
}

namespace oxide {

class TouchEditingMenuController {
 public:
  virtual ~TouchEditingMenuController() = default;

  virtual void OnSelectionEvent(ui::SelectionEventType event) = 0;

  virtual bool HandleContextMenu(const content::ContextMenuParams& params) = 0;

  virtual void TouchSelectionControllerSwapped() = 0;

  virtual void SetViewportBounds(const gfx::RectF& bounds) = 0;

  virtual void SetTopLevelWindowBounds(const gfx::Rect& bounds) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_H_
