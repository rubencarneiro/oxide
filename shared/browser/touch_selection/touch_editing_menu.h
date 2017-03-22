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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_H_

#include "ui/gfx/geometry/size.h"

namespace gfx {
class PointF;
}

namespace oxide {

// An interface that should be implemented by the platform's touch editing
// menu implementation
class TouchEditingMenu {
 public:
  virtual ~TouchEditingMenu() = default;

  virtual void Show() = 0;

  virtual void Hide() = 0;

  virtual gfx::Size GetSizeIncludingMargin() const = 0;

  // Set the origin of the menu in view coordinates. The origin can be outside
  // of the view, as TouchEditingMenuControllerImpl assumes it can position the
  // menu anywhere inside the window
  virtual void SetOrigin(const gfx::PointF& origin) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_H_
