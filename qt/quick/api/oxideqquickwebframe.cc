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

void OxideQQuickWebFramePrivate::URLCommitted() {
  Q_Q(OxideQQuickWebFrame);

  emit q->urlChanged();
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
int OxideQQuickWebFramePrivate::messageHandler_count(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->messageHandlers().size();
}

// static
OxideQQuickScriptMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
    int index) {
  OxideQQuickWebFramePrivate* p = OxideQQuickWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return adapterToQObject<OxideQQuickScriptMessageHandler>(
      p->messageHandlers().at(index));
}

OxideQQuickWebFramePrivate* OxideQQuickWebFramePrivate::get(
    OxideQQuickWebFrame* frame) {
  return frame->d_func();
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

  while (d->messageHandlers().size() > 0) {
    delete adapterToQObject<OxideQQuickScriptMessageHandler>(
        d->messageHandlers().at(0));
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
      OxideQQuickWebFramePrivate::messageHandler_count,
      OxideQQuickWebFramePrivate::messageHandler_at);
}

void OxideQQuickWebFrame::addMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (hd->isActive() && handler->parent() != this) {
    qWarning() << "MessageHandler can't be added to more than one message target";
    return;
  }

  if (d->messageHandlers().contains(hd)) {
    d->messageHandlers().removeOne(hd);
  }

  handler->setParent(this);
  d->messageHandlers().append(hd);

  emit messageHandlersChanged();
}

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickScriptMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!handler) {
    qWarning() << "Didn't specify a handler";
    return;
  }

  OxideQQuickScriptMessageHandlerPrivate* hd =
      OxideQQuickScriptMessageHandlerPrivate::get(handler);

  if (!d->messageHandlers().contains(hd)) {
    return;
  }

  handler->setParent(NULL);
  d->messageHandlers().removeOne(hd);

  emit messageHandlersChanged();
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
