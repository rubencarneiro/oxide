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

#include "menu_item_builder.h"

#include <QString>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace oxide {
namespace qt {

// static
std::vector<MenuItem> MenuItemBuilder::Build(
    const std::vector<content::MenuItem>& items) {
  std::vector<MenuItem> rv;

  QString current_group;

  for (const auto& item : items) {
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
    mi.separator = item.type == content::MenuItem::SEPARATOR;
    mi.action = item.action;
    mi.enabled = item.enabled;
    mi.checked = item.checked;

    rv.push_back(mi);
  }

  return std::move(rv);
}

} // namespace qt
} // namespace oxide
