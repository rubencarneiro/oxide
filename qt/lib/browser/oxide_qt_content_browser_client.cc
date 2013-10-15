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

#include "oxide_qt_content_browser_client.h"

#include <QGuiApplication>

#include "shared/browser/oxide_web_view.h"

#include "qt/lib/api/private/oxide_q_web_view_base_p.h"

#include "oxide_qt_message_pump.h"
#include "oxide_qt_render_widget_host_view_qquick.h"
#include "oxide_qt_web_frame_tree.h"

namespace oxide {
namespace qt {

base::MessagePump* ContentBrowserClient::CreateMessagePumpForUI() {
  return new MessagePump();
}

oxide::WebFrameTree* ContentBrowserClient::CreateWebFrameTree(
    content::RenderViewHost* rvh) {
  QWebViewBasePrivate* wvp = static_cast<QWebViewBasePrivate *>(
      oxide::WebView::FromRenderViewHost(rvh));
  if (!wvp) {
    // The first RVH created for a WebContents is before we set the delegate
    return NULL;
  }

  return wvp->CreateWebFrameTree(rvh);
}

void ContentBrowserClient::GetDefaultScreenInfoImpl(
    WebKit::WebScreenInfo* result) {
  RenderWidgetHostViewQQuick::GetScreenInfo(
      QGuiApplication::primaryScreen(),
      result);
}

} // namespace qt
} // namespace oxide
