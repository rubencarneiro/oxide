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

#include "oxideqquickscriptmessagerequest.h"
#include "oxideqquickscriptmessagerequest_p.h"

#include <QJSEngine>
#include <QString>
#include <QtDebug>
#include <QVariant>

#include "qt/core/glue/oxide_qt_script_message_request_proxy.h"

void OxideQQuickScriptMessageRequestPrivate::ReceiveReply(
    const QVariant& payload) {
  if (!reply_callback.engine()) {
    return;
  }

  QJSValueList jsargs;
  jsargs.append(reply_callback.engine()->toScriptValue(payload));

  reply_callback.call(jsargs);
}

void OxideQQuickScriptMessageRequestPrivate::ReceiveError(
    int error,
    const QVariant& payload) {
  if (!error_callback.engine()) {
    return;
  }

  QJSValueList jsargs;
  jsargs.append(QJSValue(error));
  jsargs.append(error_callback.engine()->toScriptValue(payload));

  error_callback.call(jsargs);
}

OxideQQuickScriptMessageRequestPrivate::OxideQQuickScriptMessageRequestPrivate(
    OxideQQuickScriptMessageRequest* q)
    : q_ptr(q),
      proxy_(oxide::qt::ScriptMessageRequestProxy::create(this, q)) {}

// static
OxideQQuickScriptMessageRequestPrivate* OxideQQuickScriptMessageRequestPrivate::get(
    OxideQQuickScriptMessageRequest* request) {
  return request->d_func();
}

/*!
\class OxideQQuickScriptMessageRequest
\inmodule OxideQtQuick
\inheaderfile oxideqquickscriptmessagerequest.h

\brief The context of a message sent to a user script
*/

/*!
\qmltype ScriptMessageRequest
\inqmlmodule com.canonical.Oxide 1.0
\instantiates OxideQQuickScriptMessageRequest

\brief The context of a message sent to a user script

ScriptMessageRequest represents the context of a message sent to a user script.
The application can provide reply and error callbacks by setting the \l{onreply}
and \l{onerror} callbacks accordingly.
*/

OxideQQuickScriptMessageRequest::OxideQQuickScriptMessageRequest() :
    d_ptr(new OxideQQuickScriptMessageRequestPrivate(this)) {}

OxideQQuickScriptMessageRequest::~OxideQQuickScriptMessageRequest() {}

/*!
\qmlproperty value ScriptMessageRequest::onreply

The reply callback. This will be invoked with a single argument (the message
payload, which is application defined) when the user script replies to the
message that the application sent it.
*/

QJSValue OxideQQuickScriptMessageRequest::replyCallback() const {
  Q_D(const OxideQQuickScriptMessageRequest);

  return d->reply_callback;
}

void OxideQQuickScriptMessageRequest::setReplyCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickScriptMessageRequest);

  if (d->reply_callback.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable() && !callback.isNull() && !callback.isUndefined()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->reply_callback = callback;
  emit replyCallbackChanged();
}

/*!
\qmlproperty value ScriptMessageRequest::onerror

The error callback. This will be invoked with 2 arguments in the event of an
error occuring with the message that the application sent.

The first argument will be an error code, with the following possible values:

\value ScriptMessageRequest.ErrorInvalidContext
The frame that the message was sent to does not have a JS context with the
specified ID.

\value ScriptMessageRequest.ErrorUncaughtException
The message handler provided by the user script threw an exception. Details of
the exception will be provided in the second argument.

\value ScriptMessageRequest.ErrorNoHandler
There was no handler for the message.

\value ScriptMessageRequest.ErrorHandlerReportedError
Use of this is discouraged now.

\value ScriptMessageRequest.ErrorHandlerDidNotRespond
The message was successfully delivered to a handler provided by the user script,
but it failed to respond.
*/

QJSValue OxideQQuickScriptMessageRequest::errorCallback() const {
  Q_D(const OxideQQuickScriptMessageRequest);

  return d->error_callback;
}

void OxideQQuickScriptMessageRequest::setErrorCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickScriptMessageRequest);

  if (d->error_callback.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable() && !callback.isNull() && !callback.isUndefined()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->error_callback = callback;
  emit errorCallbackChanged();
}
