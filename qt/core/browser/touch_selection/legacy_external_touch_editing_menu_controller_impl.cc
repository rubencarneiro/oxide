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

#include "legacy_external_touch_editing_menu_controller_impl.h"

#include "ui/gfx/geometry/rect_f.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "qt/core/browser/contents_view_impl.h"
#include "qt/core/browser/oxide_qt_dpi_utils.h"
#include "qt/core/browser/oxide_qt_type_conversions.h"
#include "qt/core/glue/legacy_external_touch_editing_menu_controller_delegate.h"
#include "qt/core/glue/macros.h"
#include "qt/core/glue/web_context_menu_params.h"
#include "shared/browser/chrome_controller.h"
#include "shared/browser/touch_selection/touch_editing_menu_controller_client.h"

namespace oxide {
namespace qt {

using oxide::TouchEditingMenuControllerClient;

void LegacyExternalTouchEditingMenuControllerImpl::NotifyStatusChanged(
    bool handle_drag_in_progress) {
  ui::TouchSelectionController* tsc = client_->GetTouchSelectionController();
  ui::TouchSelectionController::ActiveStatus status =
      tsc ? tsc->active_status() : ui::TouchSelectionController::INACTIVE;

  gfx::RectF bounds;
  if (tsc) {
    bounds = tsc->GetRectBetweenBounds();
    bounds.Offset(0, client_->GetChromeController()->GetTopContentOffset());
  }

  delegate()->StatusChanged(
      static_cast<LegacyExternalTouchEditingMenuControllerDelegate::ActiveStatus>(status),
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(bounds, view_->GetScreen())),
      handle_drag_in_progress);
}

void LegacyExternalTouchEditingMenuControllerImpl::OnSelectionEvent(
    ui::SelectionEventType event) {
  if (!delegate()) {
    return;
  }

  if (event == ui::INSERTION_HANDLE_TAPPED) {
    delegate()->InsertionHandleTapped();
    return;
  }

  bool handle_drag_in_progress =
      event == ui::SELECTION_HANDLE_DRAG_STARTED ||
          event == ui::INSERTION_HANDLE_DRAG_STARTED;
  NotifyStatusChanged(handle_drag_in_progress);
}

bool LegacyExternalTouchEditingMenuControllerImpl::HandleContextMenu(
    const content::ContextMenuParams& params) {
  if (!delegate()) {
    return false;
  }

  return delegate()->HandleContextMenu(
      WebContextMenuParams::From(params, view_->GetScreen()));
}

void LegacyExternalTouchEditingMenuControllerImpl
    ::TouchSelectionControllerSwapped() {
  NotifyStatusChanged(false);
}

void LegacyExternalTouchEditingMenuControllerImpl::SetViewportBounds(
    const gfx::RectF& bounds) {}

void LegacyExternalTouchEditingMenuControllerImpl::SetTopLevelWindowBounds(
    const gfx::Rect& bounds) {}

void LegacyExternalTouchEditingMenuControllerImpl
    ::ContentOrTopControlsOffsetChanged() {
  NotifyStatusChanged(false);
}

void LegacyExternalTouchEditingMenuControllerImpl
    ::HideAndDisallowShowingAutomatically() {
  client_->GetTouchSelectionController()->HideAndDisallowShowingAutomatically();
}

LegacyExternalTouchEditingMenuControllerImpl
    ::LegacyExternalTouchEditingMenuControllerImpl(
        ContentsViewImpl* view,
        TouchEditingMenuControllerClient* client,
        LegacyExternalTouchEditingMenuControllerDelegate* delegate)
        : oxide::ChromeControllerObserver(client->GetChromeController()),
          LegacyExternalTouchEditingMenuController(delegate),
          view_(view),
          client_(client) {
  STATIC_ASSERT_MATCHING_ENUM(
      LegacyExternalTouchEditingMenuControllerDelegate::ActiveStatus::INACTIVE,
      ui::TouchSelectionController::INACTIVE);
  STATIC_ASSERT_MATCHING_ENUM(
      LegacyExternalTouchEditingMenuControllerDelegate::ActiveStatus::INSERTION_ACTIVE,
      ui::TouchSelectionController::INSERTION_ACTIVE);
  STATIC_ASSERT_MATCHING_ENUM(
      LegacyExternalTouchEditingMenuControllerDelegate::ActiveStatus::SELECTION_ACTIVE,
      ui::TouchSelectionController::SELECTION_ACTIVE);
}

LegacyExternalTouchEditingMenuControllerImpl
    ::~LegacyExternalTouchEditingMenuControllerImpl() = default;

} // namespace qt
} // namespace oxide
