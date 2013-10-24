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

#include "qt/quick/api/oxideqquickwebframe_p_p.h"
#include "qt/quick/api/oxideqquickwebframe_p.h"

#include "qt/core/browser/oxide_qt_web_frame.h"

#include "qt/quick/api/oxideqquickmessagehandler_p.h"
#include "qt/quick/api/oxideqquickmessagehandler_p_p.h"
#include "qt/quick/api/oxideqquickoutgoingmessagerequest_p_p.h"

namespace oxide {
namespace qt {

QQuickWebFramePrivate::QQuickWebFramePrivate(WebFrame* owner) :
    owner_(owner) {}

// static
QQuickWebFramePrivate* QQuickWebFramePrivate::Create(WebFrame* owner) {
  return new QQuickWebFramePrivate(owner);
}

QQuickWebFramePrivate::~QQuickWebFramePrivate() {
  while (!outgoing_message_requests_.isEmpty()) {
    removeOutgoingMessageRequest(outgoing_message_requests_.first());
  }

  owner_->q_web_frame = NULL;
}

QQuickWebFramePrivate* QQuickWebFramePrivate::get(OxideQQuickWebFrame* frame) {
  return frame->d_func();
}

// static
int QQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  QQuickWebFramePrivate* p = QQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->children().count();
}

// static
OxideQQuickWebFrame* QQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  QQuickWebFramePrivate* p = QQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickWebFrame *>(p->children().at(index));
}

void QQuickWebFramePrivate::addOutgoingMessageRequest(
    OxideQQuickOutgoingMessageRequest* request) {
  Q_ASSERT(!outgoing_message_requests_.contains(request));

  QQuickOutgoingMessageRequestPrivate::get(request)->frame = this;
  outgoing_message_requests_.append(request);  
}

void QQuickWebFramePrivate::removeOutgoingMessageRequest(
    OxideQQuickOutgoingMessageRequest* request) {
  outgoing_message_requests_.removeOne(request);
  QQuickOutgoingMessageRequestPrivate::get(request)->frame = NULL;
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
  QQuickWebFramePrivate* p = QQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* QQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  QQuickWebFramePrivate* p = QQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickMessageHandler *>(
      p->message_handlers().at(index));
}

// static
void QQuickWebFramePrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);
  QQuickWebFramePrivate* p = QQuickWebFramePrivate::get(frame);

  while (p->message_handlers().size() > 0) {
    OxideQQuickMessageHandler* handler = p->message_handlers().first();
    p->message_handlers().removeFirst();
    delete handler;
  }

  emit frame->messageHandlersChanged();
}

} // namespace qt
} // namespace oxide
