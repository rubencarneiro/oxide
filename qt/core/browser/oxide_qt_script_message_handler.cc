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

#include "oxide_qt_script_message_handler.h"

#include <QString>

#include "base/bind.h"

#include "qt/core/glue/oxide_qt_script_message_handler_adapter.h"
#include "shared/common/oxide_script_message.h"

#include "oxide_qt_script_message.h"

namespace oxide {
namespace qt {

ScriptMessageHandler::ScriptMessageHandler(
    ScriptMessageHandlerAdapter* adapter)
    : adapter_(adapter) {}

bool ScriptMessageHandler::ReceiveMessageCallback(
    oxide::ScriptMessage* message,
    std::string* error_desc) {
  QString qerror;

  ScriptMessageAdapter* qmessage = adapter_->CreateScriptMessage();
  ScriptMessage::FromAdapter(qmessage)->Initialize(message);

  bool success = adapter_->OnReceiveMessage(qmessage, qerror);

  if (!success) {
    *error_desc = qerror.toStdString();
  }

  return success;
}

ScriptMessageHandler::~ScriptMessageHandler() {}

// static
ScriptMessageHandler* ScriptMessageHandler::FromAdapter(
    ScriptMessageHandlerAdapter* adapter) {
  return adapter->handler_.data();
}

void ScriptMessageHandler::AttachHandler() {
  handler_.SetCallback(
      base::Bind(&ScriptMessageHandler::ReceiveMessageCallback,
                 base::Unretained(this)));
}

void ScriptMessageHandler::DetachHandler() {
  handler_.SetCallback(oxide::ScriptMessageHandler::HandlerCallback());
}

std::string ScriptMessageHandler::GetMsgId() const {
  return handler_.msg_id();
}

void ScriptMessageHandler::SetMsgId(const std::string& msg_id) {
  handler_.set_msg_id(msg_id);
}

const std::vector<GURL>& ScriptMessageHandler::GetContexts() const {
  return handler_.contexts();
}

void ScriptMessageHandler::SetContexts(const std::vector<GURL>& contexts) {
  handler_.set_contexts(contexts);
}

} // namespace qt
} // namespace oxide
