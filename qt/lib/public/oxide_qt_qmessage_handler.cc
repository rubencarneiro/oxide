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

#include "oxide_qt_qmessage_handler.h"
#include "oxide_qt_qmessage_handler_p.h"
#include "oxide_qquick_message_handler_p.h"

#include <QQmlEngine>
#include <QtDebug>

#include "base/bind.h"

#include "shared/browser/oxide_incoming_message.h"
#include "shared/browser/oxide_message_handler.h"

#include "qt/lib/browser/oxide_qt_web_frame.h"

#include "oxide_q_incoming_message.h"
#include "oxide_qquick_web_frame_p.h"
#include "oxide_qquick_web_view_p.h"
#include "oxide_qt_qweb_frame.h"

namespace oxide {
namespace qt {

void QMessageHandlerPrivate::ReceiveMessageCallback(
    oxide::IncomingMessage* message) {
  OnReceiveMessage(
      new OxideQIncomingMessage(message),
      static_cast<WebFrame *>(message->frame())->q_web_frame());
}

QMessageHandlerPrivate::QMessageHandlerPrivate(QMessageHandler* q) :
    q_ptr(q),
    weak_factory_(this) {
  handler_.SetCallback(
      base::Bind(&QMessageHandlerPrivate::ReceiveMessageCallback,
      weak_factory_.GetWeakPtr()));
}

QMessageHandlerPrivate::~QMessageHandlerPrivate() {}

// static
QMessageHandlerPrivate* QMessageHandlerPrivate::get(
    QMessageHandler* message_handler) {
  return message_handler->d_func();
}

void QMessageHandlerPrivate::removeFromCurrentOwner() {
  Q_Q(QMessageHandler);

  // XXX: Is there a better way of doing this? Perhaps by notifying
  //      the existing owner that the handler has a new parent?
  if (QWebFrame* frame = qobject_cast<QWebFrame *>(q->parent())) {
    frame->removeMessageHandler(q);
  } else if (OxideQQuickWebView* view =
             qobject_cast<OxideQQuickWebView *>(q->parent())) {
    view->removeMessageHandler(qobject_cast<OxideQQuickMessageHandler *>(q));
  }
}

QMessageHandler::QMessageHandler(QMessageHandlerPrivate& dd,
                                 QObject* parent) :
    QObject(parent),
    d_ptr(&dd) {}

QMessageHandler::~QMessageHandler() {}

QString QMessageHandler::msgId() const {
  Q_D(const QMessageHandler);

  return QString::fromStdString(d->handler()->msg_id());
}

void QMessageHandler::setMsgId(const QString& id) {
  Q_D(QMessageHandler);

  if (id.toStdString() == d->handler()->msg_id()) {
    return;
  }

  d->handler()->set_msg_id(id.toStdString());
  emit msgIdChanged();
}

} // namespace qt
} // namespace oxide

class OxideQQuickMessageHandlerPrivate :
    public oxide::qt::QMessageHandlerPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickMessageHandler)

 public:
  OxideQQuickMessageHandlerPrivate(OxideQQuickMessageHandler* q) :
      oxide::qt::QMessageHandlerPrivate(q) {}

  QJSValue callback_;

 private:
  void OnReceiveMessage(OxideQIncomingMessage* message,
                        oxide::qt::QWebFrame* frame) FINAL;
};

void OxideQQuickMessageHandlerPrivate::OnReceiveMessage(
    OxideQIncomingMessage* message,
    oxide::qt::QWebFrame* frame) {
  QQmlEngine::setObjectOwnership(message, QQmlEngine::JavaScriptOwnership);

  QJSValueList args;
  args.append(callback_.engine()->newQObject(message));
  args.append(callback_.engine()->newQObject(
      qobject_cast<OxideQQuickWebFrame *>(frame)));

  callback_.call(args);
}

OxideQQuickMessageHandler::OxideQQuickMessageHandler(QObject* parent) :
    oxide::qt::QMessageHandler(*new OxideQQuickMessageHandlerPrivate(this),
                               parent) {}

OxideQQuickMessageHandler::~OxideQQuickMessageHandler() {}

QJSValue OxideQQuickMessageHandler::callback() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->callback_;
}

void OxideQQuickMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(OxideQQuickMessageHandler);

  if (callback.strictlyEquals(d->callback_)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback_ = callback;
  emit callbackChanged();
}
