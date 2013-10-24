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

#include "oxide_qt_message_handler_adapter.h"

#include <vector>

#include "base/bind.h"

#include "shared/browser/oxide_incoming_message.h"

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/private/oxide_qt_message_handler_adapter_p.h"

#include "qt/core/api/oxideqincomingmessage.h"

namespace oxide {
namespace qt {

void MessageHandlerAdapter::ReceiveMessageCallback(
    oxide::IncomingMessage* message,
    bool* delivered,
    bool* error,
    std::string& error_desc) {
  *delivered = true;

  QString qerror;

  *error = !OnReceiveMessage(
      new OxideQIncomingMessage(message),
      static_cast<WebFrame *>(message->frame())->q_web_frame,
      qerror);

  if (*error) {
    error_desc = qerror.toStdString();
  }
}

MessageHandlerAdapter::MessageHandlerAdapter() :
    priv_(MessageHandlerAdapterPrivate::Create(this)) {}

MessageHandlerAdapter::~MessageHandlerAdapter() {}

QString MessageHandlerAdapter::msgId() const {
  return QString::fromStdString(priv_->handler().msg_id());
}

void MessageHandlerAdapter::setMsgId(const QString& id) {
  priv_->handler().set_msg_id(id.toStdString());
}

QList<QString> MessageHandlerAdapter::worldIds() const {
  QList<QString> list;

  const std::vector<std::string>& ids = priv_->handler().world_ids();
  for (std::vector<std::string>::const_iterator it = ids.begin();
       it != ids.end(); ++it) {
    list.append(QString::fromStdString(*it));
  }

  return list;
}

void MessageHandlerAdapter::setWorldIds(const QList<QString>& ids) {
  std::vector<std::string> list;

  for (int i = 0; i < ids.size(); ++i) {
    list.push_back(ids[i].toStdString());
  }

  priv_->handler().set_world_ids(list);
}

void MessageHandlerAdapter::attachHandler() {
  priv_->handler().SetCallback(
      base::Bind(&MessageHandlerAdapter::ReceiveMessageCallback,
      priv_->GetWeakPtr()));
}

void MessageHandlerAdapter::detachHandler() {
  priv_->handler().SetCallback(oxide::MessageHandler::HandlerCallback());
}

oxide::MessageHandler* MessageHandlerAdapter::GetHandler() {
  return &priv_->handler();
}

} // namespace qt
} // namespace oxide
