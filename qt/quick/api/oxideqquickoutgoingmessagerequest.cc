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

#include "oxideqquickoutgoingmessagerequest_p.h"
#include "oxideqquickoutgoingmessagerequest_p_p.h"

#include <QtDebug>

OxideQQuickOutgoingMessageRequest::OxideQQuickOutgoingMessageRequest() :
    QObject(),
    d_ptr(OxideQQuickOutgoingMessageRequestPrivate::Create(this)) {}

OxideQQuickOutgoingMessageRequest::~OxideQQuickOutgoingMessageRequest() {
  Q_D(OxideQQuickOutgoingMessageRequest);

  d->removeFromOwner();
}

QJSValue OxideQQuickOutgoingMessageRequest::replyCallback() const {
  Q_D(const OxideQQuickOutgoingMessageRequest);

  return d->reply_callback;
}

void OxideQQuickOutgoingMessageRequest::setReplyCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickOutgoingMessageRequest);

  if (d->reply_callback.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->reply_callback = callback;
  emit replyCallbackChanged();
}

QJSValue OxideQQuickOutgoingMessageRequest::errorCallback() const {
  Q_D(const OxideQQuickOutgoingMessageRequest);

  return d->error_callback;
}

void OxideQQuickOutgoingMessageRequest::setErrorCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickOutgoingMessageRequest);

  if (d->error_callback.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->error_callback = callback;
  emit errorCallbackChanged();
}
