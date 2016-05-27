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

#include "oxide_web_contents_view_client.h"

#include "base/logging.h"

#include "oxide_web_contents_view.h"
#include "oxide_web_popup_menu.h"

namespace oxide {

WebContentsViewClient::WebContentsViewClient()
    : view_(nullptr) {}

WebContentsViewClient::~WebContentsViewClient() {
  DCHECK(!view_);
}

void WebContentsViewClient::UpdateCursor(const content::WebCursor& cursor) {}

WebContextMenu* WebContentsViewClient::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  NOTIMPLEMENTED();
  return nullptr;
}

std::unique_ptr<WebPopupMenu> WebContentsViewClient::CreatePopupMenu(
    const std::vector<content::MenuItem> & items,
    int selected_item,
    bool allow_multiple_selection,
    WebPopupMenuClient* client) {
  NOTIMPLEMENTED();
  return nullptr;
}

ui::TouchHandleDrawable*
WebContentsViewClient::CreateTouchHandleDrawable() const {
  NOTIMPLEMENTED();
  return nullptr;
}

void WebContentsViewClient::TouchSelectionChanged(
    ui::TouchSelectionController::ActiveStatus status,
    const gfx::RectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) const {}

InputMethodContext* WebContentsViewClient::GetInputMethodContext() const {
  return nullptr;
}

void WebContentsViewClient::UnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {}

} // namespace oxide
