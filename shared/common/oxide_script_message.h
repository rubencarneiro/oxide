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

#ifndef _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_
#define _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "url/gurl.h"

#include "shared/common/oxide_script_message_request.h"

namespace oxide {

class ScriptMessage {
 public:
  virtual ~ScriptMessage();

  void Reply(const std::string& args);
  void Error(ScriptMessageRequest::Error code,
             const std::string& msg);

  int serial() const { return serial_; }
  GURL context() const { return context_; }
  std::string msg_id() const { return msg_id_; }
  std::string args() const { return args_; }

 protected:
  ScriptMessage(int serial,
                const GURL& context,
                const std::string& msg_id,
                const std::string& args);

 private:
  virtual void DoSendReply(const std::string& args) = 0;
  virtual void DoSendError(ScriptMessageRequest::Error code,
                           const std::string& msg) = 0;

  int serial_;
  GURL context_;
  std::string msg_id_;
  std::string args_;
  bool has_responded_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_
