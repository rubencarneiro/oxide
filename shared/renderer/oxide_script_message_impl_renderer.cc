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

#include "oxide_script_message_impl_renderer.h"

#include "content/public/renderer/render_frame.h"

#include "shared/common/oxide_messages.h"

#include "oxide_script_message_manager.h"

namespace oxide {

ScriptMessageImplRenderer::~ScriptMessageImplRenderer() {}

void ScriptMessageImplRenderer::DoSendResponse(
    const OxideMsg_SendMessage_Params& params) {
  if (!manager()) {
    return;
  }

  content::RenderFrame* frame = manager()->frame();
  frame->Send(new OxideHostMsg_SendMessage(frame->GetRoutingID(), params));
}

ScriptMessageImplRenderer::ScriptMessageImplRenderer(
    ScriptMessageManager* mm,
    int serial,
    bool want_reply,
    const std::string& msg_id,
    const std::string& args,
    const v8::Handle<v8::Object>& handle) :
    ScriptMessage(serial, mm->GetContextURL(), want_reply, msg_id, args),
    ScriptReferencedObject<ScriptMessageImplRenderer>(mm, handle) {}

} // namespace oxide
