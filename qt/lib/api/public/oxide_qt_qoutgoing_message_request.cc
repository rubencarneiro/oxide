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

#include "oxide_q_outgoing_message_request_base.h"
#include "oxide_qquick_outgoing_message_request_p.h"

#include <QtDebug>

#include "qt/lib/api/oxide_qt_qoutgoing_message_request_p.h"

OxideQOutgoingMessageRequestBase::OxideQOutgoingMessageRequestBase(
    oxide::qt::QOutgoingMessageRequestBasePrivate& dd) :
    d_ptr(&dd) {}

OxideQOutgoingMessageRequestBase::~OxideQOutgoingMessageRequestBase() {}

OxideQQuickOutgoingMessageRequest::OxideQQuickOutgoingMessageRequest() :
    OxideQOutgoingMessageRequestBase(
      *oxide::qt::QQuickOutgoingMessageRequestPrivate::Create(this)) {}

OxideQQuickOutgoingMessageRequest::~OxideQQuickOutgoingMessageRequest() {}

QJSValue OxideQQuickOutgoingMessageRequest::replyCallback() const {
  Q_D(const oxide::qt::QQuickOutgoingMessageRequest);

  return d->reply_callback_;
}

void OxideQQuickOutgoingMessageRequest::setReplyCallback(
    const QJSValue& callback) {
  Q_D(oxide::qt::QQuickOutgoingMessageRequest);

  if (d->reply_callback_.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->reply_callback_ = callback;
  emit replyCallbackChanged();
}

QJSValue OxideQQuickOutgoingMessageRequest::errorCallback() const {
  Q_D(const oxide::qt::QQuickOutgoingMessageRequest);

  return d->error_callback_;
}

void OxideQQuickOutgoingMessageRequest::setErrorCallback(
    const QJSValue& callback) {
  Q_D(oxide::qt::QQuickOutgoingMessageRequest);

  if (d->error_callback_.strictlyEquals(callback)) {
    return;
  }

  if (!callback.isCallable()) {
    qWarning() << "Invalid callback";
    return;
  }

  d->error_callback_ = callback;
  emit errorCallbackChanged();
}
