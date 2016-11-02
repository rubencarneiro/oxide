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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_H_

#include <memory>

#include "base/macros.h"
#include "content/public/common/context_menu_params.h"

#include "qt/core/glue/web_context_menu_client.h"
#include "qt/core/glue/web_context_menu_params.h"
#include "shared/browser/web_context_menu.h"

namespace oxide {

class WebContextMenuClient;

namespace qt {

class WebContextMenu;

class WebContextMenuImpl : public oxide::WebContextMenu,
                           public WebContextMenuClient {
 public:
  WebContextMenuImpl(const content::ContextMenuParams& params,
                     oxide::WebContextMenuClient* client);
  ~WebContextMenuImpl() override;

  void Init(std::unique_ptr<qt::WebContextMenu> menu);

  WebContextMenuParams GetParams() const;

 private:
  // oxide::WebContextMenu implementation
  void Show() override;
  void Hide() override;

  // WebContextMenuClient implementation
  void close() override;
  void copyImage() const override;
  void saveLink() const override;
  void saveMedia() const override;

  content::ContextMenuParams params_;

  oxide::WebContextMenuClient* client_;

  std::unique_ptr<qt::WebContextMenu> menu_;

  DISALLOW_COPY_AND_ASSIGN(WebContextMenuImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_H_
