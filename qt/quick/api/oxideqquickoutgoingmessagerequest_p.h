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

#ifndef _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_H_
#define _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_H_

#include <QJSValue>
#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QtQml>

class OxideQQuickOutgoingMessageRequestPrivate;

class Q_DECL_EXPORT OxideQQuickOutgoingMessageRequest : public QObject {
  Q_OBJECT
  Q_PROPERTY(QJSValue onreply READ replyCallback WRITE setReplyCallback NOTIFY replyCallbackChanged)
  Q_PROPERTY(QJSValue onerror READ errorCallback WRITE setErrorCallback NOTIFY errorCallbackChanged)
  Q_ENUMS(ErrorCode)

  Q_DECLARE_PRIVATE(OxideQQuickOutgoingMessageRequest)

 public:
  virtual ~OxideQQuickOutgoingMessageRequest();

  enum ErrorCode {
    ErrorNone,
    ErrorDestinationNotFound,
    ErrorUncaughtException,
    ErrorNoHandler,
    ErrorHandlerReportedError,
    ErrorFrameDisappeared
  };

  QJSValue replyCallback() const;
  void setReplyCallback(const QJSValue& callback);

  QJSValue errorCallback() const;
  void setErrorCallback(const QJSValue& callback);

 Q_SIGNALS:
  void replyCallbackChanged();
  void errorCallbackChanged();

 protected:
  friend class OxideQQuickWebFrame;

  Q_DECL_HIDDEN OxideQQuickOutgoingMessageRequest();

 private:
  QScopedPointer<OxideQQuickOutgoingMessageRequestPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickOutgoingMessageRequest);

#endif // _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_H_
