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

OxideQQuickWebFramePrivate::OxideQQuickWebFramePrivate(
    oxide::qt::WebFrame* owner) :
    owner_(owner) {}

// static
OxideQQuickWebFramePrivate* OxideQQuickWebFramePrivate::Create(
    oxide::qt::WebFrame* owner) {
  return new OxideQQuickWebFramePrivate(owner);
}

OxideQQuickWebFramePrivate::~OxideQQuickWebFramePrivate() {
  while (!outgoing_message_requests_.isEmpty()) {
    removeOutgoingMessageRequest(outgoing_message_requests_.first());
  }

  owner_->q_web_frame = NULL;
}

OxideQQuickWebFramePrivate* OxideQQuickWebFramePrivate::get(
    OxideQQuickWebFrame* frame) {
  return frame->d_func();
}

// static
int OxideQQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->children().count();
}

// static
OxideQQuickWebFrame* OxideQQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
      static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickWebFrame *>(p->children().at(index));
}

void OxideQQuickWebFramePrivate::addOutgoingMessageRequest(
    OxideQQuickOutgoingMessageRequest* request) {
  Q_ASSERT(!outgoing_message_requests_.contains(request));

  OxideQQuickOutgoingMessageRequestPrivate::get(request)->frame = this;
  outgoing_message_requests_.append(request);  
}

void OxideQQuickWebFramePrivate::removeOutgoingMessageRequest(
    OxideQQuickOutgoingMessageRequest* request) {
  outgoing_message_requests_.removeOne(request);
  OxideQQuickOutgoingMessageRequestPrivate::get(request)->frame = NULL;
}

// static
void OxideQQuickWebFramePrivate::messageHandler_append(
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
int OxideQQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickMessageHandler *>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebFramePrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(frame);

  while (p->message_handlers().size() > 0) {
    OxideQQuickMessageHandler* handler = p->message_handlers().first();
    p->message_handlers().removeFirst();
    delete handler;
  }

  emit frame->messageHandlersChanged();
}
