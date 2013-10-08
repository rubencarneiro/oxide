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

#include "oxide_q_web_frame_base.h"
#include "oxide_qquick_web_frame_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "qt/lib/api/private/oxide_qt_qmessage_handler_p.h"
#include "qt/lib/api/private/oxide_qt_qoutgoing_message_request_p.h"
#include "qt/lib/api/private/oxide_qt_qweb_frame_p.h"
#include "qt/lib/browser/oxide_qt_web_frame.h"

#include "oxide_qquick_outgoing_message_request_p.h"

OxideQWebFrameBase::OxideQWebFrameBase(oxide::qt::QWebFrameBasePrivate& dd) :
    QObject(),
    d_ptr(&dd) {}

OxideQWebFrameBase::~OxideQWebFrameBase() {}

QUrl OxideQWebFrameBase::url() const {
  Q_D(const oxide::qt::QWebFrameBase);

  return QUrl(QString::fromStdString(d->owner()->url().spec()));
}

void OxideQQuickWebFrame::childEvent(QChildEvent* event) {
  OxideQQuickWebFrame* child = qobject_cast<OxideQQuickWebFrame *>(event->child());
  Q_ASSERT(child);

  if (event->added()) {
    emit childFrameChanged(ChildAdded, child);
  } else if (event->removed()) {
    emit childFrameChanged(ChildRemoved, child);
  }
}

OxideQQuickWebFrame::OxideQQuickWebFrame(oxide::qt::WebFrameQQuick* owner) :
    OxideQWebFrameBase(*oxide::qt::QQuickWebFramePrivate::Create(owner)) {}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {}

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  return qobject_cast<OxideQQuickWebFrame *>(parent());
}

QQmlListProperty<OxideQQuickWebFrame> OxideQQuickWebFrame::childFrames() {
  return QQmlListProperty<OxideQQuickWebFrame>(
      this, NULL,
      oxide::qt::QQuickWebFramePrivate::childFrame_count,
      oxide::qt::QQuickWebFramePrivate::childFrame_at);
}

QQmlListProperty<OxideQQuickMessageHandler>
OxideQQuickWebFrame::messageHandlers() {
  return QQmlListProperty<OxideQQuickMessageHandler>(
      this, NULL,
      oxide::qt::QQuickWebFramePrivate::messageHandler_append,
      oxide::qt::QQuickWebFramePrivate::messageHandler_count,
      oxide::qt::QQuickWebFramePrivate::messageHandler_at,
      oxide::qt::QQuickWebFramePrivate::messageHandler_clear);
}

void OxideQQuickWebFrame::addMessageHandler(OxideQQuickMessageHandler* handler) {
  Q_D(oxide::qt::QQuickWebFrame);

  if (!d->message_handlers().contains(handler)) {
    oxide::qt::QQuickMessageHandlerPrivate::get(handler)->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handler);

    emit messageHandlersChanged();
  }
}

void OxideQQuickWebFrame::removeMessageHandler(
    OxideQQuickMessageHandler* handler) {
  Q_D(oxide::qt::QQuickWebFrame);

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
  Q_D(oxide::qt::QQuickWebFrame);

  OxideQQuickOutgoingMessageRequest* request =
      new OxideQQuickOutgoingMessageRequest();

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  if (!d->owner()->SendMessage(
          world_id.toStdString(),
          msg_id.toStdString(),
          QString(jsondoc.toJson()).toStdString(),
          oxide::qt::QOutgoingMessageRequestBasePrivate::get(request)->request())) {
    delete request;
    return NULL;
  }

  d->addOutgoingMessageRequest(request);

  return request;
}

void OxideQQuickWebFrame::sendMessageNoReply(const QString& world_id,
                                             const QString& msg_id,
                                             const QVariant& args) {
  Q_D(oxide::qt::QQuickWebFrame);

  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  d->owner()->SendMessageNoReply(world_id.toStdString(),
                                 msg_id.toStdString(),
                                 QString(jsondoc.toJson()).toStdString());
}
