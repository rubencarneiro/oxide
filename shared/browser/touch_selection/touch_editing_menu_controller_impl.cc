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

#include "touch_editing_menu_controller_impl.h"

#include <algorithm>

#include "base/logging.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/context_menu_params.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/size_f.h"
#include "ui/gfx/geometry/vector2d_f.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/browser/chrome_controller.h"
#include "shared/browser/context_menu/web_context_menu_actions.h"
#include "shared/browser/web_contents_client.h"
#include "shared/common/oxide_enum_flags.h"

#include "touch_editing_menu.h"
#include "touch_editing_menu_controller_client.h"

namespace oxide {

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

blink::WebContextMenuData::EditFlags SupportedEditingCapabilities() {
  return blink::WebContextMenuData::kCanCut
      | blink::WebContextMenuData::kCanCopy
      | blink::WebContextMenuData::kCanPaste
      | blink::WebContextMenuData::kCanSelectAll;
}

}

float TouchEditingMenuControllerImpl::ComputeXOffset() const {
  gfx::RectF selection_rect =
      client_->GetTouchSelectionController()->GetRectBetweenBounds();
  int menu_width = menu_->GetSizeIncludingMargin().width();

  float x = selection_rect.x() + ((selection_rect.width() - menu_width) / 2);
  float window_x = x + viewport_bounds_in_window_.x();

  if (window_x < 0) {
    // Position at the left edge of the window
    return -viewport_bounds_in_window_.x();
  }

  int window_width = top_level_window_bounds_.width();
  if (window_x + menu_width > window_width) {
    // Position on the right edge of the window, or on the left edge if the
    // window is too small to fit it in completely
    return std::max(0, window_width - menu_width) -
        viewport_bounds_in_window_.x();
  }

  return x;
}

float TouchEditingMenuControllerImpl::ComputeYOffset() const {
  ChromeController* cc = client_->GetChromeController();
  gfx::Vector2dF top_content_offset(0, cc->GetTopContentOffset());

  ui::TouchSelectionController* tsc = client_->GetTouchSelectionController();
  gfx::RectF exclusion_area = tsc->GetRectBetweenBounds() + top_content_offset;
  if (exclusion_area.IsEmpty()) {
    exclusion_area.set_width(1.f);
  }
  exclusion_area.Union(tsc->GetStartHandleRect() + top_content_offset);
  exclusion_area.Union(tsc->GetEndHandleRect() + top_content_offset);

  float top_controls_current_height =
      cc->top_controls_height() + cc->GetTopControlsOffset();
  int menu_height = menu_->GetSizeIncludingMargin().height();

  auto top_position = [&]() -> float {
    return std::max(exclusion_area.y() - menu_height,
                    -viewport_bounds_in_window_.y());
  };

  auto bottom_position = [&]() -> float {
    return std::min(
        exclusion_area.bottom(),
        top_level_window_bounds_.height() - menu_height -
            viewport_bounds_in_window_.y());
  };

  if (exclusion_area.y() - top_controls_current_height >= menu_height) {
    // If the menu fits between the top of the view and the top of the
    // selection, then position above the selection
    return top_position();
  }

  if (viewport_bounds_in_window_.height() - exclusion_area.bottom()
      >= menu_height) {
    // ... else, if the menu fits between the bottom of the selection and the
    // bottom of the view, then position below the selection
    return bottom_position();
  }

  if (exclusion_area.y() + viewport_bounds_in_window_.y() >= menu_height) {
    // ... else, if the menu fits between the top of the window and the top of
    // the selection, then position above the selection, extending beyond the
    // view bounds
    return top_position();
  }

  if (top_level_window_bounds_.height() -
      (exclusion_area.bottom() + viewport_bounds_in_window_.y())
      >= menu_height) {
    // ... else, if the menu fits between the bottom of the selection and the
    // bottom of the window, then position below the selection, extending beyond
    // the view bounds
    return bottom_position();
  }

  // ... else, just stick it at the top of the window. It cannot fit without
  // obscuring the selection in this case
  return top_position();
}

void TouchEditingMenuControllerImpl::ClearSelection() {
  client_->GetWebContents()->CollapseSelection();
}

bool TouchEditingMenuControllerImpl::ShowMenu() {
  insertion_menu_pending_ = false;
  if (!menu_) {
    WebContentsClient* contents_client =
        WebContentsClient::FromWebContents(client_->GetWebContents());
    menu_ =
        contents_client->CreateTouchEditingMenu(
            client_->GetEditingCapabilities() & SupportedEditingCapabilities(),
            this);
  }

  if (!menu_) {
    return false;
  }

  UpdateMenuPosition();
  UpdateMenuVisibility();
  return true;
}

void TouchEditingMenuControllerImpl::UpdateMenuVisibility() {
  if (!menu_) {
    return;
  }

  ui::TouchSelectionController* tsc = client_->GetTouchSelectionController();
  if (handle_drag_in_progress_ ||
      (tsc->GetStartHandleRect().IsEmpty() &&
       tsc->GetEndHandleRect().IsEmpty())) {
    menu_->Hide();
  } else {
    menu_->Show();
  }
}

void TouchEditingMenuControllerImpl::UpdateMenuPosition() {
  if (!menu_) {
    return;
  }

  gfx::PointF origin(ComputeXOffset(), ComputeYOffset());
  menu_->SetOrigin(origin);
}

void TouchEditingMenuControllerImpl::ClearMenu() {
  insertion_menu_pending_ = false;
  menu_.reset();
}

void TouchEditingMenuControllerImpl::RecomputeViewportBoundsInWindow() {
  gfx::RectF bounds_in_window =
      viewport_bounds_ - gfx::Vector2dF(top_level_window_bounds_.x(),
                                        top_level_window_bounds_.y());
  if (bounds_in_window == viewport_bounds_in_window_) {
    return;
  }

  viewport_bounds_in_window_ = bounds_in_window;

  UpdateMenuPosition();
}

void TouchEditingMenuControllerImpl::ExecuteCommand(
    WebContextMenuAction action) {
  switch (action) {
    case WebContextMenuAction::Cut:
      client_->GetWebContents()->Cut();
      break;
    case WebContextMenuAction::Copy:
      client_->GetWebContents()->Copy();
      break;
    case WebContextMenuAction::Paste:
      client_->GetWebContents()->Paste();
      break;
    case WebContextMenuAction::SelectAll:
      client_->GetWebContents()->SelectAll();
      break;
    default:
      NOTREACHED();
  }

  if (action == WebContextMenuAction::SelectAll) {
    return;
  }

  Close();
}

void TouchEditingMenuControllerImpl::Close() {
  menu_->Hide();

  ui::TouchSelectionController* tsc = client_->GetTouchSelectionController();
  if (tsc->active_status() == ui::TouchSelectionController::SELECTION_ACTIVE) {
    ClearSelection();
  }

  std::unique_ptr<TouchEditingMenu> menu = std::move(menu_);
  tsc->HideAndDisallowShowingAutomatically();

  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE, menu.release());
}

void TouchEditingMenuControllerImpl::WasResized() {
  UpdateMenuPosition();
}

void TouchEditingMenuControllerImpl::ContentOrTopControlsOffsetChanged() {
  UpdateMenuPosition();
}

TouchEditingMenuControllerImpl::TouchEditingMenuControllerImpl(
    TouchEditingMenuControllerClient* client)
    : ChromeControllerObserver(client->GetChromeController()),
      client_(client),
      weak_ptr_factory_(this) {}

TouchEditingMenuControllerImpl::~TouchEditingMenuControllerImpl() = default;

bool TouchEditingMenuControllerImpl::HandleContextMenu(
    const content::ContextMenuParams& params) {
  if (params.source_type != ui::MENU_SOURCE_LONG_PRESS ||
      !params.is_editable ||
      !params.selection_text.empty()) {
    return false;
  }

  if (client_->GetTouchSelectionController()->active_status() ==
          ui::TouchSelectionController::INSERTION_ACTIVE) {
    ShowMenu();
  } else {
    // Don't display the menu until there is an active insertion, to stop it
    // from being initially displayed in the wrong place
    DCHECK(!menu_);
    insertion_menu_pending_ = true;
  }
  return true;
}

void TouchEditingMenuControllerImpl::OnSelectionEvent(
    ui::SelectionEventType event) {
  switch (event) {
    case ui::SELECTION_HANDLES_SHOWN:
      if (!ShowMenu()) {
        ClearSelection();
      }
      break;
    case ui::SELECTION_HANDLES_MOVED:
    case ui::INSERTION_HANDLE_MOVED:
      UpdateMenuPosition();
      UpdateMenuVisibility();
      break;
    case ui::SELECTION_HANDLES_CLEARED:
    case ui::INSERTION_HANDLE_CLEARED:
      ClearMenu();
      break;
    case ui::SELECTION_HANDLE_DRAG_STARTED:
    case ui::INSERTION_HANDLE_DRAG_STARTED:
      handle_drag_in_progress_ = true;
      UpdateMenuVisibility();
      break;
    case ui::SELECTION_HANDLE_DRAG_STOPPED:
    case ui::INSERTION_HANDLE_DRAG_STOPPED:
      handle_drag_in_progress_ = false;
      UpdateMenuVisibility();
      break;
    case ui::INSERTION_HANDLE_SHOWN:
      if (insertion_menu_pending_) {
        ShowMenu();
      }
      break;
    case ui::INSERTION_HANDLE_TAPPED:
      if (menu_) {
        ClearMenu();
      } else {
        ShowMenu();
      }
      break;
  }
}

void TouchEditingMenuControllerImpl::TouchSelectionControllerSwapped() {
  ClearMenu();
}

void TouchEditingMenuControllerImpl::SetViewportBounds(
    const gfx::RectF& bounds) {
  if (bounds == viewport_bounds_) {
    return;
  }

  viewport_bounds_ = bounds;
  RecomputeViewportBoundsInWindow();
}

void TouchEditingMenuControllerImpl::SetTopLevelWindowBounds(
    const gfx::Rect& bounds) {
  if (bounds == top_level_window_bounds_) {
    return;
  }

  bool size_changed = bounds.size() != top_level_window_bounds_.size();

  top_level_window_bounds_ = bounds;
  RecomputeViewportBoundsInWindow();

  if (!size_changed) {
    return;
  }
  UpdateMenuPosition();
}

} // namespace oxide
