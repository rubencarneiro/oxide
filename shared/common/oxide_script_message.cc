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

#include "oxide_script_message.h"

#include "base/logging.h"

#include "oxide_messages.h"
#include "oxide_script_message.h"

namespace oxide {

// static
void ScriptMessageTraits::Destruct(const ScriptMessage* x) {
  ScriptMessage* self = const_cast<ScriptMessage *>(x);
  if (!x->has_responded_) {
    self->Error(ScriptMessageRequest::ERROR_HANDLER_DID_NOT_RESPOND,
                "The handler failed to send a response");
  }
  delete x;
}

void ScriptMessage::MakeParams(OxideMsg_SendMessage_Params* params) {
  params->context = context().spec();
  params->serial = serial();
  params->type = OxideMsg_SendMessage_Type::Reply;
  params->msg_id = msg_id();
}

ScriptMessage::ScriptMessage(int serial,
                             const GURL& context,
                             bool want_reply,
                             const std::string& msg_id,
                             const std::string& args) :
    serial_(serial),
    context_(context),
    msg_id_(msg_id),
    args_(args),
    has_responded_(!want_reply) {}

ScriptMessage::~ScriptMessage() {
  DCHECK(has_responded_);
}

void ScriptMessage::Reply(const std::string& args) {
  if (has_responded_) {
    return;
  }

  has_responded_ = true;

  OxideMsg_SendMessage_Params params;
  MakeParams(&params);
  params.error = ScriptMessageRequest::ERROR_OK;
  params.payload = args;

  DoSendResponse(params);
}

void ScriptMessage::Error(ScriptMessageRequest::Error code,
                          const std::string& msg) {
  DCHECK(code != ScriptMessageRequest::ERROR_OK) <<
      "Use Reply() for non-error responses";
  if (has_responded_) {
    return;
  }

  has_responded_ = true;

  OxideMsg_SendMessage_Params params;
  MakeParams(&params);
  params.error = code;
  params.payload = msg;

  DoSendResponse(params);
}

} // namespace oxide
