// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "render_widget_host_view_oxide.h"

#include "base/logging.h"
#include "content/common/view_messages.h"
#include "ipc/ipc_message_macros.h"

namespace content {

namespace {
DefaultScreenInfoGetter* g_default_screen_info_getter;
}

void RenderWidgetHostViewBase::GetDefaultScreenInfo(
    blink::WebScreenInfo* results) {
  DCHECK(g_default_screen_info_getter);
  *results = g_default_screen_info_getter();
}

void RenderWidgetHostViewOxide::OnTextInputStateChangedThunk(
    const ViewHostMsg_TextInputState_Params& params) {
  OnTextInputStateChanged(params.type, params.show_ime_if_needed);
}

void RenderWidgetHostViewOxide::SelectionBoundsChanged(
    const ViewHostMsg_SelectionBounds_Params& params) {
  OnSelectionBoundsChanged(params.anchor_rect,
                           params.focus_rect,
                           params.is_anchor_first);
}

bool RenderWidgetHostViewOxide::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RenderWidgetHostViewOxide, msg)
    IPC_MESSAGE_HANDLER(ViewHostMsg_TextInputStateChanged,
                        OnTextInputStateChangedThunk)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

RenderWidgetHostViewOxide::~RenderWidgetHostViewOxide() {}

void SetDefaultScreenInfoGetterOxide(DefaultScreenInfoGetter* getter) {
  g_default_screen_info_getter = getter;
}

} // namespace content
