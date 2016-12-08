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

#include "web_popup_menu_impl.h"

#include "base/threading/thread_task_runner_handle.h"
#include "content/browser/frame_host/render_frame_host_impl.h" // nogncheck
#include "content/public/browser/web_contents.h"
#include "ui/gfx/geometry/vector2d.h"

#include "chrome_controller.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_contents_view_client.h"

namespace oxide {

void WebPopupMenuImpl::SelectItems(const std::vector<int>& selected_indices) {
  if (!render_frame_host_) {
    return;
  }

  render_frame_host_->DidSelectPopupMenuItems(selected_indices);
  Hide();
}

void WebPopupMenuImpl::Cancel() {
  if (!render_frame_host_) {
    return;
  }

  render_frame_host_->DidCancelPopupMenu();
  Hide();
}

void WebPopupMenuImpl::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  if (render_frame_host != render_frame_host_) {
    return;
  }

  Hide();
}

WebPopupMenuImpl::WebPopupMenuImpl(content::RenderFrameHost* render_frame_host,
                                   const std::vector<content::MenuItem>& items,
                                   int selected_item,
                                   bool allow_multiple_selection,
                                   const gfx::Rect& bounds)
    : content::WebContentsObserver(
          content::WebContents::FromRenderFrameHost(render_frame_host)),
      render_frame_host_(
          static_cast<content::RenderFrameHostImpl*>(render_frame_host)),
      items_(items),
      selected_item_(selected_item),
      allow_multiple_selection_(allow_multiple_selection),
      bounds_(bounds),
      weak_ptr_factory_(this) {}

WebPopupMenuImpl::~WebPopupMenuImpl() {
  DCHECK(!render_frame_host_);
}

base::WeakPtr<WebPopupMenuImpl> WebPopupMenuImpl::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

void WebPopupMenuImpl::Show() {
  WebContentsView* view = WebContentsView::FromWebContents(web_contents());
  if (!view->client()) {
    Cancel();
    return;
  }

  gfx::Vector2d top_content_offset;
  ChromeController* chrome_controller =
      ChromeController::FromWebContents(web_contents());
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

void WebPopupMenuImpl::Hide() {
  render_frame_host_ = nullptr;
  weak_ptr_factory_.InvalidateWeakPtrs();

  if (menu_) {
    menu_->Hide();
  }

  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE, this);
}

} // namespace oxide
