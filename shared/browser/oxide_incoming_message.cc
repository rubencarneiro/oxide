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

#include "oxide_incoming_message.h"

#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"
#include "oxide_web_view.h"

namespace oxide {

IncomingMessage::IncomingMessage(
    WebFrame* source_frame,
    int serial,
    const std::string& world_id,
    const std::string& msg_id,
    const std::string& args) :
    source_frame_(source_frame->GetWeakPtr()),
    serial_(serial),
    world_id_(world_id),
    msg_id_(msg_id),
    args_(args) {}

void IncomingMessage::Reply(const std::string& args) {
  // Check that the frame hasn't gone away
  if (!source_frame_ || serial_ == -1) {
    return;
  }

  OxideMsg_SendMessage_Params params;
  params.frame_id = source_frame()->identifier();
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = OxideMsg_SendMessage_Error::OK;
  params.args = args;

  // FIXME: This is clearly broken for OOPIF
  content::WebContents* web_contents = source_frame()->view()->web_contents();
  web_contents->Send(new OxideMsg_SendMessage(
      web_contents->GetRenderViewHost()->GetRoutingID(),
      params));
}

void IncomingMessage::Error(const std::string& msg) {
  // Check that the frame hasn't gone away
  if (!source_frame_ || serial_ == -1) {
    return;
  }

  OxideMsg_SendMessage_Params params;
  params.frame_id = source_frame()->identifier();
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = OxideMsg_SendMessage_Error::HANDLER_REPORTED_ERROR;
  params.args = msg;

  // FIXME: This is clearly broken for OOPIF
  content::WebContents* web_contents = source_frame()->view()->web_contents();
  web_contents->Send(new OxideMsg_SendMessage(
      web_contents->GetRenderViewHost()->GetRoutingID(),
      params));
}

} // namespace oxide
