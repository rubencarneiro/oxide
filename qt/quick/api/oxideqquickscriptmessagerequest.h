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

#ifndef OXIDE_QTQUICK_SCRIPT_MESSAGE_REQUEST
#define OXIDE_QTQUICK_SCRIPT_MESSAGE_REQUEST

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>
#include <QtQml/QJSValue>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickScriptMessageRequestPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickScriptMessageRequest : public QObject {
  Q_OBJECT
  Q_PROPERTY(QJSValue onreply READ replyCallback WRITE setReplyCallback NOTIFY replyCallbackChanged)
  Q_PROPERTY(QJSValue onerror READ errorCallback WRITE setErrorCallback NOTIFY errorCallbackChanged)
  Q_ENUMS(ErrorCode)

  Q_DECLARE_PRIVATE(OxideQQuickScriptMessageRequest)

 public:
  ~OxideQQuickScriptMessageRequest() Q_DECL_OVERRIDE;

  enum ErrorCode {
    ErrorNone,
    ErrorInvalidContext,
    ErrorUncaughtException,
    ErrorNoHandler,
    ErrorHandlerReportedError,
    ErrorHandlerDidNotRespond,

    ErrorDestinationNotFound = ErrorInvalidContext
  };

  QJSValue replyCallback() const;
  void setReplyCallback(const QJSValue& callback);

  QJSValue errorCallback() const;
  void setErrorCallback(const QJSValue& callback);

 Q_SIGNALS:
  void replyCallbackChanged();
  void errorCallbackChanged();

 private:
  friend class OxideQQuickWebFrame;
  Q_DECL_HIDDEN OxideQQuickScriptMessageRequest();

  QScopedPointer<OxideQQuickScriptMessageRequestPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickScriptMessageRequest);

#endif // OXIDE_QTQUICK_SCRIPT_MESSAGE_REQUEST
