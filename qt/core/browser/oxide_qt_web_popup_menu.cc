// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"
#include "qt/core/glue/oxide_qt_web_popup_menu_proxy.h"
#include "shared/browser/oxide_web_popup_menu_client.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

QList<MenuItem> BuildMenuItems(const std::vector<content::MenuItem>& items,
                               int selected_index,
                               bool allow_multiple_selection) {
  QList<MenuItem> results;

  int i = -1;
  QString current_group;

  for (const auto& item : items) {
    if (i == std::numeric_limits<int>::max()) {
      LOG(WARNING) << "Truncating menu - there are too many items!";
      break;
    }

    int index = ++i;

    if (item.type == content::MenuItem::GROUP) {
      current_group = QString::fromStdString(base::UTF16ToUTF8(item.label));
      continue;
    }

    // We don't support submenus here
    DCHECK(item.type == content::MenuItem::SEPARATOR ||
           item.type == content::MenuItem::OPTION);

    MenuItem mi;

    mi.label = QString::fromStdString(base::UTF16ToUTF8(item.label));
    mi.tooltip = QString::fromStdString(base::UTF16ToUTF8(item.tool_tip));
    mi.group = current_group;
    mi.index = index;
    mi.enabled = item.enabled;
    mi.checked =
        item.checked || (!allow_multiple_selection && 
                         selected_index == mi.index);
    mi.separator = item.type == content::MenuItem::SEPARATOR;

    // We're not using content::MenuItem::action here - if this function gets
    // re-used for custom items in context menus, we'll need that

    DCHECK(allow_multiple_selection ||
           mi.index == selected_index ||
           !mi.checked);
 
    results.append(mi);
  }

  return results;
}

}

void WebPopupMenu::Show(const gfx::Rect& bounds) {
  if (!proxy_) {
    client_->Cancel();
    return;
  }

  ContentsView* view = ContentsView::FromWebContents(contents_);

  gfx::Rect rect = bounds;
  rect += gfx::Vector2d(0, view->GetTopContentOffset());
  
  proxy_->Show(
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(rect, view->GetScreen())));
}

void WebPopupMenu::Hide() {
  contents_ = nullptr;

  if (!proxy_) {
    return;
  }

  proxy_->Hide();
}

void WebPopupMenu::selectItems(const QList<int>& selected_indices) {
  client_->SelectItems(selected_indices.toVector().toStdVector());
}

void WebPopupMenu::cancel() {
  client_->Cancel();
}

WebPopupMenu::WebPopupMenu(ContentsView* view,
                           const std::vector<content::MenuItem>& items,
                           int selected_index,
                           bool allow_multiple_selection,
                           oxide::WebPopupMenuClient* client)
    : client_(client),
      contents_(view->GetWebContents()) {
  if (!view->client()) {
    return;
  }

  proxy_ =
      view->client()->CreateWebPopupMenu(
          BuildMenuItems(items, selected_index, allow_multiple_selection),
          allow_multiple_selection, this);
}

WebPopupMenu::~WebPopupMenu() {}

} // namespace qt
} // namespace oxide
