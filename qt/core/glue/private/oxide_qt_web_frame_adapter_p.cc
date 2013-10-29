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

#include "oxide_qt_web_frame_adapter_p.h"

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"

#include "oxide_qt_outgoing_message_request_adapter_p.h"

namespace oxide {
namespace qt {

WebFrameAdapterPrivate::WebFrameAdapterPrivate() :
    owner(NULL) {}

// static
WebFrameAdapterPrivate* WebFrameAdapterPrivate::Create() {
  return new WebFrameAdapterPrivate();
}

WebFrameAdapterPrivate::~WebFrameAdapterPrivate() {
  while (!outgoing_message_requests_.isEmpty()) {
    RemoveOutgoingMessageRequest(outgoing_message_requests_.first());
  }

  owner->adapter = NULL;
}

void WebFrameAdapterPrivate::AddOutgoingMessageRequest(
    OutgoingMessageRequestAdapter* request) {
  DCHECK(!outgoing_message_requests_.contains(request));

  OutgoingMessageRequestAdapterPrivate::get(request)->frame = this;
  outgoing_message_requests_.append(request);
}

void WebFrameAdapterPrivate::RemoveOutgoingMessageRequest(
    OutgoingMessageRequestAdapter* request) {
  outgoing_message_requests_.removeOne(request);
  OutgoingMessageRequestAdapterPrivate::get(request)->frame = NULL;
}

// static
WebFrameAdapterPrivate* WebFrameAdapterPrivate::get(WebFrameAdapter* adapter) {
  return adapter->priv.data();
}

} // namespace qt
} // namespace oxide
