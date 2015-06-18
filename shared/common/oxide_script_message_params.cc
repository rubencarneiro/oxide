// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_script_message_params.h"

#include "base/logging.h"

namespace oxide {

ScriptMessageParams::ScriptMessageParams()
    : serial(kInvalidSerial),
      type(TYPE_MESSAGE),
      error(ERROR_OK) {}

ScriptMessageParams::ScriptMessageParams(ScriptMessageParams&& other)
    : serial(kInvalidSerial),
      type(TYPE_MESSAGE),
      error(ERROR_OK) {
  std::swap(context, other.context);
  std::swap(serial, other.serial);
  std::swap(type, other.type);
  std::swap(error, other.error);
  std::swap(msg_id, other.msg_id);

  wrapped_payload.Swap(&other.wrapped_payload);
}

void PopulateScriptMessageParams(int serial,
                                 const GURL& context,
                                 const std::string& msg_id,
                                 scoped_ptr<base::Value> payload,
                                 ScriptMessageParams* params) {
  DCHECK(params);
  DCHECK(!msg_id.empty());

  params->context = context;
  params->serial = serial;
  params->type = ScriptMessageParams::TYPE_MESSAGE;
  params->error = ScriptMessageParams::ERROR_OK;
  params->msg_id = msg_id;
  params->wrapped_payload.Set(0, payload ?
      payload.Pass() :
      base::Value::CreateNullValue());
}

} // namespace oxide
