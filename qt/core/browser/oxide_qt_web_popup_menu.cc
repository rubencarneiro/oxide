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
#include "content/public/common/menu_item.h"
#include "ui/gfx/geometry/rect.h"

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"
#include "qt/core/glue/oxide_qt_web_popup_menu_proxy.h"
#include "shared/browser/oxide_web_popup_menu_client.h"

#include "menu_item_builder.h"
#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

std::vector<MenuItem> BuildMenuItems(
    const std::vector<content::MenuItem>& items) {
  std::vector<content::MenuItem> local_items = items;
  if (local_items.size() >
      static_cast<size_t>(std::numeric_limits<int>::max() + 1)) {
    LOG(WARNING) << "Truncating menu - there are too many items!";
    local_items.resize(std::numeric_limits<int>::max() + 1);
  }

  for (size_t i = 0; i < local_items.size(); ++i) {
    local_items[i].action = static_cast<unsigned>(i);
  }

  return MenuItemBuilder::Build(local_items);
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

void WebPopupMenu::selectItems(const QList<unsigned>& selected_indices) {
  std::vector<int> x;
  for (unsigned i : selected_indices) {
    DCHECK_LE(i, static_cast<unsigned>(std::numeric_limits<int>::max()));
    x.push_back(static_cast<int>(i));
  }
  client_->SelectItems(x);
}

void WebPopupMenu::cancel() {
  client_->Cancel();
}

WebPopupMenu::WebPopupMenu(ContentsView* view,
                           const std::vector<content::MenuItem>& items,
                           bool allow_multiple_selection,
                           oxide::WebPopupMenuClient* client)
    : client_(client),
      contents_(view->GetWebContents()) {
  if (!view->client()) {
    return;
  }

  proxy_ =
      view->client()->CreateWebPopupMenu(
          BuildMenuItems(items), allow_multiple_selection, this);
}

WebPopupMenu::~WebPopupMenu() {}

} // namespace qt
} // namespace oxide

