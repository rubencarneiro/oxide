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

#include "oxide_qt_message_handler_adapter_p.h"

#include "shared/browser/oxide_incoming_message.h"

#include "qt/core/api/oxideqincomingmessage.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/oxide_qt_message_handler_adapter.h"

namespace oxide {
namespace qt {

MessageHandlerAdapterPrivate::MessageHandlerAdapterPrivate(
    MessageHandlerAdapter* adapter) :
    pub_(adapter),
    weak_factory_(this) {}

void MessageHandlerAdapterPrivate::ReceiveMessageCallback(
    oxide::IncomingMessage* message,
    bool* delivered,
    bool* error,
    std::string& error_desc) {
  *delivered = true;

  QString qerror;

  *error = !pub_->OnReceiveMessage(
      new OxideQIncomingMessage(message),
      static_cast<WebFrame *>(message->frame())->q_web_frame,
      qerror);

  if (*error) {
    error_desc = qerror.toStdString();
  }
}

// static
MessageHandlerAdapterPrivate* MessageHandlerAdapterPrivate::Create(
    MessageHandlerAdapter* adapter) {
  return new MessageHandlerAdapterPrivate(adapter);
}

base::WeakPtr<MessageHandlerAdapterPrivate>
MessageHandlerAdapterPrivate::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

// static
MessageHandlerAdapterPrivate* MessageHandlerAdapterPrivate::get(
    MessageHandlerAdapter* adapter) {
  return adapter->priv_.data();
}

} // namespace qt
} // namespace oxide
