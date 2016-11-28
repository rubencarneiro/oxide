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

#include "uitk_auxiliary_ui_factory.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>

#include "qt/core/glue/screen_utils.h"

#include "uitk_web_context_menu.h"

static void InitResources() {
  Q_INIT_RESOURCE(resources);
}

namespace oxide {
namespace uitk {

using qt::GetScreenFormFactor;
using qt::MenuItem;
using qt::ScreenFormFactor;
using qt::WebContextMenuClient;
using qt::WebContextMenuParams;

namespace {

bool IsItemOnMobileScreen(QQuickItem* item) {
  QQuickWindow* window = item->window();
  if (!window) {
    return false;
  }

  QScreen* screen = window->screen();
  if (!screen) {
    return false;
  }

  return GetScreenFormFactor(screen) == ScreenFormFactor::Mobile;
}

}

std::unique_ptr<qt::WebContextMenu> AuxiliaryUIFactory::CreateWebContextMenu(
    const WebContextMenuParams& params,
    const std::vector<MenuItem>& items,
    WebContextMenuClient* client) {
  // TODO(chrisccoulson): Allow the application to customize the menu here

  return WebContextMenu::Create(item(), params, items, client,
                                IsItemOnMobileScreen(item()));
}

AuxiliaryUIFactory::AuxiliaryUIFactory() {
  InitResources();
}

AuxiliaryUIFactory::~AuxiliaryUIFactory() = default;

} // namespace uitk
} // namespace oxide
