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

#ifndef _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_
#define _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_

#include <string>

#include "base/basictypes.h"
#include "base/macros.h"

#include "shared/common/oxide_script_message_handler.h"

namespace oxide {

class ScriptMessage;

namespace qt {

class ScriptMessageHandlerAdapter;

class ScriptMessageHandler FINAL {
 public:
  ~ScriptMessageHandler();

  static ScriptMessageHandler* FromAdapter(
      ScriptMessageHandlerAdapter* adapter);

  const oxide::ScriptMessageHandler* handler() const { return &handler_; }

  void AttachHandler();
  void DetachHandler();

  std::string GetMsgId() const;
  void SetMsgId(const std::string& msg_id);

  const std::vector<GURL>& GetContexts() const;
  void SetContexts(const std::vector<GURL>& contexts);

 private:
  friend class ScriptMessageHandlerAdapter;

  ScriptMessageHandler(ScriptMessageHandlerAdapter* adapter);

  bool ReceiveMessageCallback(oxide::ScriptMessage* message,
                              std::string* error_desc);

  ScriptMessageHandlerAdapter* adapter_;
  oxide::ScriptMessageHandler handler_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageHandler);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_
