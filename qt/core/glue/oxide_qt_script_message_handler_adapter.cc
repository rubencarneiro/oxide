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

#include <vector>

#include "qt/core/browser/oxide_qt_script_message_handler.h"

namespace oxide {
namespace qt {

ScriptMessageHandlerAdapter::ScriptMessageHandlerAdapter(QObject* q)
    : AdapterBase(q),
      handler_(new ScriptMessageHandler(this)) {}

ScriptMessageHandlerAdapter::~ScriptMessageHandlerAdapter() {}

QString ScriptMessageHandlerAdapter::msgId() const {
  return QString::fromStdString(handler_->GetMsgId());
}

void ScriptMessageHandlerAdapter::setMsgId(const QString& id) {
  handler_->SetMsgId(id.toStdString());
}

QList<QUrl> ScriptMessageHandlerAdapter::contexts() const {
  QList<QUrl> list;

  const std::vector<GURL>& contexts = handler_->GetContexts();
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

  handler_->SetContexts(list);
}

void ScriptMessageHandlerAdapter::attachHandler() {
  handler_->AttachHandler();
}

void ScriptMessageHandlerAdapter::detachHandler() {
  handler_->DetachHandler();
}

} // namespace qt
} // namespace oxide
