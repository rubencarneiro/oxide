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

#ifndef _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_
#define _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_

#include "base/macros.h"

#include "qt/core/glue/oxide_qt_contents_view_proxy.h"
#include "shared/browser/oxide_web_contents_view_client.h"

namespace oxide {
namespace qt {

class ContentsViewProxyClient;

class ContentsView : public ContentsViewProxy,
                     public oxide::WebContentsViewClient {
 public:
  ContentsView(ContentsViewProxyClient* client);
  ~ContentsView() override;

  // TODO: Get rid
  ContentsViewProxyClient* client() const { return client_; }

 private:
  // ContentsViewProxy implementation

  // oxide::WebContentsViewClient implementation
  blink::WebScreenInfo GetScreenInfo() const override;
  gfx::Rect GetBoundsPix() const override;
  oxide::WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params) override;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) override;

  ContentsViewProxyClient* client_;

  DISALLOW_COPY_AND_ASSIGN(ContentsView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_
