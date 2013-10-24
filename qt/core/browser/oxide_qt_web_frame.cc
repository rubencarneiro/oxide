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

#include "base/logging.h"

#include "qt/quick/api/oxideqquickmessagehandler_p_p.h"
#include "qt/quick/api/oxideqquickoutgoingmessagerequest_p_p.h"
#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebframe_p_p.h"

namespace oxide {
namespace qt {

WebFrame::~WebFrame() {
  delete q_web_frame;
  DCHECK(!q_web_frame);
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  static_cast<WebFrame *>(child)->q_web_frame->setParent(q_web_frame);
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  static_cast<WebFrame *>(child)->q_web_frame->setParent(NULL);
}

void WebFrame::OnURLChanged() {
  q_web_frame->urlChanged();
}

WebFrame::WebFrame() :
    q_web_frame(new OxideQQuickWebFrame(this)) {}

size_t WebFrame::GetMessageHandlerCount() const {
  return OxideQQuickWebFramePrivate::get(
      q_web_frame)->message_handlers().size();
}

oxide::MessageHandler* WebFrame::GetMessageHandlerAt(
    size_t index) const {
  OxideQQuickMessageHandler* handler =
      OxideQQuickWebFramePrivate::get(
        q_web_frame)->message_handlers().at(index);
  // FIXME: Stop using OxideQQuickMessageHandlerPrivate from here
  return OxideQQuickMessageHandlerPrivate::get(handler)->GetHandler();
}

size_t WebFrame::GetOutgoingMessageRequestCount() const {
  return OxideQQuickWebFramePrivate::get(
      q_web_frame)->outgoing_message_requests().size();
}

oxide::OutgoingMessageRequest* WebFrame::GetOutgoingMessageRequestAt(
    size_t index) const {
  OxideQQuickOutgoingMessageRequest* req =
      OxideQQuickWebFramePrivate::get(
        q_web_frame)->outgoing_message_requests().at(index);
  return OxideQQuickOutgoingMessageRequestPrivate::get(req)->request();
}

} // namespace qt
} // namespace oxide
