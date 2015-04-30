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
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"

#include "oxide_web_contents_view.h"

namespace oxide {

void WebPopupMenu::RenderFrameDeleted(content::RenderFrameHost* rfh) {
  if (rfh != render_frame_host_) {
    return;
  }

  Close();
}

void WebPopupMenu::Hide() {}

WebPopupMenu::WebPopupMenu(content::RenderFrameHost* rfh)
    : content::WebContentsObserver(content::WebContents::FromRenderFrameHost(rfh)),
      render_frame_host_(static_cast<content::RenderFrameHostImpl *>(rfh)),
      weak_ptr_factory_(this) {}

WebPopupMenu::~WebPopupMenu() {
  DCHECK(!render_frame_host_);
}

void WebPopupMenu::Close() {
  render_frame_host_ = nullptr;
  weak_ptr_factory_.InvalidateWeakPtrs();
  Hide();
  content::BrowserThread::DeleteSoon(
      content::BrowserThread::UI, FROM_HERE, this);
}

void WebPopupMenu::SelectItems(const std::vector<int>& selected_indices) {
  if (!render_frame_host_) {
    return;
  }

  render_frame_host_->DidSelectPopupMenuItems(selected_indices);
  Close();
}

void WebPopupMenu::Cancel() {
  if (!render_frame_host_) {
    return;
  }

  render_frame_host_->DidCancelPopupMenu();
  Close();
}

} // namespace oxide
