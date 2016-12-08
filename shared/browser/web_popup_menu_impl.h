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

#ifndef _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_IMPL_H_
#define _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_IMPL_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/menu_item.h"
#include "ui/gfx/geometry/rect.h"

#include "shared/browser/web_popup_menu.h"
#include "shared/browser/web_popup_menu_client.h"

namespace content {
class RenderFrameHost;
class RenderFrameHostImpl;
}

namespace oxide {

class WebPopupMenuImpl : public WebPopupMenu,
                         public WebPopupMenuClient,
                         public content::WebContentsObserver {
 public:
  WebPopupMenuImpl(content::RenderFrameHost* render_frame_host,
                   const std::vector<content::MenuItem>& items,
                   int selected_item,
                   bool allow_multiple_selection,
                   const gfx::Rect& bounds);
  ~WebPopupMenuImpl() override;

  base::WeakPtr<WebPopupMenuImpl> GetWeakPtr();

  // WebPopupMenu implementation
  void Show() override;
  void Hide() override;

 private:
  // WebPopupMenuClient implementation
  void SelectItems(const std::vector<int>& selected_indices) override;
  void Cancel() override;

  // content::WebContentsObserver implementation
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;

  content::RenderFrameHostImpl* render_frame_host_;

  std::vector<content::MenuItem> items_;

  int selected_item_;

  bool allow_multiple_selection_;

  gfx::Rect bounds_;

  std::unique_ptr<WebPopupMenu> menu_;

  base::WeakPtrFactory<WebPopupMenuImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebPopupMenuImpl);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_IMPL_H_
