// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include "qt/core/glue/oxide_qt_web_context_menu_proxy_client.h"
#include "shared/browser/oxide_web_context_menu.h"

namespace oxide {
namespace qt {

class WebContextMenuProxy;

class WebContextMenu : public oxide::WebContextMenu,
                       public WebContextMenuProxyClient {
 public:
  WebContextMenu(content::RenderFrameHost* rfh,
                 const content::ContextMenuParams& params);

  void SetProxy(WebContextMenuProxy* proxy);

 private:
  ~WebContextMenu() override;

  // oxide::WebContextMenu implementation
  void Show() override;
  void Hide() override;

  // WebContextMenuProxyClient implementation
  MediaType mediaType() const override;
  QPoint position() const override;
  QUrl linkUrl() const override;
  QString linkText() const override;
  QUrl unfilteredLinkUrl() const override;
  QUrl srcUrl() const override;
  bool hasImageContents() const override;
  QUrl pageUrl() const override;
  QUrl frameUrl() const override;
  QString selectionText() const override;
  bool isEditable() const override;
  void cancel() override;
  int editFlags() const override;
  int mediaFlags() const override;
  void copyImage() const override;
  void saveLink() const override;
  void saveMedia() const override;

  std::unique_ptr<WebContextMenuProxy> proxy_;

  DISALLOW_COPY_AND_ASSIGN(WebContextMenu);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_CONTEXT_MENU_H_
