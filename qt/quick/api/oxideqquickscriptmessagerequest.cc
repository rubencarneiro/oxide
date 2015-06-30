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

#include "oxideqquickscriptmessagerequest_p.h"
#include "oxideqquickscriptmessagerequest_p_p.h"

#include <QJSEngine>
#include <QString>
#include <QtDebug>
#include <QVariant>

OXIDE_Q_IMPL_PROXY_HANDLE_CONVERTER(OxideQQuickScriptMessageRequest,
                                    oxide::qt::ScriptMessageRequestProxyHandle);

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
    : oxide::qt::ScriptMessageRequestProxyHandle(
        oxide::qt::ScriptMessageRequestProxy::create(this), q) {}

// static
OxideQQuickScriptMessageRequestPrivate* OxideQQuickScriptMessageRequestPrivate::get(
    OxideQQuickScriptMessageRequest* request) {
  return request->d_func();
}

OxideQQuickScriptMessageRequest::OxideQQuickScriptMessageRequest() :
    d_ptr(new OxideQQuickScriptMessageRequestPrivate(this)) {}

OxideQQuickScriptMessageRequest::~OxideQQuickScriptMessageRequest() {}

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
