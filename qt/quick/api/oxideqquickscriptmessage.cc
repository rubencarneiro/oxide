// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxideqquickscriptmessage.h"
#include "oxideqquickscriptmessage_p.h"

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/glue/oxide_qt_script_message_proxy.h"

#include "oxideqquickwebframe.h"
#include "oxideqquickwebframe_p.h"

OxideQQuickScriptMessagePrivate::OxideQQuickScriptMessagePrivate(
    oxide::qt::ScriptMessageProxy* proxy,
    OxideQQuickScriptMessage* q)
    : q_ptr(q),
      proxy_(proxy) {}

// static
OxideQQuickScriptMessage* OxideQQuickScriptMessagePrivate::create(
    oxide::qt::ScriptMessageProxy* proxy) {
  OxideQQuickScriptMessage* message = new OxideQQuickScriptMessage();
  message->d_ptr.reset(new OxideQQuickScriptMessagePrivate(proxy, message));
  return message;
}

// static
OxideQQuickScriptMessagePrivate* OxideQQuickScriptMessagePrivate::get(
    OxideQQuickScriptMessage* q) {
  return q->d_func();
}

/*!
\class OxideQQuickScriptMessage
\inmodule OxideQtQuick
\inheaderfile oxideqquickscriptmessage.h

\brief An incoming JS message from a user script
*/

/*!
\qmltype ScriptMessage
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickScriptMessage

\brief A JS message sent from a user script

ScriptMessage represents a JS message sent from a user script. It provides
various details about the message such as the source WebFrame (\l{frame}), the
ID of the source JS context (\l{context}) and the message ID (msgId).

The message payload is provided by \l{payload}.

The application can choose to reply to this message by calling \l{reply}.
*/

OxideQQuickScriptMessage::OxideQQuickScriptMessage() {}

OxideQQuickScriptMessage::~OxideQQuickScriptMessage() {}

/*!
\qmlproperty WebFrame ScriptMessage::frame

The WebFrame representing the frame that this message was sent from.
*/

OxideQQuickWebFrame* OxideQQuickScriptMessage::frame() const {
  Q_D(const OxideQQuickScriptMessage);

  QObject* f = d->proxy_->frame();
  if (!f) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebFrame*>(f);
}

/*!
\qmlproperty url ScriptMessage::context

The ID of the JS context that this message was sent from.
*/

QUrl OxideQQuickScriptMessage::context() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->context();
}

/*!
\qmlproperty string ScriptMessage::msgId

The ID of this message.
*/

QString OxideQQuickScriptMessage::msgId() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->msgId();
}

/*!
\qmlproperty variant ScriptMessage::args
\deprecated
*/

QVariant OxideQQuickScriptMessage::args() const {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickScriptMessage: args is deprecated. Please use payload "
      "instead";

  return payload();
}

/*!
\qmlproperty variant ScriptMessage::payload
\since OxideQt 1.9

The message payload. The format is defined by the application.
*/

QVariant OxideQQuickScriptMessage::payload() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->payload();
}

/*!
\qmlmethod void ScriptMessage::reply(variant payload)

Reply to this message, responding with the provided \a{payload}. \a{payload}
must be a value, object or array that can be represented by JSON values.

This is ignored if the message isn't expecting a reply.
*/

void OxideQQuickScriptMessage::reply(const QVariant& payload) {
  Q_D(OxideQQuickScriptMessage);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy_->reply(aux);
}

/*!
\qmlmethod void ScriptMessage::error(variant payload)
\deprecated
*/

void OxideQQuickScriptMessage::error(const QVariant& payload) {
  Q_D(OxideQQuickScriptMessage);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy_->error(aux);
}
