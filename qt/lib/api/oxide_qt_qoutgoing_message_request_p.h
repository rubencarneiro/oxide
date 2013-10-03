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

#ifndef _OXIDE_QT_LIB_PUBLIC_QOUTGOING_MESSAGE_REQUEST_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QOUTGOING_MESSAGE_REQUEST_P_H_

#include <QJSValue>
#include <QtGlobal>
#include <string>

#include "base/basictypes.h"

#include "shared/browser/oxide_outgoing_message_request.h"

#include "qt/lib/api/public/oxide_q_outgoing_message_request_base.h"

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

class OxideQQuickOutgoingMessageRequest;

namespace oxide {
namespace qt {

class QWebFrameBasePrivate;

class QOutgoingMessageRequestBasePrivate {
  Q_DECLARE_PUBLIC(OxideQOutgoingMessageRequestBase)

 public:
  virtual ~QOutgoingMessageRequestBasePrivate();

  oxide::OutgoingMessageRequest* request() {
    return &request_;
  }

  void setFramePrivate(QWebFrameBasePrivate* frame);

  static QOutgoingMessageRequestBasePrivate* get(
      OxideQOutgoingMessageRequestBase* request);

 protected:
  QOutgoingMessageRequestBasePrivate(OxideQOutgoingMessageRequestBase* q);

  OxideQOutgoingMessageRequestBase* q_ptr;

 private:
  void ReceiveReplyCallback(const std::string& args);
  void ReceiveErrorCallback(int error, const std::string& msg);

  virtual void OnReceiveReply(const QVariant& args) = 0;
  virtual void OnReceiveError(
      OxideQOutgoingMessageRequestBase::ErrorCode error,
      const QString& msg) = 0;

  oxide::OutgoingMessageRequest request_;
  QWebFrameBasePrivate* frame_;
  base::WeakPtrFactory<QOutgoingMessageRequestBasePrivate> weak_factory_;
};

class QQuickOutgoingMessageRequestPrivate :
    public QOutgoingMessageRequestBasePrivate {
 public:
  static QQuickOutgoingMessageRequestPrivate* Create(
      OxideQQuickOutgoingMessageRequest* q);

  QJSValue reply_callback;
  QJSValue error_callback;

 private:
  QQuickOutgoingMessageRequestPrivate(OxideQQuickOutgoingMessageRequest* q);

  void OnReceiveReply(const QVariant& args);
  void OnReceiveError(
      OxideQOutgoingMessageRequestBase::ErrorCode error,
      const QString& msg);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_QOUTGOING_MESSAGE_REQUEST_P_H_
