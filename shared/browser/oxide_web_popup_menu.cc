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

#include "oxide_web_popup_menu.h"

#include "base/logging.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "oxide_web_contents_view.h"

namespace oxide {

WebPopupMenu::WebPopupMenu(content::RenderViewHost* rvh) :
    content::WebContentsObserver(content::WebContents::FromRenderViewHost(rvh)),
    shown_(false),
    render_view_host_(rvh),
    weak_factory_(this) {}

WebPopupMenu::~WebPopupMenu() {}

void WebPopupMenu::RenderViewDeleted(content::RenderViewHost* rvh) {
  if (shown_) {
    Cancel();
  }
}

void WebPopupMenu::ShowPopup(const gfx::Rect& bounds,
                             const std::vector<content::MenuItem>& items,
                             int selected_item,
                             bool allow_multiple_selection) {
  shown_ = true;
  Show(bounds, items, selected_item, allow_multiple_selection);
}

void WebPopupMenu::HidePopup() {
  shown_ = false;
  Hide();
}

void WebPopupMenu::SelectItems(const std::vector<int>& selected_indices) {
  DCHECK(shown_);
  render_view_host()->DidSelectPopupMenuItems(selected_indices);
  HidePopup();
}

void WebPopupMenu::Cancel() {
  DCHECK(shown_);
  render_view_host()->DidCancelPopupMenu();
  HidePopup();
}

base::WeakPtr<WebPopupMenu> WebPopupMenu::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

content::RenderViewHostImpl* WebPopupMenu::render_view_host() const {
  return static_cast<content::RenderViewHostImpl *>(render_view_host_);
}

} // namespace oxide
