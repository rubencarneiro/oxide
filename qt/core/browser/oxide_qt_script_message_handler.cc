// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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
#include "base/values.h"

#include "qt/core/glue/oxide_qt_script_message_handler_proxy_client.h"
#include "shared/common/oxide_script_message.h"

#include "oxide_qt_script_message.h"
#include "oxide_qt_variant_value_converter.h"

namespace oxide {
namespace qt {

bool ScriptMessageHandler::ReceiveMessageCallback(
    oxide::ScriptMessage* message,
    std::unique_ptr<base::Value>* error_payload) {
  std::unique_ptr<ScriptMessage> m(new ScriptMessage(message));

  QVariant error;
  bool success = client_->ReceiveMessage(m.release(), &error);

  if (!success) {
    *error_payload = VariantValueConverter::FromVariantValue(error);
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
                 // The callback cannot run after |this| is deleted, as it
                 // exclusively owns |handler_|
                 base::Unretained(this)));
}

void ScriptMessageHandler::detachHandler() {
  handler_.SetCallback(oxide::ScriptMessageHandler::HandlerCallback());
}

ScriptMessageHandler::ScriptMessageHandler(
    ScriptMessageHandlerProxyClient* client,
    QObject* handle)
    : client_(client) {
  DCHECK(client);
  DCHECK(handle);

  setHandle(handle);
}

ScriptMessageHandler::~ScriptMessageHandler() {}

} // namespace qt
} // namespace oxide
