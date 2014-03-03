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
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

#include "shared/common/oxide_script_message_request.h"

class OxideMsg_SendMessage_Params;

namespace oxide {

class ScriptMessage;

struct ScriptMessageTraits {
  static void Destruct(const ScriptMessage* x);
};

// We use RefCountedThreadSafe here so we can use a different trait,
// which we need in order to be able to send an error on destruction if the
// handler doesn't respond itself. We can't do this from the destructor,
// as it relies a vcall in to the derived class
class ScriptMessage :
    public base::RefCountedThreadSafe<ScriptMessage, ScriptMessageTraits> {
 public:

  void Reply(const std::string& args);
  void Error(ScriptMessageRequest::Error code,
             const std::string& msg);

  int serial() const { return serial_; }
  GURL context() const { return context_; }
  std::string msg_id() const { return msg_id_; }
  std::string args() const { return args_; }
  bool want_reply() const { return !has_responded_; }

 protected:
  friend struct ScriptMessageTraits;
  ScriptMessage(int serial,
                const GURL& context,
                bool want_reply,
                const std::string& msg_id,
                const std::string& args);
  virtual ~ScriptMessage();

 private:
  virtual void DoSendResponse(const OxideMsg_SendMessage_Params& params) = 0;
  void MakeParams(OxideMsg_SendMessage_Params* params);

  int serial_;
  GURL context_;
  std::string msg_id_;
  std::string args_;
  bool has_responded_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_
