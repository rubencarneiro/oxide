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

#include "oxide_qt_qweb_frame_p.h"

#include "qt/lib/api/public/oxide_q_web_frame_base.h"
#include "qt/lib/api/public/oxide_qquick_message_handler_p.h"
#include "qt/lib/api/public/oxide_qquick_web_frame_p.h"
#include "qt/lib/browser/oxide_qt_web_frame.h"

#include "oxide_qt_qmessage_handler_p.h"
#include "oxide_qt_qoutgoing_message_request_p.h"

namespace oxide {
namespace qt {

QWebFrameBasePrivate::QWebFrameBasePrivate(WebFrame* owner) :
    owner_(owner) {}

QWebFrameBasePrivate::~QWebFrameBasePrivate() {
  while (!outgoing_message_requests_.isEmpty()) {
    removeOutgoingMessageRequest(outgoing_message_requests_.first());
  }
}

QWebFrameBasePrivate* QWebFrameBasePrivate::get(OxideQWebFrameBase* frame) {
  return frame->d_func();
}

void QWebFrameBasePrivate::addOutgoingMessageRequest(
    OxideQOutgoingMessageRequestBase* request) {
  Q_ASSERT(!outgoing_message_requests_.contains(request));

  QOutgoingMessageRequestBasePrivate::get(request)->frame = this;
  outgoing_message_requests_.append(request);  
}

void QWebFrameBasePrivate::removeOutgoingMessageRequest(
    OxideQOutgoingMessageRequestBase* request) {
  outgoing_message_requests_.removeOne(request);
  QOutgoingMessageRequestBasePrivate::get(request)->frame = NULL;
}

QQuickWebFramePrivate::QQuickWebFramePrivate(WebFrameQQuick* owner) :
    QWebFrameBasePrivate(owner) {}

// static
QQuickWebFramePrivate* QQuickWebFramePrivate::Create(WebFrameQQuick* owner) {
  return new QQuickWebFramePrivate(owner);
}

// static
int QQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  QWebFrameBasePrivate* p = QWebFrameBasePrivate::get(
        static_cast<OxideQWebFrameBase *>(prop->object));

  if (p->owner()->ChildCount() > INT_MAX) {
    qWarning() << "Number of child frames exceed maximum";
    return INT_MAX;
  }

  return static_cast<int>(p->owner()->ChildCount());
}

// static
OxideQQuickWebFrame* QQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  QWebFrameBasePrivate* p = QWebFrameBasePrivate::get(
        static_cast<OxideQWebFrameBase *>(prop->object));

  return static_cast<WebFrameQQuick *>(
      p->owner()->ChildAt(index))->QQuickWebFrame();
}

// static
void QQuickWebFramePrivate::messageHandler_append(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    OxideQQuickMessageHandler* value) {
  if (!value) {
    return;
  }

  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);

  frame->addMessageHandler(value);
}

// static
int QQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  QWebFrameBasePrivate* p = QWebFrameBasePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* QQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  QWebFrameBasePrivate* p = QWebFrameBasePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickMessageHandler *>(
      p->message_handlers().at(index));
}

// static
void QQuickWebFramePrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);
  QWebFrameBasePrivate* p = QWebFrameBasePrivate::get(frame);

  while (p->message_handlers().size() > 0) {
    OxideQMessageHandlerBase* handler = p->message_handlers().first();
    p->message_handlers().removeFirst();
    delete handler;
  }

  emit frame->messageHandlersChanged();
}

} // namespace qt
} // namespace oxide
