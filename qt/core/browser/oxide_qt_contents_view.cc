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

#include "oxide_qt_contents_view.h"

#include <QGuiApplication>

#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"
#include "shared/browser/oxide_web_contents_view.h"

#include "oxide_qt_screen_utils.h"
#include "oxide_qt_web_context_menu.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

blink::WebScreenInfo ContentsView::GetScreenInfo() const {
  QScreen* screen = client_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetWebScreenInfoFromQScreen(screen);
}

gfx::Rect ContentsView::GetBoundsPix() const {
  QRect bounds = client_->GetBoundsPix();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

oxide::WebContextMenu* ContentsView::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  WebContextMenu* menu = new WebContextMenu(rfh, params);
  menu->SetProxy(client_->CreateWebContextMenu(menu));
  return menu;
}

oxide::WebPopupMenu* ContentsView::CreatePopupMenu(
    content::RenderFrameHost* rfh) {
  WebPopupMenu* menu = new WebPopupMenu(rfh);
  menu->SetProxy(client_->CreateWebPopupMenu(menu));
  return menu;
}

ContentsView::ContentsView(ContentsViewProxyClient* client,
                           QObject* native_view)
    : client_(client),
      native_view_(native_view) {
  DCHECK(!client_->view_);
  client_->view_ = this;
}

ContentsView::~ContentsView() {
  DCHECK_EQ(client_->view_, this);
  client_->view_ = nullptr;
}

// static
ContentsView* ContentsView::FromWebContents(content::WebContents* contents) {
  oxide::WebContentsView* view =
      oxide::WebContentsView::FromWebContents(contents);
  if (!view) {
    return nullptr;
  }

  return static_cast<ContentsView*>(view->client());
}

} // namespace qt
} // namespace oxide
