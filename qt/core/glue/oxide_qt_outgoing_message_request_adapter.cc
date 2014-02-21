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

#include "base/bind.h"

#include "qt/core/glue/private/oxide_qt_outgoing_message_request_adapter_p.h"

namespace oxide {
namespace qt {

OutgoingMessageRequestAdapter::OutgoingMessageRequestAdapter(
    QObject* q) :
    AdapterBase(q),
    priv(OutgoingMessageRequestAdapterPrivate::Create(this)) {
  priv->request().SetReplyCallback(
      base::Bind(
        &OutgoingMessageRequestAdapterPrivate::ReceiveReplyCallback,
        priv->GetWeakPtr()));
  priv->request().SetErrorCallback(
      base::Bind(
        &OutgoingMessageRequestAdapterPrivate::ReceiveErrorCallback,
        priv->GetWeakPtr()));
}

OutgoingMessageRequestAdapter::~OutgoingMessageRequestAdapter() {}

void OutgoingMessageRequestAdapter::removeFromOwner() {
  priv->RemoveFromOwner();
}

} // namespace qt
} // namespace oxide
