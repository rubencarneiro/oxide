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

#include "oxide_message_handler.h"

#include "base/logging.h"
#include "content/public/browser/render_view_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_incoming_message.h"
#include "oxide_web_frame.h"

namespace oxide {

MessageHandler::MessageHandler() {}

bool MessageHandler::IsValid() const {
  return !msg_id_.empty() && world_ids_.size() > 0 && !callback_.is_null();
}

void MessageHandler::SetCallback(const HandlerCallback& callback) {
  callback_ = callback;
}

void MessageHandler::OnReceiveMessage(
    const MessageDispatcherBrowser::V8Message& message) {
  DCHECK_EQ(message.msg_id, msg_id());
  DCHECK(!callback_.is_null());

  bool delivered = false;
  bool error = false;
  std::string error_desc;

  callback_.Run(new IncomingMessage(message), &delivered, &error, error_desc);

  if (!delivered || error) {
    OxideMsg_SendMessage_Params params;
    params.frame_id = message.frame->identifier();
    params.world_id = message.world_id;
    params.serial = message.serial;
    params.type = OxideMsg_SendMessage_Type::Reply;
    params.error = error ?
        OxideMsg_SendMessage_Error::UNCAUGHT_EXCEPTION :
        OxideMsg_SendMessage_Error::UNDELIVERABLE;
    params.msg_id = message.msg_id;
    params.args = error_desc;

    message.dispatcher->GetRenderViewHost()->Send(
        new OxideMsg_SendMessage(
          message.dispatcher->GetRenderViewHost()->GetRoutingID(),
          params));
  }
}

} // namespace oxide
