// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qt_web_popup_menu.h"

#include <QList>
#include <QRect>
#include <QString>
#include <QVector>
#include <string>
#include <vector>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/menu_item.h"
#include "ui/gfx/geometry/rect.h"

#include "qt/core/glue/oxide_qt_web_popup_menu_proxy.h"

namespace oxide {
namespace qt {

WebPopupMenu::~WebPopupMenu() {}

void WebPopupMenu::Show(const gfx::Rect& bounds,
                        const std::vector<content::MenuItem>& items,
                        int selected_item,
                        bool allow_multiple_selection) {
  QList<MenuItem> qitems;
  QString current_group;
  // We get a vector of size_t number of elements but Chromium uses
  // an int when responding with the index of selected items, so
  // we truncate anything above INT_MAX number of items
  // XXX: Should this ever happen?
  // XXX: And shouldn't we do this in shared/?
  // XXX: Does it matter?

  if (items.size() > INT_MAX) {
    LOG(WARNING) << "Number of menu items exceeds maximum";
  }

  for (size_t i = 0;
       i < items.size() && i <= static_cast<size_t>(INT_MAX);
       ++i) {
    const content::MenuItem& item = items[i];

    if (item.type == content::MenuItem::GROUP) {
      current_group = QString::fromStdString(base::UTF16ToUTF8(item.label));
      continue;
    }

    DCHECK(item.type == content::MenuItem::SEPARATOR ||
           item.type == content::MenuItem::OPTION);

    MenuItem qitem;

    qitem.label = QString::fromStdString(base::UTF16ToUTF8(item.label));
    qitem.tooltip = QString::fromStdString(base::UTF16ToUTF8(item.tool_tip));
    qitem.group = current_group;
    // This cast is ok, as this is guaranteed not to cast to a negative index
    qitem.index = static_cast<int>(i);
    qitem.enabled = item.enabled;
    qitem.checked = item.checked || (!allow_multiple_selection && 
                                     selected_item == qitem.index);
    qitem.separator = item.type == content::MenuItem::SEPARATOR;

    DCHECK(allow_multiple_selection || qitem.index == selected_item || !qitem.checked);
 
    qitems.append(qitem);
  }

  proxy_->Show(QRect(bounds.x(), bounds.y(), bounds.width(), bounds.height()),
               qitems, allow_multiple_selection);
}

void WebPopupMenu::Hide() {
  proxy_->Hide();
}

void WebPopupMenu::selectItems(const QList<int>& selected_indices) {
  SelectItems(selected_indices.toVector().toStdVector());
}

void WebPopupMenu::cancel() {
  Cancel();
}

WebPopupMenu::WebPopupMenu(content::RenderFrameHost* rfh)
    : oxide::WebPopupMenu(rfh) {}

void WebPopupMenu::SetProxy(WebPopupMenuProxy* proxy) {
  proxy_.reset(proxy);
}

} // namespace qt
} // namespace oxide
