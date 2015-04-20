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

#include <QList>
#include <QString>
#include <QUrl>

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/glue/oxide_qt_script_message_handler_proxy_client.h"
#include "shared/common/oxide_script_message.h"

#include "oxide_qt_script_message.h"

namespace oxide {
namespace qt {

bool ScriptMessageHandler::ReceiveMessageCallback(
    oxide::ScriptMessage* message,
    std::string* error_desc) {
  scoped_ptr<ScriptMessage> m(new ScriptMessage(message));

  QString qerror;
  bool success = client_->ReceiveMessage(m.release(), qerror);

  if (!success) {
    *error_desc = qerror.toStdString();
  }

  return success;
}

QString ScriptMessageHandler::msgId() const {
  return QString::fromStdString(handler_.msg_id());
}

void ScriptMessageHandler::setMsgId(const QString& id) {
  handler_.set_msg_id(id.toStdString());
}

QList<QUrl> ScriptMessageHandler::contexts() const {
  QList<QUrl> list;

  const std::vector<GURL>& contexts = handler_.contexts();
  for (std::vector<GURL>::const_iterator it = contexts.begin();
       it != contexts.end(); ++it) {
    list.append(QUrl(QString::fromStdString((*it).spec())));
  }

  return list;
}

void ScriptMessageHandler::setContexts(const QList<QUrl>& contexts) {
  std::vector<GURL> list;

  for (int i = 0; i < contexts.size(); ++i) {
    list.push_back(GURL(contexts[i].toString().toStdString()));
  }

  handler_.set_contexts(list);
}

void ScriptMessageHandler::attachHandler() {
  handler_.SetCallback(
      base::Bind(&ScriptMessageHandler::ReceiveMessageCallback,
                 base::Unretained(this)));
}

void ScriptMessageHandler::detachHandler() {
  handler_.SetCallback(oxide::ScriptMessageHandler::HandlerCallback());
}

ScriptMessageHandler::ScriptMessageHandler(
    ScriptMessageHandlerProxyClient* client)
    : client_(client) {}

ScriptMessageHandler::~ScriptMessageHandler() {}

// static
ScriptMessageHandler* ScriptMessageHandler::FromProxyHandle(
    ScriptMessageHandlerProxyHandle* handle) {
  return static_cast<ScriptMessageHandler*>(handle->proxy_.data());
}

} // namespace qt
} // namespace oxide
