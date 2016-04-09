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

#ifndef _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_H_
#define _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_H_

#include <vector>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/menu_item.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
template <typename T> class DeleteHelper;
}

namespace content {
class RenderFrameHost;
class RenderFrameHostImpl;
}

namespace gfx {
class Rect;
}

namespace oxide {

class OXIDE_SHARED_EXPORT WebPopupMenu : public content::WebContentsObserver {
 public:
  virtual void Show(const gfx::Rect& bounds,
                    const std::vector<content::MenuItem>& items,
                    int selected_item,
                    bool allow_multiple_selection) = 0;
  void Close();

  void SelectItems(const std::vector<int>& selected_indices);
  void Cancel();

  base::WeakPtr<WebPopupMenu> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 protected:
  friend class base::DeleteHelper<WebPopupMenu>;

  WebPopupMenu(content::RenderFrameHost* rfh);
  virtual ~WebPopupMenu();

 private:
  void RenderFrameDeleted(content::RenderFrameHost* rfh) final;

  virtual void Hide();

  content::RenderFrameHostImpl* render_frame_host_;

  base::WeakPtrFactory<WebPopupMenu> weak_ptr_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebPopupMenu);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_H_
