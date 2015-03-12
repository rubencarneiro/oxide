// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_top_controls_handler.h"

#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "shared/common/oxide_messages.h"

namespace oxide {

void TopControlsHandler::OnUpdateTopControlsState(
    blink::WebTopControlsState constraints,
    blink::WebTopControlsState current,
    bool animate) {
  blink::WebView* view = render_view()->GetWebView();
  if (!view) {
    return;
  }

  view->updateTopControlsState(constraints, current, animate);
}

bool TopControlsHandler::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(TopControlsHandler, message)
    IPC_MESSAGE_HANDLER(OxideMsg_UpdateTopControlsState, OnUpdateTopControlsState)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

TopControlsHandler::TopControlsHandler(content::RenderView* render_view)
    : content::RenderViewObserver(render_view) {}

TopControlsHandler::~TopControlsHandler() {}

} // namespace oxide
