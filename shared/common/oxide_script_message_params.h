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

#ifndef _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_PARAMS_H_
#define _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_PARAMS_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "url/gurl.h"

namespace oxide {

struct ScriptMessageParams {
  ScriptMessageParams();
  ScriptMessageParams(ScriptMessageParams&& other);

  // An identifier for the context in which the user script containing
  // the message handler or initiating a message lives
  GURL context;

  // The message serial is used to identify replies and deliver them
  // to the correct request
  int serial;

  enum Type {
    TYPE_MESSAGE,
    TYPE_MESSAGE_NO_REPLY,
    TYPE_REPLY
  };

  // The type of message
  Type type;

  enum Error {
    ERROR_OK,

    // Destination frame or context not found
    ERROR_INVALID_DESTINATION,

    // The message handler threw an exception
    ERROR_UNCAUGHT_EXCEPTION,

    // No handler was registered for this message
    ERROR_NO_HANDLER,

    // The handler reporter an error via the error() function
    ERROR_HANDLER_REPORTED_ERROR,

    // The handler terminated the message transaction without
    // replying
    ERROR_HANDLER_DID_NOT_RESPOND
  };

  // The error code (only applies if the message type is TYPE_REPLY)
  Error error;

  // The message ID
  std::string msg_id;

  // The message payload. This is a list of one value, with that value
  // being the actual payload
  base::ListValue wrapped_payload;
};

void PopulateScriptMessageParams(int serial,
                                 bool expects_reply,
                                 const GURL& context,
                                 const std::string& msg_id,
                                 scoped_ptr<base::Value> payload,
                                 ScriptMessageParams* params);

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_PARAMS_H_
