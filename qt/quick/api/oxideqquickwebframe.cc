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

#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebframe_p_p.h"

#include <QtDebug>
#include <QString>

#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagehandler_p_p.h"
#include "oxideqquickscriptmessagerequest_p.h"
#include "oxideqquickscriptmessagerequest_p_p.h"

OxideQQuickWebFramePrivate::OxideQQuickWebFramePrivate(
    OxideQQuickWebFrame* q) :
    oxide::qt::WebFrameAdapter(q) {}

void OxideQQuickWebFramePrivate::URLChanged() {
  Q_Q(OxideQQuickWebFrame);

  emit q->urlChanged();
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

// static
void OxideQQuickWebFramePrivate::messageHandler_append(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    OxideQQuickScriptMessageHandler* value) {
  if (!value) {
    return;
  }

  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);

  frame->addMessageHandler(value);
}

// static
int OxideQQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return adapterToQObject<OxideQQuickScriptMessageHandler>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebFramePrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(frame);

  while (p->message_handlers().size() > 0) {
    oxide::qt::ScriptMessageHandlerAdapter* handler =
        p->message_handlers().first();
    p->message_handlers().removeFirst();
    delete adapterToQObject(handler);
  }

  emit frame->messageHandlersChanged();
}

OxideQQuickWebFrame::OxideQQuickWebFrame() :
    d_ptr(new OxideQQuickWebFramePrivate(this)) {}

void OxideQQuickWebFrame::childEvent(QChildEvent* event) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickWebFrame* child = qobject_cast<OxideQQuickWebFrame *>(event->child());
  if (!child) {
    // We also manage message handlers and outgoing message requests
    return;
  }
  
  if (event->added()) {
    d->children().append(child);
  } else if (event->removed()) {
    d->children().removeOne(child);
  }

  emit childFramesChanged();
}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {
  Q_D(OxideQQuickWebFrame);

  while (d->message_handlers().size() > 0) {
    removeMessageHandler(
        adapterToQObject<OxideQQuickScriptMessageHandler>(
          d->message_handlers().at(0)));
  }
}

QUrl OxideQQuickWebFrame::url() const {
  Q_D(const OxideQQuickWebFrame);

  return d->url();
}

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  return qobject_cast<OxideQQuickWebFrame *>(parent());
}

QQmlListProperty<OxideQQuickWebFrame> OxideQQuickWebFrame::childFrames() {
  return QQmlListProperty<OxideQQuickWebFrame>(
      this, NULL,
      OxideQQuickWebFramePrivate::childFrame_count,
      OxideQQuickWebFramePrivate::childFrame_at);
}

QQmlListProperty<OxideQQuickScriptMessageHandler>
OxideQQuickWebFrame::messageHandlers() {
  return QQmlListProperty<OxideQQuickScriptMessageHandler>(
      this, NULL,
      OxideQQuickWebFramePrivate::messageHandler_append,
      OxideQQuickWebFramePrivate::messageHandler_count,
      OxideQQuickWebFramePrivate::messageHandler_at,
      OxideQQuickWebFramePrivate::messageHandler_clear);
}

void OxideQQuickWebFrame::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* handlerp =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (!d->message_handlers().contains(handlerp)) {
    handlerp->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handlerp);

    emit messageHandlersChanged();
  }
}

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* handlerp =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (d->message_handlers().contains(handlerp)) {
    d->message_handlers().removeOne(handlerp);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
}

OxideQQuickScriptMessageRequest* OxideQQuickWebFrame::sendMessage(
    const QUrl& context,
    const QString& msg_id,
    const QVariant& args) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickScriptMessageRequest* request =
      new OxideQQuickScriptMessageRequest();

  if (!d->sendMessage(context, msg_id, args,
                      OxideQQuickScriptMessageRequestPrivate::get(request))) {
    delete request;
    return NULL;
  }

  return request;
}

void OxideQQuickWebFrame::sendMessageNoReply(const QUrl& context,
                                             const QString& msg_id,
                                             const QVariant& args) {
  Q_D(OxideQQuickWebFrame);

  d->sendMessageNoReply(context, msg_id, args);
}
