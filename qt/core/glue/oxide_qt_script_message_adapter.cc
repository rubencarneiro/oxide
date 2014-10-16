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

#include "oxide_qt_script_message_adapter.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "qt/core/browser/oxide_qt_script_message.h"

#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

ScriptMessageAdapter::ScriptMessageAdapter(QObject* q)
    : AdapterBase(q),
      message_(new ScriptMessage(this)) {}

ScriptMessageAdapter::~ScriptMessageAdapter() {}

WebFrameAdapter* ScriptMessageAdapter::frame() const {
  return WebFrameAdapter::FromWebFrame(message_->GetFrame());
}

QString ScriptMessageAdapter::msgId() const {
  return QString::fromStdString(message_->GetMsgId());
}

QUrl ScriptMessageAdapter::context() const {
  return QUrl(QString::fromStdString(message_->GetContext().spec()));
}

QVariant ScriptMessageAdapter::args() const {
  if (!args_.isValid()) {
    QJsonDocument jsondoc(QJsonDocument::fromJson(
        QByteArray(message_->GetArgs().data(), message_->GetArgs().length())));
    args_ = jsondoc.toVariant();
  }

  return args_;
}

void ScriptMessageAdapter::reply(const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  message_->Reply(QString(jsondoc.toJson()).toStdString());
}

void ScriptMessageAdapter::error(const QString& msg) {
  message_->Error(msg.toStdString());
}

} // namespace qt
} // namespace oxide
