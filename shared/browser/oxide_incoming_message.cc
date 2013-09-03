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

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"

namespace oxide {

IncomingMessage::IncomingMessage(
    const MessageDispatcherBrowser::V8Message& message) :
    frame_(message.frame->GetWeakPtr()),
    world_id_(message.world_id),
    serial_(message.serial),
    args_(message.args),
    source_(message.dispatcher->GetWeakPtr()) {}

void IncomingMessage::Reply(const std::string& args) {
  if (!source_ || !frame_ || serial_ == -1) {
    return;
  }

  OxideMsg_SendMessage_Params params;
  params.frame_id = frame_->identifier();
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.args = args;

  source_->GetRenderViewHost()->Send(
      new OxideMsg_SendMessage(source_->GetRenderViewHost()->GetRoutingID(),
                               params));
}

void IncomingMessage::Error(const std::string& msg) {
  if (!source_ || !frame_ || serial_ == -1) {
    return;
  }

  OxideMsg_SendMessage_Params params;
  params.frame_id = frame_->identifier();
  params.world_id = world_id_;
  params.serial = serial_;
  params.type = OxideMsg_SendMessage_Type::Error;
  params.args = msg;

  source_->GetRenderViewHost()->Send(
      new OxideMsg_SendMessage(source_->GetRenderViewHost()->GetRoutingID(),
                               params));
}

} // namespace oxide
