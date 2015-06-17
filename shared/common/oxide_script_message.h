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

#ifndef _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_
#define _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_

#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "url/gurl.h"

#include "shared/common/oxide_script_message_params.h"

namespace base {
class ListValue;
class Value;
}

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

  void Reply(scoped_ptr<base::Value> payload);
  void Error(ScriptMessageParams::Error code,
             scoped_ptr<base::Value> payload = base::Value::CreateNullValue());

  int serial() const { return serial_; }
  GURL context() const { return context_; }
  std::string msg_id() const { return msg_id_; }
  base::Value* payload() const { return payload_.get(); }
  bool want_reply() const { return !has_responded_; }

 protected:
  friend struct ScriptMessageTraits;
  ScriptMessage(int serial,
                const GURL& context,
                bool want_reply,
                const std::string& msg_id,
                base::ListValue* wrapped_payload);
  virtual ~ScriptMessage();

 private:
  virtual void DoSendResponse(const ScriptMessageParams& params) = 0;
  void MakeResponseParams(ScriptMessageParams* params,
                          ScriptMessageParams::Error error,
                          scoped_ptr<base::Value> payload);

  int serial_;
  GURL context_;
  std::string msg_id_;
  scoped_ptr<base::Value> payload_;
  bool has_responded_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_SCRIPT_MESSAGE_H_
