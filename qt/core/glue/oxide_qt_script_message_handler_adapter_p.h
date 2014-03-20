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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/common/oxide_script_message_handler.h"

namespace oxide {

class ScriptMessage;

namespace qt {

class ScriptMessageHandlerAdapter;

class ScriptMessageHandlerAdapterPrivate FINAL {
 public:
  ScriptMessageHandlerAdapterPrivate(ScriptMessageHandlerAdapter* adapter);

  static ScriptMessageHandlerAdapterPrivate* get(
      ScriptMessageHandlerAdapter* adapter);

  oxide::ScriptMessageHandler handler;

 private:
  friend class ScriptMessageHandlerAdapter;

  bool ReceiveMessageCallback(oxide::ScriptMessage* message,
                              std::string* error_desc);

  ScriptMessageHandlerAdapter* a;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageHandlerAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_SCRIPT_MESSAGE_HANDLER_ADAPTER_H_
