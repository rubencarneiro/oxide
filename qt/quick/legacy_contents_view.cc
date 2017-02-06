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

#include "legacy_contents_view.h"

#include "qt/core/glue/web_popup_menu.h"
#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"

#include "oxide_qquick_touch_handle_drawable.h"
#include "qquick_legacy_web_popup_menu.h"

namespace oxide {
namespace qquick {

std::unique_ptr<qt::WebPopupMenu> LegacyContentsView::CreateWebPopupMenu(
    const std::vector<qt::MenuItem>& items,
    bool allow_multiple_selection,
    const QRect& bounds,
    qt::WebPopupMenuClient* client) {
  if (!popup_menu_) {
    return nullptr;
  }

  return std::unique_ptr<qt::WebPopupMenu>(
      new LegacyWebPopupMenu(item(),
                             popup_menu_,
                             items,
                             allow_multiple_selection,
                             bounds,
                             client));
}

qt::TouchHandleDrawableProxy* LegacyContentsView::CreateTouchHandleDrawable() {
  return new TouchHandleDrawable(item(), touch_selection_controller_.get());
}

void LegacyContentsView::TouchSelectionChanged(
    qt::TouchSelectionControllerActiveStatus status,
    const QRectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) {
  if (touch_selection_controller_) {
    touch_selection_controller_->onTouchSelectionChanged(
        static_cast<OxideQQuickTouchSelectionController::Status>(status),
        bounds,
        handle_drag_in_progress,
        insertion_handle_tapped);
  }
}

void LegacyContentsView::ContextMenuIntercepted() const {
  if (touch_selection_controller_) {
    Q_EMIT touch_selection_controller_->contextMenuIntercepted();
  }
}

LegacyContentsView::LegacyContentsView(QQuickItem* item)
    : ContentsView(item),
      touch_selection_controller_(
          new OxideQQuickTouchSelectionController(this)) {}

LegacyContentsView::~LegacyContentsView() = default;

void LegacyContentsView::HideTouchSelectionController() const {
  if (!view()) {
    return;
  }

  view()->hideTouchSelectionController();
}

} // namespace qquick
} // namespace oxide
