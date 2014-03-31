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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_web_popup_menu.h"

namespace oxide {
namespace qt {

class WebPopupMenuDelegate;

class WebPopupMenu FINAL : public oxide::WebPopupMenu {
 public:
  WebPopupMenu(WebPopupMenuDelegate* delegate,
               content::RenderViewHost* rvh);

 private:
  void Show(const gfx::Rect& bounds,
            const std::vector<content::MenuItem>& items,
            int selected_item,
            bool allow_multiple_selection) FINAL;
  void OnHide() FINAL;

  scoped_ptr<WebPopupMenuDelegate> delegate_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebPopupMenu);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_QQUICK_H_
