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

#ifndef _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_HANDLER_H_
#define _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_HANDLER_H_

#include <string>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"

#include "shared/common/oxide_script_message_handler.h"

#include "shared/renderer/oxide_v8_scoped_persistent.h"

namespace base {
class Value;
}

namespace oxide {

class ScriptMessage;
class ScriptMessageManager;

class ScriptMessageHandlerRenderer {
 public:
  ScriptMessageHandlerRenderer(ScriptMessageManager* mm,
                               const std::string& msg_id,
                               const v8::Handle<v8::Function>& callback);

  ScriptMessageHandler& handler() { return handler_; }

 private:
  bool ReceiveMessageCallback(ScriptMessage* message,
                              scoped_ptr<base::Value>* error_payload);

  ScriptMessageManager* manager_;
  ScriptMessageHandler handler_;
  ScopedPersistent<v8::Function> callback_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_SCRIPT_MESSAGE_HANDLER_H_
