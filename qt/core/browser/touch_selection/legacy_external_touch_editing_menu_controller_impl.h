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

#ifndef _OXIDE_QT_CORE_BROWSER_LEGACY_EXTERNAL_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_
#define _OXIDE_QT_CORE_BROWSER_LEGACY_EXTERNAL_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_

#include "base/macros.h"

#include "qt/core/glue/legacy_external_touch_editing_menu_controller.h"
#include "shared/browser/chrome_controller_observer.h"
#include "shared/browser/touch_selection/touch_editing_menu_controller.h"

namespace oxide {

class TouchEditingMenuControllerClient;

namespace qt {

class ContentsViewImpl;
class LegacyExternalTouchEditingMenuControllerDelegate;

class LegacyExternalTouchEditingMenuControllerImpl
    : public oxide::TouchEditingMenuController,
      public oxide::ChromeControllerObserver,
      public LegacyExternalTouchEditingMenuController {
 public:
  LegacyExternalTouchEditingMenuControllerImpl(
      ContentsViewImpl* view,
      oxide::TouchEditingMenuControllerClient* client,
      LegacyExternalTouchEditingMenuControllerDelegate* delegate);
  ~LegacyExternalTouchEditingMenuControllerImpl() override;

 private:
  void NotifyStatusChanged(bool handle_drag_in_progress);

  // oxide::TouchEditingMenuController implementation
  void OnSelectionEvent(ui::SelectionEventType event) override;
  bool HandleContextMenu(const content::ContextMenuParams& params) override;
  void TouchSelectionControllerSwapped() override;
  void SetViewportBounds(const gfx::RectF& bounds) override;
  void SetTopLevelWindowBounds(const gfx::Rect& bounds) override;

  // oxide::ChromeControllerObserver implementation
  void ContentOrTopControlsOffsetChanged() override;

  // LegacyExternalTouchEditingMenuController implementation
  void HideAndDisallowShowingAutomatically() override;

  ContentsViewImpl* view_;

  oxide::TouchEditingMenuControllerClient* client_;

  DISALLOW_COPY_AND_ASSIGN(LegacyExternalTouchEditingMenuControllerImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_LEGACY_EXTERNAL_TOUCH_EDITING_MENU_CONTROLLER_IMPL_H_
