// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_script_message_impl_browser.h"

#include "content/public/browser/render_frame_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"

namespace oxide {

ScriptMessageImplBrowser::~ScriptMessageImplBrowser() {}

void ScriptMessageImplBrowser::DoSendResponse(
    const ScriptMessageParams& params) {
  // Check that the frame hasn't gone away
  if (!source_frame()) {
    return;
  }

  content::RenderFrameHost* rfh = source_frame()->render_frame_host();
  rfh->Send(new OxideMsg_SendMessage(rfh->GetRoutingID(), params));
}

ScriptMessageImplBrowser::ScriptMessageImplBrowser(
    WebFrame* source_frame,
    int serial,
    const GURL& context,
    const std::string& msg_id,
    base::ListValue* wrapped_payload)
    : ScriptMessage(serial, context, msg_id, wrapped_payload),
      source_frame_(source_frame->GetWeakPtr()) {}

} // namespace oxide
