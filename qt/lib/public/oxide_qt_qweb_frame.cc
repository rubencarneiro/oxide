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
#include "oxide_q_web_frame_base_p.h"
#include "oxide_qquick_web_frame_p.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QString>

#include "qt/lib/browser/oxide_qt_web_frame.h"

#include "oxide_q_message_handler_base_p.h"
#include "oxide_q_outgoing_message_request_base_p.h"
#include "oxide_qquick_message_handler_p.h"
#include "oxide_qquick_outgoing_message_request_p.h"

namespace oxide {
namespace qt {

QWebFrameBasePrivate::QWebFrameBasePrivate(WebFrame* owner) :
    owner_(owner) {}

QWebFrameBasePrivate::~QWebFrameBasePrivate() {}

QWebFrameBasePrivate* QWebFrameBasePrivate::get(OxideQWebFrameBase* frame) {
  return frame->d_func();
}

void QWebFrameBasePrivate::addOutgoingMessageRequest(
    OxideQOutgoingMessageRequestBase* request) {
  Q_ASSERT(!outgoing_message_requests_.contains(request));

  QOutgoingMessageRequestBasePrivate::get(request)->setFramePrivate(this);
  outgoing_message_requests_.append(request);  
}

void QWebFrameBasePrivate::removeOutgoingMessageRequest(
    OxideQOutgoingMessageRequestBase* request) {
  outgoing_message_requests_.removeOne(request);
  QOutgoingMessageRequestBasePrivate::get(request)->setFramePrivate(NULL);
}

class QQuickWebFramePrivate : public QWebFrameBasePrivate {
 public:
  QQuickWebFramePrivate(WebFrameQQuick* owner) :
      QWebFrameBasePrivate(owner) {}

  static int childFrame_count(QQmlListProperty<OxideQQuickWebFrame>* prop);
  static OxideQQuickWebFrame* childFrame_at(
      QQmlListProperty<OxideQQuickWebFrame>* prop, int index);

  static void messageHandler_append(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      OxideQQuickMessageHandler* value);
  static int messageHandler_count(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);
  static OxideQQuickMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      int index);
  static void messageHandler_clear(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);
};

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

OxideQWebFrameBase::OxideQWebFrameBase(oxide::qt::QWebFrameBasePrivate& dd) :
    QObject(),
    d_ptr(&dd) {}

OxideQWebFrameBase::~OxideQWebFrameBase() {
  Q_D(oxide::qt::QWebFrameBase);

  for (int i = 0; i < d->outgoing_message_requests().size(); ++i) {
    d->removeOutgoingMessageRequest(d->outgoing_message_requests().at(i));
  }
}

QUrl OxideQWebFrameBase::url() const {
  Q_D(const oxide::qt::QWebFrameBase);

  return QUrl(QString::fromStdString(d->owner()->url().spec()));
}

void OxideQWebFrameBase::addMessageHandler(OxideQMessageHandlerBase* handler) {
  Q_D(oxide::qt::QWebFrameBase);

  if (!d->message_handlers().contains(handler)) {
    oxide::qt::QMessageHandlerBasePrivate::get(handler)->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handler);

    emit messageHandlersChanged();
  }
}

void OxideQWebFrameBase::removeMessageHandler(
    OxideQMessageHandlerBase* handler) {
  Q_D(oxide::qt::QWebFrameBase);

  if (d->message_handlers().contains(handler)) {
    d->message_handlers().removeOne(handler);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
}

OxideQQuickWebFrame::OxideQQuickWebFrame(oxide::qt::WebFrameQQuick* owner) :
    OxideQWebFrameBase(*new oxide::qt::QQuickWebFramePrivate(owner)) {}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {}

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  Q_D(const oxide::qt::QQuickWebFrame);

  return static_cast<oxide::qt::WebFrameQQuick *>(
      d->owner()->parent())->QQuickWebFrame();
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
