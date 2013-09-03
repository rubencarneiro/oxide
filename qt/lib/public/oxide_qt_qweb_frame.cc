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

#include "oxide_qt_qweb_frame.h"
#include "oxide_qt_qweb_frame_p.h"
#include "oxide_qquick_web_frame_p.h"

#include <QString>

#include "qt/lib/browser/oxide_qt_web_frame.h"

#include "oxide_qquick_message_handler_p.h"
#include "oxide_qquick_outgoing_message_request_p.h"
#include "oxide_qt_qmessage_handler_p.h"
#include "oxide_qt_qoutgoing_message_request_p.h"

namespace oxide {
namespace qt {

QWebFramePrivate::QWebFramePrivate(WebFrame* owner) :
    owner_(owner) {}

QWebFramePrivate::~QWebFramePrivate() {}

QWebFramePrivate* QWebFramePrivate::get(QWebFrame* frame) {
  return frame->d_func();
}

void QWebFramePrivate::addOutgoingMessageRequest(
    QOutgoingMessageRequest* request) {
  Q_ASSERT(!outgoing_message_requests_.contains(request));

  QOutgoingMessageRequestPrivate::get(request)->setFramePrivate(this);
  outgoing_message_requests_.append(request);  
}

void QWebFramePrivate::removeOutgoingMessageRequest(
    QOutgoingMessageRequest* request) {
  outgoing_message_requests_.removeOne(request);
  QOutgoingMessageRequestPrivate::get(request)->setFramePrivate(NULL);
}

QWebFrame::QWebFrame(QWebFramePrivate& dd) :
    QObject(),
    d_ptr(&dd) {}

QWebFrame::~QWebFrame() {
  Q_D(QWebFrame);

  for (int i = 0; i < d->outgoing_message_requests().size(); ++i) {
    d->removeOutgoingMessageRequest(d->outgoing_message_requests().at(i));    
  }
}

QUrl QWebFrame::url() const {
  Q_D(const QWebFrame);

  return QUrl(QString::fromStdString(d->owner()->url().spec()));
}

void QWebFrame::addMessageHandler(QMessageHandler* handler) {
  Q_D(QWebFrame);

  if (!d->message_handlers().contains(handler)) {
    QMessageHandlerPrivate::get(handler)->removeFromCurrentOwner();
    handler->setParent(this);

    d->message_handlers().append(handler);

    emit messageHandlersChanged();
  }
}

void QWebFrame::removeMessageHandler(QMessageHandler* handler) {
  Q_D(QWebFrame);

  if (d->message_handlers().contains(handler)) {
    d->message_handlers().removeOne(handler);
    handler->setParent(NULL);

    emit messageHandlersChanged();
  }
}

} // namespace qt
} // namespace oxide

class OxideQQuickWebFramePrivate : public oxide::qt::QWebFramePrivate {
 public:
  OxideQQuickWebFramePrivate(oxide::qt::WebFrameQQuick* owner) :
      oxide::qt::QWebFramePrivate(owner) {}

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
int OxideQQuickWebFramePrivate::childFrame_count(
    QQmlListProperty<OxideQQuickWebFrame>* prop) {
  oxide::qt::QWebFramePrivate* p =
      oxide::qt::QWebFramePrivate::get(
        static_cast<oxide::qt::QWebFrame *>(prop->object));

  if (p->owner()->ChildCount() > INT_MAX) {
    qWarning() << "Number of child frames exceed maximum";
    return INT_MAX;
  }

  return static_cast<int>(p->owner()->ChildCount());
}

// static
OxideQQuickWebFrame* OxideQQuickWebFramePrivate::childFrame_at(
    QQmlListProperty<OxideQQuickWebFrame>* prop,
    int index) {
  oxide::qt::QWebFramePrivate* p =
      oxide::qt::QWebFramePrivate::get(
        static_cast<oxide::qt::QWebFrame *>(prop->object));

  return static_cast<oxide::qt::WebFrameQQuick *>(
      p->owner()->ChildAt(index))->QQuickWebFrame();
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
  oxide::qt::QWebFramePrivate* p =
      oxide::qt::QWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return p->message_handlers().size();
}

// static
OxideQQuickMessageHandler* OxideQQuickWebFramePrivate::messageHandler_at(
    QQmlListProperty<OxideQQuickMessageHandler>* prop,
    int index) {
  oxide::qt::QWebFramePrivate* p =
      oxide::qt::QWebFramePrivate::get(
        static_cast<OxideQQuickWebFrame *>(prop->object));

  return qobject_cast<OxideQQuickMessageHandler *>(
      p->message_handlers().at(index));
}

// static
void OxideQQuickWebFramePrivate::messageHandler_clear(
    QQmlListProperty<OxideQQuickMessageHandler>* prop) {
  OxideQQuickWebFrame* frame =
      static_cast<OxideQQuickWebFrame *>(prop->object);
  oxide::qt::QWebFramePrivate* p = oxide::qt::QWebFramePrivate::get(frame);

  while (p->message_handlers().size() > 0) {
    oxide::qt::QMessageHandler* handler = p->message_handlers().first();
    p->message_handlers().removeFirst();
    delete handler;
  }

  emit frame->messageHandlersChanged();
}

OxideQQuickWebFrame::OxideQQuickWebFrame(oxide::qt::WebFrameQQuick* owner) :
    oxide::qt::QWebFrame(*new OxideQQuickWebFramePrivate(owner)) {}

OxideQQuickWebFrame::~OxideQQuickWebFrame() {}

OxideQQuickWebFrame* OxideQQuickWebFrame::parentFrame() const {
  Q_D(const OxideQQuickWebFrame);

  return static_cast<oxide::qt::WebFrameQQuick *>(
      d->owner()->parent())->QQuickWebFrame();
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

OxideQQuickOutgoingMessageRequest* OxideQQuickWebFrame::sendMessage(
    const QString& world_id,
    const QString& msg_id,
    const QString& args) {
  Q_D(OxideQQuickWebFrame);

  OxideQQuickOutgoingMessageRequest* request =
      new OxideQQuickOutgoingMessageRequest();

  if (!d->owner()->SendMessage(
          world_id.toStdString(),
          msg_id.toStdString(),
          args.toStdString(),
          oxide::qt::QOutgoingMessageRequestPrivate::get(request)->request())) {
    delete request;
    return NULL;
  }

  d->addOutgoingMessageRequest(request);

  return request;
}

void OxideQQuickWebFrame::sendMessageNoReply(const QString& world_id,
                                             const QString& msg_id,
                                             const QString& args) {
  Q_D(OxideQQuickWebFrame);

  d->owner()->SendMessageNoReply(world_id.toStdString(),
                                 msg_id.toStdString(),
                                 args.toStdString());
}
