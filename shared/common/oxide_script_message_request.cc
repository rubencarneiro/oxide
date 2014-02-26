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

#include "oxide_script_message_request.h"

#include "base/logging.h"

namespace oxide {

ScriptMessageRequest::ScriptMessageRequest(int serial,
                                           const GURL& context,
                                           const std::string& msg_id,
                                           const std::string& args) :
    serial_(serial),
    context_(context),
    msg_id_(msg_id),
    args_(args),
    has_sent_message_(false),
    has_received_response_(false) {}

ScriptMessageRequest::~ScriptMessageRequest() {}

bool ScriptMessageRequest::SendMessage() {
  DCHECK(!has_sent_message_);
  has_sent_message_ = true;
  return DoSendMessage();
}

bool ScriptMessageRequest::IsWaitingForResponse() const {
  return !has_received_response_;
}

void ScriptMessageRequest::OnReceiveResponse(const std::string& payload,
                                             Error error) {
  DCHECK(has_sent_message_ && !has_received_response_);
  has_received_response_ = true;

  if (error == ERROR_OK) {
    OnReply(payload);
  } else {
    OnError(error, payload);
  }
}

} // namespace oxide
