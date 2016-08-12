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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_H_

#include <memory>
#include <vector>

#include "base/macros.h"

#include "qt/core/glue/oxide_qt_web_popup_menu_proxy_client.h"
#include "shared/browser/oxide_web_popup_menu.h"

namespace content {
struct MenuItem;
class WebContents;
}

namespace oxide {

class WebPopupMenuClient;

namespace qt {

class ContentsView;
class WebPopupMenuProxy;

class WebPopupMenu : public oxide::WebPopupMenu,
                     public WebPopupMenuProxyClient {
 public:
  WebPopupMenu(ContentsView* view,
               const std::vector<content::MenuItem>& items,
               int selected_index,
               bool allow_multiple_selection,
               oxide::WebPopupMenuClient* client);
  ~WebPopupMenu() override;

 private:
  // oxide::WebPopupMenu implementation
  void Show(const gfx::Rect& bounds) override;
  void Hide() override;

  // WebPopupMenuProxyClient implementation
  void selectItems(const QList<int>& selected_indices) override;
  void cancel() override;

  oxide::WebPopupMenuClient* client_; // Owns us

  content::WebContents* contents_;

  std::unique_ptr<WebPopupMenuProxy> proxy_;

  DISALLOW_COPY_AND_ASSIGN(WebPopupMenu);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_QQUICK_H_
