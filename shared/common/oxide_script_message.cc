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

#include "oxide_script_message.h"

#include "base/logging.h"

namespace oxide {

// static
void ScriptMessageTraits::Destruct(const ScriptMessage* x) {
  ScriptMessage* self = const_cast<ScriptMessage *>(x);
  if (!x->has_responded_) {
    self->Error(ScriptMessageParams::ERROR_HANDLER_DID_NOT_RESPOND);
  }
  delete x;
}

void ScriptMessage::MakeResponseParams(ScriptMessageParams* params,
                                       ScriptMessageParams::Error error,
                                       scoped_ptr<base::Value> payload) {
  params->serial = serial();
  params->context = context();
  params->type = ScriptMessageParams::TYPE_REPLY;
  params->error = error;
  params->msg_id = msg_id();
  params->wrapped_payload.Set(0, payload.Pass());
}

ScriptMessage::ScriptMessage(int serial,
                             const GURL& context,
                             bool want_reply,
                             const std::string& msg_id,
                             base::ListValue* wrapped_payload)
    : serial_(serial),
      context_(context),
      msg_id_(msg_id),
      has_responded_(!want_reply) {
  if (!wrapped_payload->Remove(0, &payload_)) {
    payload_ = base::Value::CreateNullValue();
  }
}

ScriptMessage::~ScriptMessage() {
  DCHECK(has_responded_);
}

void ScriptMessage::Reply(scoped_ptr<base::Value> payload) {
  if (has_responded_) {
    return;
  }

  has_responded_ = true;

  ScriptMessageParams params;
  MakeResponseParams(&params, ScriptMessageParams::ERROR_OK, payload.Pass());

  DoSendResponse(params);
}

void ScriptMessage::Error(ScriptMessageParams::Error code,
                          scoped_ptr<base::Value> payload) {
  DCHECK(code != ScriptMessageParams::ERROR_OK) <<
      "Use Reply() for non-error responses";
  if (has_responded_) {
    return;
  }

  has_responded_ = true;

  ScriptMessageParams params;
  MakeResponseParams(&params, code, payload.Pass());

  DoSendResponse(params);
}

} // namespace oxide
