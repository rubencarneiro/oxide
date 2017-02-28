// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_HOST_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_HOST_H_

#include <memory>

#include "base/macros.h"

#include "qt/core/glue/web_context_menu_client.h"
#include "shared/browser/context_menu/web_context_menu.h"

namespace oxide {

class WebContextMenuClient;

namespace qt {

class WebContextMenu;

class WebContextMenuHost : public oxide::WebContextMenu,
                           public WebContextMenuClient {
 public:
  WebContextMenuHost(oxide::WebContextMenuClient* client);
  ~WebContextMenuHost() override;

  void Init(std::unique_ptr<qt::WebContextMenu> menu);

 private:
  // oxide::WebContextMenu implementation
  void Show() override;
  void Hide() override;

  // WebContextMenuClient implementation
  void close() override;
  void execCommand(WebContextMenuAction action, bool close) override;

  oxide::WebContextMenuClient* client_;

  std::unique_ptr<qt::WebContextMenu> menu_;

  DISALLOW_COPY_AND_ASSIGN(WebContextMenuHost);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_HOST_H_
