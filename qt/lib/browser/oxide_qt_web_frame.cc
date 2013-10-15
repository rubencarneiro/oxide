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

#include "qt/lib/api/oxide_q_web_frame_base.h"
#include "qt/lib/api/oxide_qquick_web_frame_p.h"
#include "qt/lib/api/private/oxide_qt_qmessage_handler_p.h"
#include "qt/lib/api/private/oxide_qt_qoutgoing_message_request_p.h"
#include "qt/lib/api/private/oxide_qt_qweb_frame_p.h"

namespace oxide {
namespace qt {

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  static_cast<WebFrame *>(child)->q_web_frame->setParent(q_web_frame);
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  static_cast<WebFrame *>(child)->q_web_frame->setParent(NULL);
}

void WebFrame::OnURLChanged() {
  q_web_frame->urlChanged();
}

WebFrame::WebFrame(OxideQWebFrameBase* q_web_frame) :
    q_web_frame(q_web_frame) {}

WebFrame::~WebFrame() {
  delete q_web_frame;
  DCHECK(!q_web_frame);
}

MessageDispatcherBrowser::MessageHandlerVector
WebFrame::GetMessageHandlers() const {
  MessageDispatcherBrowser::MessageHandlerVector list;
  QList<OxideQMessageHandlerBase *>& handlers =
      QWebFrameBasePrivate::get(q_web_frame)->message_handlers();
  for (int i = 0; i < handlers.size(); ++i) {
    list.push_back(QMessageHandlerBasePrivate::get(handlers.at(i))->handler());
  }

  return list;
}

MessageDispatcherBrowser::OutgoingMessageRequestVector
WebFrame::GetOutgoingMessageRequests() const {
  MessageDispatcherBrowser::OutgoingMessageRequestVector list;
  QList<OxideQOutgoingMessageRequestBase *>& requests =
      QWebFrameBasePrivate::get(q_web_frame)->outgoing_message_requests();
  for (int i = 0; i < requests.size(); ++i) {
    list.push_back(
        QOutgoingMessageRequestBasePrivate::get(requests.at(i))->request());
  }

  return list;
}

WebFrameQQuick::WebFrameQQuick() :
    WebFrame(new OxideQQuickWebFrame(this)) {}

WebFrameQQuick::~WebFrameQQuick() {}

OxideQQuickWebFrame* WebFrameQQuick::QQuickWebFrame() const {
  return qobject_cast<OxideQQuickWebFrame *>(q_web_frame);
}

} // namespace qt
} // namespace oxide
