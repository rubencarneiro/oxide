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

#include "oxide_qquick_outgoing_message_request_p.h"
#include "oxide_qt_qoutgoing_message_request.h"
#include "oxide_qt_qoutgoing_message_request_p.h"

#include <QString>
#include <QtDebug>

#include "base/bind.h"

#include "oxide_qt_qweb_frame_p.h"

namespace oxide {
namespace qt {

void QOutgoingMessageRequestPrivate::ReceiveReplyCallback(
    const std::string& args) {
  OnReceiveReply(QString::fromStdString(args));
}

void QOutgoingMessageRequestPrivate::ReceiveErrorCallback(
    const std::string& msg) {
  OnReceiveError(QString::fromStdString(msg));
}

QOutgoingMessageRequestPrivate::~QOutgoingMessageRequestPrivate() {
  Q_Q(QOutgoingMessageRequest);

  if (frame_) {
    frame_->removeOutgoingMessageRequest(q);
    Q_ASSERT(!frame_);
  }
}

void QOutgoingMessageRequestPrivate::setFramePrivate(QWebFramePrivate* frame) {
  Q_Q(QOutgoingMessageRequest);

  if (frame_) {
    frame_->removeOutgoingMessageRequest(q);
    Q_ASSERT(!frame_);
  }

  frame_ = frame;
}

// static
QOutgoingMessageRequestPrivate* QOutgoingMessageRequestPrivate::get(
    QOutgoingMessageRequest* request) {
  return request->d_func();
}

QOutgoingMessageRequestPrivate::QOutgoingMessageRequestPrivate(
    QOutgoingMessageRequest* q) :
    q_ptr(q),
    frame_(NULL),
    weak_factory_(this) {
  request_.SetReplyCallback(
      base::Bind(&QOutgoingMessageRequestPrivate::ReceiveReplyCallback,
                 weak_factory_.GetWeakPtr()));
  request_.SetErrorCallback(
      base::Bind(&QOutgoingMessageRequestPrivate::ReceiveErrorCallback,
                 weak_factory_.GetWeakPtr()));
}

QOutgoingMessageRequest::QOutgoingMessageRequest(
    QOutgoingMessageRequestPrivate& dd) :
    d_ptr(&dd) {}

QOutgoingMessageRequest::~QOutgoingMessageRequest() {}

} // namespace qt
} // namespace oxide

class OxideQQuickOutgoingMessageRequestPrivate :
    public oxide::qt::QOutgoingMessageRequestPrivate {
 public:
  OxideQQuickOutgoingMessageRequestPrivate(
      OxideQQuickOutgoingMessageRequest* q) :
        oxide::qt::QOutgoingMessageRequestPrivate(q) {}

  QJSValue reply_callback_;
  QJSValue error_callback_;

 private:
  void OnReceiveReply(const QString& args);
  void OnReceiveError(const QString& msg);
};

void OxideQQuickOutgoingMessageRequestPrivate::OnReceiveReply(
    const QString& args) {
  QJSValueList jsargs;
  jsargs.append(QJSValue(args));

  reply_callback_.call(jsargs);
}

void OxideQQuickOutgoingMessageRequestPrivate::OnReceiveError(
    const QString& msg) {
  QJSValueList jsargs;
  jsargs.append(QJSValue(msg));

  error_callback_.call(jsargs);
}

OxideQQuickOutgoingMessageRequest::OxideQQuickOutgoingMessageRequest() :
    oxide::qt::QOutgoingMessageRequest(
      *new OxideQQuickOutgoingMessageRequestPrivate(this)) {}

OxideQQuickOutgoingMessageRequest::~OxideQQuickOutgoingMessageRequest() {}

QJSValue OxideQQuickOutgoingMessageRequest::replyCallback() const {
  Q_D(const OxideQQuickOutgoingMessageRequest);

  return d->reply_callback_;
}

void OxideQQuickOutgoingMessageRequest::setReplyCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickOutgoingMessageRequest);

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
  Q_D(const OxideQQuickOutgoingMessageRequest);

  return d->error_callback_;
}

void OxideQQuickOutgoingMessageRequest::setErrorCallback(
    const QJSValue& callback) {
  Q_D(OxideQQuickOutgoingMessageRequest);

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
