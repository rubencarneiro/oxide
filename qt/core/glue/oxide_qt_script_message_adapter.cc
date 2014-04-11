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
#include "oxide_qt_script_message_adapter_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "shared/browser/oxide_script_message_impl_browser.h"

namespace oxide {
namespace qt {

ScriptMessageAdapterPrivate::ScriptMessageAdapterPrivate(
    ScriptMessageAdapter* adapter) :
    a(adapter),
    incoming_(NULL) {}

void ScriptMessageAdapterPrivate::Initialize(oxide::ScriptMessage* message) {
  DCHECK(!incoming());
  incoming_ = static_cast<oxide::ScriptMessageImplBrowser *>(message);

  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(message->args().data(), message->args().length())));
  a->args_ = jsondoc.toVariant();
}

// static
ScriptMessageAdapterPrivate* ScriptMessageAdapterPrivate::get(
    ScriptMessageAdapter* adapter) {
  return adapter->priv.data();
}

ScriptMessageAdapter::ScriptMessageAdapter(QObject* q) :
    AdapterBase(q),
    priv(new ScriptMessageAdapterPrivate(this)) {}

ScriptMessageAdapter::~ScriptMessageAdapter() {}

WebFrameAdapter* ScriptMessageAdapter::frame() const {
  return static_cast<WebFrame *>(priv->incoming()->source_frame())->adapter();
}

QString ScriptMessageAdapter::msgId() const {
  return QString::fromStdString(priv->incoming()->msg_id());
}

QUrl ScriptMessageAdapter::context() const {
  return QUrl(QString::fromStdString(priv->incoming()->context().spec()));
}

void ScriptMessageAdapter::reply(const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  priv->incoming()->Reply(QString(jsondoc.toJson()).toStdString());
}

void ScriptMessageAdapter::error(const QString& msg) {
  priv->incoming()->Error(
      oxide::ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
      msg.toStdString());
}

} // namespace qt
} // namespace oxide
