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

#include "oxide_qt_qoutgoing_message_request_p.h"

#include <QByteArray>
#include <QJSEngine>
#include <QJsonDocument>
#include <QString>
#include <QVariant>

#include "base/bind.h"

#include "qt/lib/api/oxide_qquick_outgoing_message_request_p.h"

#include "oxide_qt_qweb_frame_p.h"

namespace oxide {
namespace qt {

void QOutgoingMessageRequestBasePrivate::ReceiveReplyCallback(
    const std::string& args) {
  QJsonDocument jsondoc(QJsonDocument::fromJson(
      QByteArray(args.data(), args.length())));
  OnReceiveReply(jsondoc.toVariant());

  removeFromOwner();
}

void QOutgoingMessageRequestBasePrivate::ReceiveErrorCallback(
    int error,
    const std::string& msg) {
  OnReceiveError(
      static_cast<OxideQOutgoingMessageRequestBase::ErrorCode>(error),
      QString::fromStdString(msg));

  removeFromOwner();
}

void QOutgoingMessageRequestBasePrivate::removeFromOwner() {
  Q_Q(OxideQOutgoingMessageRequestBase);

  if (frame) {
    frame->removeOutgoingMessageRequest(q);
    frame = NULL;
  }
}

QOutgoingMessageRequestBasePrivate::~QOutgoingMessageRequestBasePrivate() {
  removeFromOwner();
}

// static
QOutgoingMessageRequestBasePrivate* QOutgoingMessageRequestBasePrivate::get(
    OxideQOutgoingMessageRequestBase* request) {
  return request->d_func();
}

QOutgoingMessageRequestBasePrivate::QOutgoingMessageRequestBasePrivate(
    OxideQOutgoingMessageRequestBase* q) :
    frame(NULL),
    q_ptr(q),
    weak_factory_(this) {
  request_.SetReplyCallback(
      base::Bind(&QOutgoingMessageRequestBasePrivate::ReceiveReplyCallback,
                 weak_factory_.GetWeakPtr()));
  request_.SetErrorCallback(
      base::Bind(&QOutgoingMessageRequestBasePrivate::ReceiveErrorCallback,
                 weak_factory_.GetWeakPtr()));
}

QQuickOutgoingMessageRequestPrivate::QQuickOutgoingMessageRequestPrivate(
    OxideQQuickOutgoingMessageRequest* q) :
    QOutgoingMessageRequestBasePrivate(q) {}

void QQuickOutgoingMessageRequestPrivate::OnReceiveReply(
    const QVariant& args) {
  QJSValueList jsargs;
  jsargs.append(reply_callback.engine()->toScriptValue(args));

  reply_callback.call(jsargs);
}

void QQuickOutgoingMessageRequestPrivate::OnReceiveError(
    OxideQOutgoingMessageRequestBase::ErrorCode error,
    const QString& msg) {
  QJSValueList jsargs;
  jsargs.append(QJSValue(error));
  jsargs.append(QJSValue(msg));

  error_callback.call(jsargs);
}

// static
QQuickOutgoingMessageRequestPrivate* QQuickOutgoingMessageRequestPrivate::Create(
    OxideQQuickOutgoingMessageRequest* q) {
  return new QQuickOutgoingMessageRequestPrivate(q);
}

} // namespace qt
} // namespace oxide
