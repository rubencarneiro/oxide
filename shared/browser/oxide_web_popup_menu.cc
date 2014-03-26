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

void WebPopupMenu::RenderViewDeleted(content::RenderViewHost* rvh) {
  if (rvh != render_view_host_) {
    return;
  }
  render_view_host_ = NULL;
  Hide();
}

WebPopupMenu::WebPopupMenu(content::RenderViewHost* rvh) :
    content::WebContentsObserver(content::WebContents::FromRenderViewHost(rvh)),
    popup_was_hidden_(false),
    render_view_host_(static_cast<content::RenderViewHostImpl *>(rvh)) {}

WebPopupMenu::~WebPopupMenu() {
  if (!popup_was_hidden_) {
    render_view_host_->DidCancelPopupMenu();
  }
}

void WebPopupMenu::Hide() {
  if (popup_was_hidden_) {
    return;
  }
  popup_was_hidden_ = true;
  OnHide();
}

void WebPopupMenu::SelectItems(const std::vector<int>& selected_indices) {
  DCHECK(!popup_was_hidden_);
  render_view_host_->DidSelectPopupMenuItems(selected_indices);
  Hide();
}

void WebPopupMenu::Cancel() {
  DCHECK(!popup_was_hidden_);
  render_view_host_->DidCancelPopupMenu();
  Hide();
}

bool WebPopupMenu::WasHidden() const {
  return popup_was_hidden_;
}

} // namespace oxide
