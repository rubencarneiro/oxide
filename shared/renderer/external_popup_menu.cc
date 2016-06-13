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

#include "external_popup_menu.h"

#include "content/common/view_messages.h" // For content::MenuItem ParamTraits
#include "content/public/common/menu_item.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/WebKit/public/platform/WebRect.h"
#include "third_party/WebKit/public/web/WebExternalPopupMenuClient.h"

#include "shared/common/oxide_messages.h"

namespace oxide {

namespace {

content::MenuItem BuildMenuItem(const blink::WebMenuItemInfo& info) {
  content::MenuItem item;

  item.label = info.label;
  item.icon = info.icon;
  item.tool_tip = info.toolTip;
  item.type = static_cast<content::MenuItem::Type>(info.type);
  item.action = info.action;
  item.rtl = info.textDirection == blink::WebTextDirectionRightToLeft;
  item.has_directional_override = info.hasTextDirectionOverride;
  item.enabled = info.enabled;
  item.checked = info.checked;

  for (const auto& i : info.subMenuItems) {
    item.submenu.push_back(BuildMenuItem(i));
  }

  return item;
}

}

void ExternalPopupMenu::OnDidSelectPopupMenuItems(
    const std::vector<int>& indices) {
  client_->didAcceptIndices(indices);
  delete this;
}

void ExternalPopupMenu::OnDidCancelPopupMenu() {
  client_->didCancel();
  delete this;
}

void ExternalPopupMenu::show(const blink::WebRect& bounds) {
  OxideHostMsg_ShowPopup_Params params;

  blink::WebRect rect = bounds;
  if (origin_scale_for_emulation_) {
    rect.x *= origin_scale_for_emulation_;
    rect.y *= origin_scale_for_emulation_;
  }
  rect.x += origin_offset_for_emulation_.x();
  rect.y += origin_offset_for_emulation_.y();

  params.bounds = rect;

  params.selected_item = popup_menu_info_.selectedIndex;

  for (const auto& item : popup_menu_info_.items) {
    params.popup_items.push_back(BuildMenuItem(item));
  }

  params.allow_multiple_selection = popup_menu_info_.allowMultipleSelection;

  render_frame()->Send(
      new OxideHostMsg_ShowPopup(render_frame()->GetRoutingID(), params));
}

void ExternalPopupMenu::close() {
  render_frame()->Send(
      new OxideHostMsg_HidePopup(render_frame()->GetRoutingID()));
  delete this;
}

void ExternalPopupMenu::OnDestruct() {
  delete this;
}

bool ExternalPopupMenu::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ExternalPopupMenu, message)
    IPC_MESSAGE_HANDLER(OxideMsg_DidSelectPopupMenuItems,
                        OnDidSelectPopupMenuItems)
    IPC_MESSAGE_HANDLER(OxideMsg_DidCancelPopupMenu,
                        OnDidCancelPopupMenu)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

ExternalPopupMenu::ExternalPopupMenu(
    content::RenderFrame* render_frame,
    const blink::WebPopupMenuInfo& popup_menu_info,
    blink::WebExternalPopupMenuClient* popup_menu_client,
    float origin_scale_for_emulation,
    const gfx::PointF& origin_offset_for_emulation)
    : content::RenderFrameObserver(render_frame),
      content::RenderFrameObserverTracker<ExternalPopupMenu>(render_frame),
      popup_menu_info_(popup_menu_info),
      client_(popup_menu_client),
      origin_scale_for_emulation_(origin_scale_for_emulation),
      origin_offset_for_emulation_(origin_offset_for_emulation) {}

ExternalPopupMenu::~ExternalPopupMenu() {}

} // namespace oxide
