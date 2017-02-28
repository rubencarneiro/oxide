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

#include "touch_editing_menu_host.h"

#include "qt/core/browser/contents_view_impl.h"
#include "qt/core/browser/oxide_qt_dpi_utils.h"
#include "qt/core/browser/oxide_qt_type_conversions.h"
#include "qt/core/glue/macros.h"
#include "qt/core/glue/touch_editing_menu.h"
#include "qt/core/glue/web_context_menu_actions.h"
#include "shared/browser/context_menu/web_context_menu_actions.h"
#include "shared/browser/touch_selection/touch_editing_menu_client.h"

namespace oxide {
namespace qt {

void TouchEditingMenuHost::ExecuteCommand(WebContextMenuAction action) {
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Cut,
                              oxide::WebContextMenuAction::Cut)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Copy,
                              oxide::WebContextMenuAction::Copy)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Paste,
                              oxide::WebContextMenuAction::Paste)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::SelectAll,
                              oxide::WebContextMenuAction::SelectAll)

  client_->ExecuteCommand(static_cast<oxide::WebContextMenuAction>(action));
}

void TouchEditingMenuHost::Close() {
  client_->Close();
}

void TouchEditingMenuHost::WasResized() {
  client_->WasResized();
}

void TouchEditingMenuHost::Show() {
  menu_->Show();
}

void TouchEditingMenuHost::Hide() {
  menu_->Hide();
}

gfx::Size TouchEditingMenuHost::GetSizeIncludingMargin() const {
  return DpiUtils::ConvertQtPixelsToChromium(
      ToChromium(menu_->GetSizeIncludingMargin()),
      view_->GetScreen());
}

void TouchEditingMenuHost::SetOrigin(const gfx::PointF& origin) {
  menu_->SetOrigin(
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(origin, view_->GetScreen())));
}

TouchEditingMenuHost::TouchEditingMenuHost(
    const ContentsViewImpl* view,
    oxide::TouchEditingMenuClient* client)
    : view_(view),
      client_(client) {}

TouchEditingMenuHost::~TouchEditingMenuHost() = default;

void TouchEditingMenuHost::Init(std::unique_ptr<qt::TouchEditingMenu> menu) {
  menu_ = std::move(menu);
}

} // namespace qt
} // namespace oxide
