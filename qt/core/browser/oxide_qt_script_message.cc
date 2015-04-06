// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qt_script_message.h"

#include <QByteArray>
#include <QJsonDocument>

#include "base/logging.h"

#include "shared/browser/oxide_script_message_impl_browser.h"
#include "shared/common/oxide_script_message_request.h"

#include "oxide_qt_web_frame.h"

namespace oxide {
namespace qt {

WebFrameProxyHandle* ScriptMessage::frame() const {
  return static_cast<WebFrame*>(impl_->source_frame())->handle();
}

QString ScriptMessage::msgId() const {
  return QString::fromStdString(impl_->msg_id());
}

QUrl ScriptMessage::context() const {
  return QUrl(QString::fromStdString(impl_->context().spec()));
}

QVariant ScriptMessage::args() const {
  if (!args_.isValid()) {
    QJsonDocument jsondoc(QJsonDocument::fromJson(
        QByteArray(impl_->args().data(), impl_->args().length())));
    args_ = jsondoc.toVariant();
  }

  return args_;
}

void ScriptMessage::reply(const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));
  impl_->Reply(QString(jsondoc.toJson()).toStdString());
}

void ScriptMessage::error(const QString& msg) {
  impl_->Error(oxide::ScriptMessageRequest::ERROR_HANDLER_REPORTED_ERROR,
               msg.toStdString());
}

ScriptMessage::ScriptMessage() {}

ScriptMessage::~ScriptMessage() {}

void ScriptMessage::Initialize(oxide::ScriptMessage* message) {
  DCHECK(!impl_.get());
  impl_ = static_cast<oxide::ScriptMessageImplBrowser*>(message);
}

// static
ScriptMessage* ScriptMessage::FromProxyHandle(ScriptMessageProxyHandle* handle) {
  return static_cast<ScriptMessage*>(handle->proxy_.data());
}

} // namespace qt
} // namespace oxide
