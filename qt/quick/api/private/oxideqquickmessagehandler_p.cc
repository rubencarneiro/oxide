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

#include "qt/quick/api/oxideqquickmessagehandler_p.h"
#include "qt/quick/api/oxideqquickmessagehandler_p_p.h"

#include <QQmlEngine>

#include "base/bind.h"

#include "shared/browser/oxide_incoming_message.h"
#include "shared/browser/oxide_message_handler.h"

#include "qt/core/browser/oxide_qt_web_frame.h"

#include "qt/core/api/oxideqincomingmessage.h"

#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

OxideQQuickMessageHandlerPrivate::OxideQQuickMessageHandlerPrivate(
    OxideQQuickMessageHandler* q) :
    weak_factory_(this),
    q_ptr(q) {}

void OxideQQuickMessageHandlerPrivate::ReceiveMessageCallback(
    oxide::IncomingMessage* message,
    bool* delivered,
    bool* error,
    std::string* error_desc) {

  *delivered = true;
  *error = true;

  QJSValueList args;
  args.append(callback.engine()->newQObject(new OxideQIncomingMessage(message)));
  args.append(callback.engine()->newQObject(
      qobject_cast<OxideQQuickWebFrame *>(
        static_cast<oxide::qt::WebFrame *>(message->frame())->q_web_frame)));

  QJSValue rv = callback.call(args);
  if (rv.isError()) {
    *error_desc = rv.toString().toStdString();
    return;
  }
}

// static
OxideQQuickMessageHandlerPrivate* OxideQQuickMessageHandlerPrivate::Create(
    OxideQQuickMessageHandler* q) {
  return new OxideQQuickMessageHandlerPrivate(q);
}

void OxideQQuickMessageHandlerPrivate::attachHandler() {
  handler_.SetCallback(
      base::Bind(&OxideQQuickMessageHandlerPrivate::ReceiveMessageCallback,
      weak_factory_.GetWeakPtr()));
}

void OxideQQuickMessageHandlerPrivate::disconnectHandler() {
  handler_.SetCallback(oxide::MessageHandler::HandlerCallback());
}

void OxideQQuickMessageHandlerPrivate::removeFromCurrentOwner() {
  Q_Q(OxideQQuickMessageHandler);

  // XXX: Is there a better way of doing this? Perhaps by notifying
  //      the existing owner that the handler has a new parent?
  if (OxideQQuickWebFrame* frame =
      qobject_cast<OxideQQuickWebFrame *>(q->parent())) {
    frame->removeMessageHandler(q);
  } else if (OxideQQuickWebView* view =
             qobject_cast<OxideQQuickWebView *>(q->parent())) {
    view->removeMessageHandler(q);
  }
}

// static
OxideQQuickMessageHandlerPrivate* OxideQQuickMessageHandlerPrivate::get(
    OxideQQuickMessageHandler* message_handler) {
  return message_handler->d_func();
}
