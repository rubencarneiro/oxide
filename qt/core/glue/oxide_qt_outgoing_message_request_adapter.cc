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

#include "oxide_qt_outgoing_message_request_adapter.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "base/bind.h"

#include "qt/core/glue/private/oxide_qt_outgoing_message_request_adapter_p.h"

namespace oxide {
namespace qt {

void OutgoingMessageRequestAdapter::ReceiveReplyCallback(
    const std::string& args) {
  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(args.data(), args.length())));

  OnReceiveReply(jsondoc.toVariant());
}

void OutgoingMessageRequestAdapter::ReceiveErrorCallback(
    int error,
    const std::string& msg) {
  OnReceiveError(error, QString::fromStdString(msg));
}

OutgoingMessageRequestAdapter::OutgoingMessageRequestAdapter() :
    priv_(OutgoingMessageRequestAdapterPrivate::Create(this)) {
  priv_->request().SetReplyCallback(
      base::Bind(
        &OutgoingMessageRequestAdapter::ReceiveReplyCallback,
        priv_->GetWeakPtr()));
  priv_->request().SetErrorCallback(
      base::Bind(
        &OutgoingMessageRequestAdapter::ReceiveErrorCallback,
        priv_->GetWeakPtr()));
}

OutgoingMessageRequestAdapter::~OutgoingMessageRequestAdapter() {}

oxide::OutgoingMessageRequest* OutgoingMessageRequestAdapter::GetRequest() {
  return &priv_->request();
}

} // namespace qt
} // namespace oxide
