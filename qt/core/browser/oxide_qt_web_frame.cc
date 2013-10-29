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

#include "oxide_qt_web_frame.h"

#include <QObject>

#include "base/logging.h"

#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/private/oxide_qt_message_handler_adapter_p.h"
#include "qt/core/glue/private/oxide_qt_outgoing_message_request_adapter_p.h"
#include "qt/core/glue/private/oxide_qt_web_frame_adapter_p.h"

namespace oxide {
namespace qt {

WebFrame::~WebFrame() {
  delete adapterToQObject(adapter);
  DCHECK(!adapter);
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  adapterToQObject(static_cast<WebFrame *>(child)->adapter)->setParent(
      adapterToQObject(adapter));
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  adapterToQObject(static_cast<WebFrame *>(child)->adapter)->setParent(NULL);
}

void WebFrame::OnURLChanged() {
  adapter->URLChanged();
}

WebFrame::WebFrame(WebFrameAdapter* adapter) :
    adapter(adapter) {
  WebFrameAdapterPrivate::get(adapter)->owner = this;
}

size_t WebFrame::GetMessageHandlerCount() const {
  return adapter->message_handlers().size();
}

oxide::MessageHandler* WebFrame::GetMessageHandlerAt(
    size_t index) const {
  MessageHandlerAdapter* handler = adapter->message_handlers().at(index);
  return &MessageHandlerAdapterPrivate::get(handler)->handler();
}

size_t WebFrame::GetOutgoingMessageRequestCount() const {
  return WebFrameAdapterPrivate::get(
      adapter)->outgoing_message_requests().size();
}

oxide::OutgoingMessageRequest* WebFrame::GetOutgoingMessageRequestAt(
    size_t index) const {
  OutgoingMessageRequestAdapter* req =
      WebFrameAdapterPrivate::get(adapter)->outgoing_message_requests().at(index);
  return &OutgoingMessageRequestAdapterPrivate::get(req)->request();
}

} // namespace qt
} // namespace oxide
