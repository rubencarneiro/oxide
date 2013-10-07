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

#include "oxide_qt_qmessage_handler_p.h"

#include <QQmlEngine>

#include "base/bind.h"

#include "shared/browser/oxide_incoming_message.h"
#include "shared/browser/oxide_message_handler.h"

#include "qt/lib/api/public/oxide_q_incoming_message.h"
#include "qt/lib/api/public/oxide_q_web_frame_base.h"
#include "qt/lib/api/public/oxide_qquick_web_frame_p.h"
#include "qt/lib/api/public/oxide_qquick_web_view_p.h"
#include "qt/lib/browser/oxide_qt_web_frame.h"

namespace oxide {
namespace qt {

void QMessageHandlerBasePrivate::ReceiveMessageCallback(
    oxide::IncomingMessage* message,
    bool* delivered,
    bool* error,
    std::string* error_desc) {

  *delivered = true;

  QString qerror_desc;
  *error = !OnReceiveMessage(
      new OxideQIncomingMessage(message),
      static_cast<WebFrame *>(message->frame())->q_web_frame,
      &qerror_desc);

  *error_desc = qerror_desc.toStdString();
}

QMessageHandlerBasePrivate::QMessageHandlerBasePrivate(
    OxideQMessageHandlerBase* q) :
    q_ptr(q),
    weak_factory_(this) {
  handler_.SetCallback(
      base::Bind(&QMessageHandlerBasePrivate::ReceiveMessageCallback,
      weak_factory_.GetWeakPtr()));
}

QMessageHandlerBasePrivate::~QMessageHandlerBasePrivate() {}

// static
QMessageHandlerBasePrivate* QMessageHandlerBasePrivate::get(
    OxideQMessageHandlerBase* message_handler) {
  return message_handler->d_func();
}

void QMessageHandlerBasePrivate::removeFromCurrentOwner() {
  Q_Q(OxideQMessageHandlerBase);

  // XXX: Is there a better way of doing this? Perhaps by notifying
  //      the existing owner that the handler has a new parent?
  if (OxideQWebFrameBase* frame =
      qobject_cast<OxideQWebFrameBase *>(q->parent())) {
    frame->removeMessageHandler(q);
  } else if (OxideQQuickWebView* view =
             qobject_cast<OxideQQuickWebView *>(q->parent())) {
    view->removeMessageHandler(qobject_cast<OxideQQuickMessageHandler *>(q));
  }
}

bool QQuickMessageHandlerPrivate::OnReceiveMessage(
    OxideQIncomingMessage* message,
    OxideQWebFrameBase* frame,
    QString* error_desc) {
  QQmlEngine::setObjectOwnership(message, QQmlEngine::JavaScriptOwnership);

  QJSValueList args;
  args.append(callback.engine()->newQObject(message));
  args.append(callback.engine()->newQObject(
      qobject_cast<OxideQQuickWebFrame *>(frame)));

  QJSValue rv = callback.call(args);
  if (rv.isError()) {
    *error_desc = rv.toString();
    return false;
  }

  return true;
}

QQuickMessageHandlerPrivate::QQuickMessageHandlerPrivate(
    OxideQQuickMessageHandler* q) :
    QMessageHandlerBasePrivate(q) {}

// static
QQuickMessageHandlerPrivate* QQuickMessageHandlerPrivate::Create(
    OxideQQuickMessageHandler* q) {
  return new QQuickMessageHandlerPrivate(q);
}

} // namespace qt
} // namespace oxide
