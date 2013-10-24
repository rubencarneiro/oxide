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

#include "qt/core/api/oxideqincomingmessage.h"

#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

OxideQQuickMessageHandlerPrivate::OxideQQuickMessageHandlerPrivate(
    OxideQQuickMessageHandler* q) :
    q_ptr(q) {}

bool OxideQQuickMessageHandlerPrivate::OnReceiveMessage(
    OxideQIncomingMessage* message,
    OxideQQuickWebFrame* frame,
    QString& error) {
  QJSValueList args;
  args.append(callback.engine()->newQObject(message));
  args.append(callback.engine()->newQObject(frame));

  QJSValue rv = callback.call(args);
  if (rv.isError()) {
    error = rv.toString();
    return false;
  }

  return true;
}

// static
OxideQQuickMessageHandlerPrivate* OxideQQuickMessageHandlerPrivate::Create(
    OxideQQuickMessageHandler* q) {
  return new OxideQQuickMessageHandlerPrivate(q);
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
