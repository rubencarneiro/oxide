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

#include "oxideqquickscriptmessagehandler_p.h"
#include "oxideqquickscriptmessagehandler_p_p.h"

#include <QQmlEngine>
#include <QtDebug>

#include "oxideqquickscriptmessage_p.h"
#include "oxideqquickscriptmessage_p_p.h"

OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(OxideQQuickScriptMessageHandler,
                                    oxide::qt::ScriptMessageHandlerProxyHandle);

bool OxideQQuickScriptMessageHandlerPrivate::ReceiveMessage(
    oxide::qt::ScriptMessageProxy* message,
    QVariant* error) {
  QJSValueList args;
  args.append(callback_.engine()->newQObject(
      OxideQQuickScriptMessagePrivate::create(message)));

  QJSValue rv = callback_.call(args);
  if (rv.isError()) {
    *error = QVariant(rv.toString());
    return false;
  }

  return true;
}

OxideQQuickScriptMessageHandlerPrivate::OxideQQuickScriptMessageHandlerPrivate(
    OxideQQuickScriptMessageHandler* q) :
    oxide::qt::ScriptMessageHandlerProxyHandle(
      oxide::qt::ScriptMessageHandlerProxy::create(this), q) {}

bool OxideQQuickScriptMessageHandlerPrivate::isActive() {
  Q_Q(OxideQQuickScriptMessageHandler);

  if (!q->parent()) {
    return false;
  }

  if (q->parent()->inherits("OxideQQuickWebFrame") ||
      q->parent()->inherits("OxideQQuickWebView")) {
    return true;
  }

  return false;
}

// static
OxideQQuickScriptMessageHandlerPrivate*
OxideQQuickScriptMessageHandlerPrivate::get(
    OxideQQuickScriptMessageHandler* message_handler) {
  return message_handler->d_func();
}

OxideQQuickScriptMessageHandler::OxideQQuickScriptMessageHandler(
    QObject* parent)
    : QObject(parent),
      d_ptr(new OxideQQuickScriptMessageHandlerPrivate(this)) {}

OxideQQuickScriptMessageHandler::~OxideQQuickScriptMessageHandler() {
  Q_D(OxideQQuickScriptMessageHandler);

  if (d->isActive()) {
    bool res = QMetaObject::invokeMethod(
        parent(), "removeMessageHandler",
        Qt::DirectConnection,
        Q_ARG(OxideQQuickScriptMessageHandler*, this));
    Q_ASSERT(res);
  }
}

QString OxideQQuickScriptMessageHandler::msgId() const {
  Q_D(const OxideQQuickScriptMessageHandler);

  return d->proxy()->msgId();
}

void OxideQQuickScriptMessageHandler::setMsgId(const QString& id) {
  Q_D(OxideQQuickScriptMessageHandler);

  if (id == msgId()) {
    return;
  }

  d->proxy()->setMsgId(id);
  emit msgIdChanged();
}

QList<QUrl> OxideQQuickScriptMessageHandler::contexts() const {
  Q_D(const OxideQQuickScriptMessageHandler);

  return d->proxy()->contexts();
}

void OxideQQuickScriptMessageHandler::setContexts(
    const QList<QUrl>& contexts) {
  Q_D(OxideQQuickScriptMessageHandler);

  d->proxy()->setContexts(contexts);
  emit contextsChanged();
}

QJSValue OxideQQuickScriptMessageHandler::callback() const {
  Q_D(const OxideQQuickScriptMessageHandler);

  return d->callback_;
}

void OxideQQuickScriptMessageHandler::setCallback(const QJSValue& callback) {
  Q_D(OxideQQuickScriptMessageHandler);

  if (callback.strictlyEquals(d->callback_)) {
    return;
  }

  bool is_null = callback.isNull() || callback.isUndefined();

  if (!callback.isCallable() && !is_null) {
    qWarning() << "Invalid callback";
    return;
  }

  d->callback_ = callback;

  if (is_null) {
    d->proxy()->detachHandler();
  } else {
    d->proxy()->attachHandler();
  }

  emit callbackChanged();
}

void OxideQQuickScriptMessageHandler::classBegin() {}

void OxideQQuickScriptMessageHandler::componentComplete() {
  Q_D(OxideQQuickScriptMessageHandler);

  if (d->isActive()) {
    QMetaObject::invokeMethod(parent(), "addMessageHandler",
                              Qt::DirectConnection,
                              Q_ARG(OxideQQuickScriptMessageHandler*, this));
  }
}
