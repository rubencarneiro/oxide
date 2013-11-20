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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/menu_item.h"

namespace content {
class RenderViewHostImpl;
}

namespace gfx {
class Rect;
}

namespace oxide {

class WebPopupMenu : public content::WebContentsObserver {
 public:
  virtual ~WebPopupMenu();

  void RenderViewDeleted(content::RenderViewHost* rvh) FINAL;

  void ShowPopup(const gfx::Rect& bounds,
                 const std::vector<content::MenuItem>& items,
                 int selected_item,
                 bool allow_multiple_selection);
  void HidePopup();

  void SelectItems(const std::vector<int>& selected_indices);
  void Cancel();

  base::WeakPtr<WebPopupMenu> GetWeakPtr();

  content::RenderViewHostImpl* render_view_host() const;

 protected:
  WebPopupMenu(content::RenderViewHost* rvh);

 private:
  virtual void Show(const gfx::Rect& bounds,
                    const std::vector<content::MenuItem>& items,
                    int selected_item,
                    bool allow_multiple_selection) = 0;
  virtual void Hide() = 0;

  bool shown_;
  content::RenderViewHost* render_view_host_;
  base::WeakPtrFactory<WebPopupMenu> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebPopupMenu);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_POPUP_MENU_H_
