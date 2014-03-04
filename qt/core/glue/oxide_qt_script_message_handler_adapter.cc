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

#include "oxide_qt_script_message_handler_adapter.h"
#include "oxide_qt_script_message_handler_adapter_p.h"

#include <vector>

#include "base/bind.h"

#include "shared/common/oxide_script_message.h"

#include "oxide_qt_script_message_adapter_p.h"

namespace oxide {
namespace qt {

ScriptMessageHandlerAdapterPrivate::ScriptMessageHandlerAdapterPrivate(
    ScriptMessageHandlerAdapter* adapter) :
    a(adapter) {}

// static
ScriptMessageHandlerAdapterPrivate* ScriptMessageHandlerAdapterPrivate::get(
    ScriptMessageHandlerAdapter* adapter) {
  return adapter->priv.data();
}

bool ScriptMessageHandlerAdapterPrivate::ReceiveMessageCallback(
    oxide::ScriptMessage* message, std::string* error_desc) {
  QString qerror;

  ScriptMessageAdapter* qmessage = a->CreateScriptMessage();
  ScriptMessageAdapterPrivate::get(qmessage)->Initialize(message);

  bool success = a->OnReceiveMessage(qmessage, qerror);

  if (!success) {
    *error_desc = qerror.toStdString();
  }

  return success;
}

ScriptMessageHandlerAdapter::ScriptMessageHandlerAdapter(QObject* q) :
    AdapterBase(q),
    priv(new ScriptMessageHandlerAdapterPrivate(this)) {}

ScriptMessageHandlerAdapter::~ScriptMessageHandlerAdapter() {}

QString ScriptMessageHandlerAdapter::msgId() const {
  return QString::fromStdString(priv->handler.msg_id());
}

void ScriptMessageHandlerAdapter::setMsgId(const QString& id) {
  priv->handler.set_msg_id(id.toStdString());
}

QList<QUrl> ScriptMessageHandlerAdapter::contexts() const {
  QList<QUrl> list;

  const std::vector<GURL>& contexts = priv->handler.contexts();
  for (std::vector<GURL>::const_iterator it = contexts.begin();
       it != contexts.end(); ++it) {
    list.append(QUrl(QString::fromStdString((*it).spec())));
  }

  return list;
}

void ScriptMessageHandlerAdapter::setContexts(const QList<QUrl>& contexts) {
  std::vector<GURL> list;

  for (int i = 0; i < contexts.size(); ++i) {
    list.push_back(GURL(contexts[i].toString().toStdString()));
  }

  priv->handler.set_contexts(list);
}

void ScriptMessageHandlerAdapter::attachHandler() {
  priv->handler.SetCallback(
      base::Bind(&ScriptMessageHandlerAdapterPrivate::ReceiveMessageCallback,
                 base::Unretained(priv.data())));
}

void ScriptMessageHandlerAdapter::detachHandler() {
  priv->handler.SetCallback(oxide::ScriptMessageHandler::HandlerCallback());
}

} // namespace qt
} // namespace oxide
