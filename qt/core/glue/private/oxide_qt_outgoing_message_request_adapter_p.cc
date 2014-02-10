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

#include "oxide_qt_outgoing_message_request_adapter_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "qt/core/glue/oxide_qt_outgoing_message_request_adapter.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter_p.h"

namespace oxide {
namespace qt {

OutgoingMessageRequestAdapterPrivate::OutgoingMessageRequestAdapterPrivate(
    OutgoingMessageRequestAdapter* adapter) :
    a(adapter),
    weak_factory_(this) {}

void OutgoingMessageRequestAdapterPrivate::ReceiveReplyCallback(
    const std::string& args) {
  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(args.data(), args.length())));

  a->OnReceiveReply(jsondoc.toVariant());
}

void OutgoingMessageRequestAdapterPrivate::ReceiveErrorCallback(
    int error,
    const std::string& msg) {
  a->OnReceiveError(error, QString::fromStdString(msg));
}

// static
OutgoingMessageRequestAdapterPrivate* OutgoingMessageRequestAdapterPrivate::Create(
    OutgoingMessageRequestAdapter* adapter) {
  return new OutgoingMessageRequestAdapterPrivate(adapter);
}

void OutgoingMessageRequestAdapterPrivate::RemoveFromOwner() {
  if (frame) {
    frame->RemoveOutgoingMessageRequest(a);
    frame = NULL;
  }
}

base::WeakPtr<OutgoingMessageRequestAdapterPrivate>
OutgoingMessageRequestAdapterPrivate::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

// static
OutgoingMessageRequestAdapterPrivate* OutgoingMessageRequestAdapterPrivate::get(
    OutgoingMessageRequestAdapter* adapter) {
  return adapter->priv.data();
}

} // namespace qt
} // namespace oxide
