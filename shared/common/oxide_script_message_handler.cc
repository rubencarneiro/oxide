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

#include "oxide_script_message_handler.h"

#include <utility>

#include "base/logging.h"
#include "base/values.h"

#include "oxide_script_message.h"
#include "oxide_script_message_request.h"

namespace oxide {

ScriptMessageHandler::ScriptMessageHandler() {}

bool ScriptMessageHandler::IsValid() const {
  return !msg_id().empty() && contexts().size() > 0 && !callback_.is_null();
}

void ScriptMessageHandler::SetCallback(const HandlerCallback& callback) {
  callback_ = callback;
}

void ScriptMessageHandler::OnReceiveMessage(ScriptMessage* message) const {
  DCHECK_EQ(message->msg_id(), msg_id());
  DCHECK(!callback_.is_null());

  scoped_ptr<base::Value> error_payload;
  bool success = callback_.Run(message, &error_payload);

  if (!success) {
    message->Error(ScriptMessageParams::ERROR_UNCAUGHT_EXCEPTION,
                   std::move(error_payload));
  }
}

} // namespace oxide
