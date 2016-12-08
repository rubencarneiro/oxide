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
#include "content/public/common/menu_item.h"

#include "qt/core/glue/menu_item.h"
#include "qt/core/glue/web_popup_menu_client.h"
#include "shared/browser/web_popup_menu.h"

namespace oxide {

class WebPopupMenuClient;

namespace qt {

class WebPopupMenu;

class WebPopupMenuImpl : public oxide::WebPopupMenu,
                         public WebPopupMenuClient {
 public:
  WebPopupMenuImpl(oxide::WebPopupMenuClient* client);
  ~WebPopupMenuImpl() override;

  bool Init(std::unique_ptr<qt::WebPopupMenu> menu);

  static std::vector<MenuItem> BuildMenuItems(
      const std::vector<content::MenuItem>& items);

 private:
  // oxide::WebPopupMenu implementation
  void Show() override;
  void Hide() override;

  // WebPopupMenuClient implementation
  void selectItems(const QList<unsigned>& selected_indices) override;
  void cancel() override;

  oxide::WebPopupMenuClient* client_; // Owns us

  std::unique_ptr<qt::WebPopupMenu> menu_;

  DISALLOW_COPY_AND_ASSIGN(WebPopupMenuImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_POPUP_MENU_QQUICK_H_
