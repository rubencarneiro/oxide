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

#include "web_popup_menu_host.h"

#include <QList>

#include "base/logging.h"

#include "qt/core/glue/web_popup_menu.h"
#include "shared/browser/web_popup_menu_client.h"

#include "menu_item_builder.h"

namespace oxide {
namespace qt {

void WebPopupMenuHost::Show() {
  menu_->Show();
}

void WebPopupMenuHost::Hide() {
  menu_->Hide();
}

void WebPopupMenuHost::selectItems(const QList<unsigned>& selected_indices) {
  std::vector<int> x;
  for (unsigned i : selected_indices) {
    DCHECK_LE(i, static_cast<unsigned>(std::numeric_limits<int>::max()));
    x.push_back(static_cast<int>(i));
  }
  client_->SelectItems(x);
}

void WebPopupMenuHost::cancel() {
  client_->Cancel();
}

WebPopupMenuHost::WebPopupMenuHost(oxide::WebPopupMenuClient* client)
    : client_(client) {}

WebPopupMenuHost::~WebPopupMenuHost() = default;

void WebPopupMenuHost::Init(std::unique_ptr<qt::WebPopupMenu> menu) {
  menu_ = std::move(menu);
}

// static
std::vector<MenuItem> WebPopupMenuHost::BuildMenuItems(
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

} // namespace qt
} // namespace oxide

