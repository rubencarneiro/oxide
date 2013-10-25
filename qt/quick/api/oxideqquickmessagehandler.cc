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

#include "oxideqquickmessagehandler_p.h"
#include "oxideqquickmessagehandler_p_p.h"

#include <QQmlEngine>
#include <QtDebug>

#include "qt/core/api/oxideqincomingmessage.h"

#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebview_p.h"

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

OxideQQuickMessageHandlerPrivate::OxideQQuickMessageHandlerPrivate(
    OxideQQuickMessageHandler* q) :
    q_ptr(q) {}

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

OxideQQuickMessageHandler::OxideQQuickMessageHandler(QObject* parent) :
    QObject(parent),
    d_ptr(new OxideQQuickMessageHandlerPrivate(this)) {}

OxideQQuickMessageHandler::~OxideQQuickMessageHandler() {
  Q_D(OxideQQuickMessageHandler);

  d->removeFromCurrentOwner();
}

QString OxideQQuickMessageHandler::msgId() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->msgId();
}

void OxideQQuickMessageHandler::setMsgId(const QString& id) {
  Q_D(OxideQQuickMessageHandler);

  if (id == d->msgId()) {
    return;
  }

  d->setMsgId(id);
  emit msgIdChanged();
}

QList<QString> OxideQQuickMessageHandler::worldIds() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->worldIds();
}

void OxideQQuickMessageHandler::setWorldIds(const QList<QString>& ids) {
  Q_D(OxideQQuickMessageHandler);

  d->setWorldIds(ids);
  emit worldIdsChanged();
}

QJSValue OxideQQuickMessageHandler::callback() const {
  Q_D(const OxideQQuickMessageHandler);

  return d->callback;
}

void OxideQQuickMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(OxideQQuickMessageHandler);

  if (callback.strictlyEquals(d->callback)) {
    return;
  }

  bool is_null = callback.isNull() || callback.isUndefined();

  if (!callback.isCallable() && !is_null) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback = callback;

  if (is_null) {
    d->detachHandler();
  } else {
    d->attachHandler();
  }

  emit callbackChanged();
}

void OxideQQuickMessageHandler::classBegin() {}

void OxideQQuickMessageHandler::componentComplete() {
  if (OxideQQuickWebView* view = qobject_cast<OxideQQuickWebView *>(parent())) {
    view->addMessageHandler(this);
  } else if (OxideQQuickWebFrame* frame =
             qobject_cast<OxideQQuickWebFrame *>(parent())) {
    frame->addMessageHandler(this);
  }
}
