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

#ifndef _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_REQUEST_H_
#define _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_REQUEST_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "url/gurl.h"

class OxideMsg_SendMessage_Params;

namespace oxide {

class ScriptMessageRequest {
 public:
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

    ERROR_HANDLER_DID_NOT_RESPOND
  };

  virtual ~ScriptMessageRequest();

  int serial() const { return serial_; }
  GURL context() const { return context_; }
  std::string msg_id() const { return msg_id_; }
  std::string args() const { return args_; }

  bool SendMessage();

  bool IsWaitingForResponse() const;
  void OnReceiveResponse(const std::string& payload,
                         Error error);

 protected:
  ScriptMessageRequest(int serial,
                       const GURL& context,
                       bool want_reply,
                       const std::string& msg_id,
                       const std::string& args);

 private:
  virtual bool DoSendMessage(const OxideMsg_SendMessage_Params& params) = 0;

  virtual void OnReply(const std::string& args) = 0;
  virtual void OnError(Error error, const std::string& msg) = 0;

  int serial_;
  GURL context_;
  bool want_reply_;
  std::string msg_id_;
  std::string args_;
  bool has_sent_message_;
  bool has_received_response_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageRequest);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_REQUEST_H_
