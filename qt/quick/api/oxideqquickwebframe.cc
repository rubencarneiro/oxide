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

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "qt/core/browser/oxide_qt_web_frame.h"

#include "oxideqquickmessagehandler_p.h"
#include "oxideqquickmessagehandler_p_p.h"
#include "oxideqquickoutgoingmessagerequest_p.h"
#include "oxideqquickoutgoingmessagerequest_p_p.h"

OxideQQuickWebFrame::OxideQQuickWebFrame(oxide::qt::WebFrame* owner) :
    d_ptr(OxideQQuickWebFramePrivate::Create(owner)) {}

void OxideQQuickWebFrame::childEvent(QChildEvent* event) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickWebFrame* child = qobject_cast<OxideQQuickWebFrame *>(event->child());
  if (!child) {
    // We also manage message handlers and outgoing message requests
    return;
  }
  
  if (event->added()) {
    d->children().append(child);
    emit childFrameChanged(ChildAdded, child);
  } else if (event->removed()) {
    d->children().removeOne(child);
    emit childFrameChanged(ChildRemoved, child);
  }
}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {}

QUrl OxideQQuickWebFrame::url() const {
  Q_D(const OxideQQuickWebFrame);

  return QUrl(QString::fromStdString(d->owner()->url().spec()));
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

QQmlListProperty<OxideQQuickMessageHandler>
OxideQQuickWebFrame::messageHandlers() {
  return QQmlListProperty<OxideQQuickMessageHandler>(
      this, NULL,
      OxideQQuickWebFramePrivate::messageHandler_append,
      OxideQQuickWebFramePrivate::messageHandler_count,
      OxideQQuickWebFramePrivate::messageHandler_at,
      OxideQQuickWebFramePrivate::messageHandler_clear);
}

void OxideQQuickWebFrame::addMessageHandler(OxideQQuickMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!d->message_handlers().contains(handler)) {
    OxideQQuickMessageHandlerPrivate::get(handler)->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handler);

    emit messageHandlersChanged();
  }
}

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(OxideQQuickWebFrame);

  if (!d) {
    return;
  }

  if (d->message_handlers().contains(handler)) {
    d->message_handlers().removeOne(handler);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
}

OxideQQuickOutgoingMessageRequest* OxideQQuickWebFrame::sendMessage(
    const QString& world_id,
    const QString& msg_id,
    const QVariant& args) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickOutgoingMessageRequest* request =
      new OxideQQuickOutgoingMessageRequest();

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  if (!d->owner()->SendMessage(
          world_id.toStdString(),
          msg_id.toStdString(),
          QString(jsondoc.toJson()).toStdString(),
          OxideQQuickOutgoingMessageRequestPrivate::get(request)->request())) {
    delete request;
    return NULL;
  }

  d->addOutgoingMessageRequest(request);

  return request;
}

void OxideQQuickWebFrame::sendMessageNoReply(const QString& world_id,
                                             const QString& msg_id,
                                             const QVariant& args) {
  Q_D(OxideQQuickWebFrame);

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  d->owner()->SendMessageNoReply(world_id.toStdString(),
                                 msg_id.toStdString(),
                                 QString(jsondoc.toJson()).toStdString());
}
