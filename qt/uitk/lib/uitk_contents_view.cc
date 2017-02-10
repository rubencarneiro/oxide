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

#include "uitk_contents_view.h"

#include <QtDebug>

#include "qt/core/glue/touch_handle_drawable.h"

#include "uitk_touch_handle_drawable.h"
#include "uitk_web_popup_menu.h"

namespace oxide {
namespace uitk {

using qt::MenuItem;

std::unique_ptr<qt::WebPopupMenu> ContentsView::CreateWebPopupMenu(
    const std::vector<qt::MenuItem>& items,
    bool allow_multiple_selection,
    const QRect& bounds,
    qt::WebPopupMenuClient* client) {
  if (allow_multiple_selection) {
    qCritical() <<
        "uitk::ContentsView: Failed to create popup menu - menus that allow "
        "multiple selection are not supported and are meant to be handled "
        "in Blink";
    return nullptr;
  }

  return WebPopupMenu::Create(item(), items, bounds, client);
}

std::unique_ptr<qt::TouchHandleDrawable>
ContentsView::CreateTouchHandleDrawable() {
  return TouchHandleDrawable::Create(item());
}

void ContentsView::TouchSelectionChanged(
    qt::TouchSelectionControllerActiveStatus status,
    const QRectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) {
}

void ContentsView::ContextMenuIntercepted() const {
}

ContentsView::ContentsView(QQuickItem* item)
    : qquick::ContentsView(item) {}

ContentsView::~ContentsView() = default;

} // namespace uitk
} // namespace oxide
