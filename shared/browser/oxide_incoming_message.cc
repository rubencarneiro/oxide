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

#include "content/browser/frame_host/frame_tree_node.h"
#include "content/public/browser/render_frame_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"

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
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = OxideMsg_SendMessage_Error::OK;
  params.payload = args;

  content::RenderFrameHost* rfh =
      source_frame()->frame_tree_node()->current_frame_host();
  rfh->Send(new OxideMsg_SendMessage(rfh->GetRoutingID(), params));
}

void IncomingMessage::Error(const std::string& msg) {
  // Check that the frame hasn't gone away
  if (!source_frame_ || serial_ == -1) {
    return;
  }

  OxideMsg_SendMessage_Params params;
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = OxideMsg_SendMessage_Error::HANDLER_REPORTED_ERROR;
  params.payload = msg;

  content::RenderFrameHost* rfh =
      source_frame()->frame_tree_node()->current_frame_host();
  rfh->Send(new OxideMsg_SendMessage(rfh->GetRoutingID(), params));
}

} // namespace oxide
