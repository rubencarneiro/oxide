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

#ifndef OXIDE_QTQUICK_SCRIPT_MESSAGE_HANDLER
#define OXIDE_QTQUICK_SCRIPT_MESSAGE_HANDLER

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <QtQml/QJSValue>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickScriptMessageHandlerPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickScriptMessageHandler
    : public QObject,
      public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QString msgId READ msgId WRITE setMsgId NOTIFY msgIdChanged)
  Q_PROPERTY(QList<QUrl> contexts READ contexts WRITE setContexts NOTIFY contextsChanged)
  Q_PROPERTY(QJSValue callback READ callback WRITE setCallback NOTIFY callbackChanged)

  Q_DECLARE_PRIVATE(OxideQQuickScriptMessageHandler)

 public:
  OxideQQuickScriptMessageHandler(QObject* parent = nullptr);
  ~OxideQQuickScriptMessageHandler() Q_DECL_OVERRIDE;

  QString msgId() const;
  void setMsgId(const QString& id);

  QList<QUrl> contexts() const;
  void setContexts(const QList<QUrl>& contexts);

  QJSValue callback() const;
  void setCallback(const QJSValue& callback);

 Q_SIGNALS:
  void msgIdChanged();
  void contextsChanged();
  void callbackChanged();

 protected:
  // QQmlParserStatus implementation
  void classBegin() Q_DECL_OVERRIDE;
  void componentComplete() Q_DECL_OVERRIDE;

 private:
  QScopedPointer<OxideQQuickScriptMessageHandlerPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickScriptMessageHandler)

#endif // OXIDE_QTQUICK_SCRIPT_MESSAGE_HANDLER
