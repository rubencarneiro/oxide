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

#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_impl.h" // nogncheck
#include "content/public/browser/web_contents.h"
#include "ui/gfx/geometry/vector2d.h"

#include "chrome_controller.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_contents_view_client.h"
#include "web_popup_menu.h"

namespace oxide {

void WebPopupMenuHost::Hide() {
  DCHECK(!hidden_callback_.is_null());
  base::Closure hidden_callback = std::move(hidden_callback_);

  if (menu_) {
    menu_->Hide();
  }

  hidden_callback.Run();
}

void WebPopupMenuHost::SelectItems(const std::vector<int>& selected_indices) {
  if (hidden_callback_.is_null()) {
    return;
  }

  content::RenderFrameHostImpl* render_frame_host =
      static_cast<content::RenderFrameHostImpl*>(
          render_frame_host_id_.ToInstance());
  DCHECK(render_frame_host);

  render_frame_host->DidSelectPopupMenuItems(selected_indices);
  Hide();
}

void WebPopupMenuHost::Cancel() {
  if (hidden_callback_.is_null()) {
    return;
  }

  content::RenderFrameHostImpl* render_frame_host =
      static_cast<content::RenderFrameHostImpl*>(
          render_frame_host_id_.ToInstance());
  DCHECK(render_frame_host);

  render_frame_host->DidCancelPopupMenu();
  Hide();
}

WebPopupMenuHost::WebPopupMenuHost(content::RenderFrameHost* render_frame_host,
                                   const std::vector<content::MenuItem>& items,
                                   int selected_item,
                                   bool allow_multiple_selection,
                                   const gfx::Rect& bounds,
                                   const base::Closure& hidden_callback)
    : render_frame_host_id_(render_frame_host),
      items_(items),
      selected_item_(selected_item),
      allow_multiple_selection_(allow_multiple_selection),
      bounds_(bounds),
      hidden_callback_(hidden_callback) {}

WebPopupMenuHost::~WebPopupMenuHost() {
  hidden_callback_.Reset();
}

void WebPopupMenuHost::Show() {
  content::RenderFrameHost* render_frame_host =
      render_frame_host_id_.ToInstance();
  DCHECK(render_frame_host);

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  DCHECK(web_contents);

  WebContentsView* view = WebContentsView::FromWebContents(web_contents);
  if (!view->client()) {
    Cancel();
    return;
  }

  gfx::Vector2d top_content_offset;
  ChromeController* chrome_controller =
      ChromeController::FromWebContents(web_contents);
  if (chrome_controller) {
    top_content_offset =
        gfx::Vector2d(0, chrome_controller->GetTopContentOffset());
  }

  menu_ = view->client()->CreatePopupMenu(items_,
                                          selected_item_,
                                          allow_multiple_selection_,
                                          bounds_ + top_content_offset,
                                          this);
  if (!menu_) {
    Cancel();
    return;
  }

  menu_->Show();
}

content::RenderFrameHost* WebPopupMenuHost::GetRenderFrameHost() const {
  return render_frame_host_id_.ToInstance();
}

} // namespace oxide
