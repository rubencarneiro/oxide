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
    incoming_(NULL),
    weak_factory_(this) {}

void ScriptMessageAdapterPrivate::Initialize(oxide::ScriptMessage* message) {
  DCHECK(!incoming());
  incoming_ = static_cast<oxide::ScriptMessageImplBrowser *>(message);

  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(message->args().data(), message->args().length())));
  a->args_ = jsondoc.toVariant();
}

void ScriptMessageAdapterPrivate::Consume(
    scoped_ptr<oxide::ScriptMessage> message) {
  DCHECK_EQ(incoming(), message.get());
  DCHECK(!owned_incoming_);
  owned_incoming_ = message.Pass();
}

void ScriptMessageAdapterPrivate::Invalidate() {
  incoming_ = NULL;
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
  oxide::ScriptMessageImplBrowser* message = priv->incoming();
  if (!message) {
    return NULL;
  }

  return static_cast<WebFrame *>(message->GetSourceFrame())->GetAdapter();
}

QUrl ScriptMessageAdapter::context() const {
  oxide::ScriptMessageImplBrowser* message = priv->incoming();
  if (!message) {
    return QUrl();
  }

  return QUrl(QString::fromStdString(message->context().spec()));
}

void ScriptMessageAdapter::reply(const QVariant& args) {
  oxide::ScriptMessageImplBrowser* message = priv->incoming();
  if (!message) {
    return;
  }

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  message->Reply(QString(jsondoc.toJson()).toStdString());
}

void ScriptMessageAdapter::error(const QString& msg) {
  oxide::ScriptMessageImplBrowser* message = priv->incoming();
  if (!message) {
    return;
  }

  message->Error(
      oxide::ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
      msg.toStdString());
}

} // namespace qt
} // namespace oxide
