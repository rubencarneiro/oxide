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

#include "qt/quick/api/oxideqquickoutgoingmessagerequest_p_p.h"
#include "qt/quick/api/oxideqquickoutgoingmessagerequest_p.h"

#include <QByteArray>
#include <QJSEngine>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "base/bind.h"

#include "qt/quick/api/oxideqquickwebframe_p_p.h"

namespace oxide {
namespace qt {

QQuickOutgoingMessageRequestPrivate::QQuickOutgoingMessageRequestPrivate(
    OxideQQuickOutgoingMessageRequest* q) :
    frame(NULL),
    weak_factory_(this),
    q_ptr(q) {
  request_.SetReplyCallback(
      base::Bind(&QQuickOutgoingMessageRequestPrivate::ReceiveReplyCallback,
                 weak_factory_.GetWeakPtr()));
  request_.SetErrorCallback(
      base::Bind(&QQuickOutgoingMessageRequestPrivate::ReceiveErrorCallback,
                 weak_factory_.GetWeakPtr()));
}

void QQuickOutgoingMessageRequestPrivate::ReceiveReplyCallback(
    const std::string& args) {
  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(args.data(), args.length())));

  QJSValueList jsargs;
  jsargs.append(reply_callback.engine()->toScriptValue(jsondoc.toVariant()));

  reply_callback.call(jsargs);

  removeFromOwner();
}

void QQuickOutgoingMessageRequestPrivate::ReceiveErrorCallback(
    int error,
    const std::string& msg) {
  QJSValueList jsargs;
  jsargs.append(QJSValue(error));
  jsargs.append(QJSValue(QString::fromStdString(msg)));

  error_callback.call(jsargs);

  removeFromOwner();
}

void QQuickOutgoingMessageRequestPrivate::removeFromOwner() {
  Q_Q(OxideQQuickOutgoingMessageRequest);

  if (frame) {
    frame->removeOutgoingMessageRequest(q);
    frame = NULL;
  }
}

QQuickOutgoingMessageRequestPrivate::~QQuickOutgoingMessageRequestPrivate() {
  removeFromOwner();
}

// static
QQuickOutgoingMessageRequestPrivate* QQuickOutgoingMessageRequestPrivate::Create(
    OxideQQuickOutgoingMessageRequest* q) {
  return new QQuickOutgoingMessageRequestPrivate(q);
}

// static
QQuickOutgoingMessageRequestPrivate* QQuickOutgoingMessageRequestPrivate::get(
    OxideQQuickOutgoingMessageRequest* request) {
  return request->d_func();
}

} // namespace qt
} // namespace oxide
