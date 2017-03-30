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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"

#include "shared/browser/chrome_controller_observer.h"
#include "shared/browser/touch_selection/touch_editing_menu_client.h"
#include "shared/browser/touch_selection/touch_editing_menu_controller.h"
#include "shared/common/oxide_shared_export.h"

namespace oxide {

class TouchEditingMenu;
class TouchEditingMenuControllerClient;

class OXIDE_SHARED_EXPORT TouchEditingMenuControllerImpl
    : public TouchEditingMenuController,
      public TouchEditingMenuClient,
      public ChromeControllerObserver {
 public:
  TouchEditingMenuControllerImpl(TouchEditingMenuControllerClient* client);
  ~TouchEditingMenuControllerImpl() override;

  // TouchEditingMenuController implementation
  void OnSelectionEvent(ui::SelectionEventType event) override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;
  void TouchSelectionControllerSwapped() override;
  void SetViewportBounds(const gfx::RectF& bounds) override;
  void SetTopLevelWindowBounds(const gfx::Rect& bounds) override;

 private:
  float ComputeXOffset() const;
  float ComputeYOffset() const;

  void ClearSelection();

  bool ShowMenu();
  void UpdateMenuVisibility();
  void UpdateMenuPosition();
  void ClearMenu();

  void RecomputeViewportBoundsInWindow();

  // TouchEditingMenuClient implementation
  void ExecuteCommand(WebContextMenuAction action) override;
  void Close() override;
  void WasResized() override;

  // ChromeControllerObserver implementation
  void ContentOrTopControlsOffsetChanged() override;

  TouchEditingMenuControllerClient* client_;

  gfx::RectF viewport_bounds_;
  gfx::RectF viewport_bounds_in_window_;

  gfx::Rect top_level_window_bounds_;

  bool insertion_menu_pending_ = false;

  bool handle_drag_in_progress_ = false;

  std::unique_ptr<TouchEditingMenu> menu_;

  base::WeakPtrFactory<TouchEditingMenuControllerImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(TouchEditingMenuControllerImpl);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_
