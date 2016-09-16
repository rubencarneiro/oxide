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

#include "oxideqquickscriptmessagehandler.h"
#include "oxideqquickscriptmessagehandler_p.h"

#include <QQmlEngine>
#include <QtDebug>

#include "qt/core/glue/oxide_qt_script_message_handler_proxy.h"

#include "oxideqquickscriptmessage.h"
#include "oxideqquickscriptmessage_p.h"

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
    OxideQQuickScriptMessageHandler* q)
    : q_ptr(q),
      proxy_(oxide::qt::ScriptMessageHandlerProxy::create(this, q)) {}

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

/*!
\class OxideQQuickScriptMessageHandler
\inmodule OxideQtQuick
\inheaderfile oxideqquickscriptmessagehandler.h

\brief A handler for messages from user scripts
*/

/*!
\qmltype ScriptMessageHandler
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickScriptMessageHandler

\brief A handler for messages from user scripts

ScriptMessageHandler is a handler for JS messages sent from user scripts. The
handler will intercept messages with the specified msgId, from JS contexts with
URLs listed in \l{contexts}.

Incoming messages will be passed to the application provided \l{callback}.
*/

void OxideQQuickScriptMessageHandler::classBegin() {}

void OxideQQuickScriptMessageHandler::componentComplete() {
  Q_D(OxideQQuickScriptMessageHandler);

  if (d->isActive()) {
    QMetaObject::invokeMethod(parent(), "addMessageHandler",
                              Qt::DirectConnection,
                              Q_ARG(OxideQQuickScriptMessageHandler*, this));
  }
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

/*!
\qmlproperty string ScriptMessageHandler::msgId

Specify the ID of the messages to handle.
*/

QString OxideQQuickScriptMessageHandler::msgId() const {
  Q_D(const OxideQQuickScriptMessageHandler);

  return d->proxy_->msgId();
}

void OxideQQuickScriptMessageHandler::setMsgId(const QString& id) {
  Q_D(OxideQQuickScriptMessageHandler);

  if (id == msgId()) {
    return;
  }

  d->proxy_->setMsgId(id);
  emit msgIdChanged();
}

/*!
\qmlproperty list<url> ScriptMessageHandler::contexts

Specify a list of JS contexts from which to handle messages.
*/

QList<QUrl> OxideQQuickScriptMessageHandler::contexts() const {
  Q_D(const OxideQQuickScriptMessageHandler);

  return d->proxy_->contexts();
}

void OxideQQuickScriptMessageHandler::setContexts(
    const QList<QUrl>& contexts) {
  Q_D(OxideQQuickScriptMessageHandler);

  d->proxy_->setContexts(contexts);
  emit contextsChanged();
}

/*!
\qmlproperty value ScriptMessageHandler::callback

Specify a JS callback that will be called when an incoming message is received.
The callback will be called with a single argument - a ScriptMessage instance
whose ownership will be transferred to the callback.
*/

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
    d->proxy_->detachHandler();
  } else {
    d->proxy_->attachHandler();
  }

  emit callbackChanged();
}
