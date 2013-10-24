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

#ifndef _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_H_
#define _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_H_

#include <QJSValue>
#include <QList>
#include <QObject>
#include <QQmlParserStatus>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QtQml>

class OxideQQuickMessageHandlerPrivate;

class Q_DECL_EXPORT OxideQQuickMessageHandler : public QObject,
                                                public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QString msgId READ msgId WRITE setMsgId NOTIFY msgIdChanged)
  Q_PROPERTY(QList<QString> worldIds READ worldIds WRITE setWorldIds NOTIFY worldIdsChanged)
  Q_PROPERTY(QJSValue callback READ callback WRITE setCallback NOTIFY callbackChanged)

  Q_DECLARE_PRIVATE(OxideQQuickMessageHandler)

 public:
  OxideQQuickMessageHandler(QObject* parent = NULL);
  virtual ~OxideQQuickMessageHandler();

  QString msgId() const;
  void setMsgId(const QString& id);

  QList<QString> worldIds() const;
  void setWorldIds(const QList<QString>& ids);

  QJSValue callback() const;
  void setCallback(const QJSValue& callback);

  void classBegin();
  void componentComplete();

 Q_SIGNALS:
  void msgIdChanged();
  void worldIdsChanged();
  void callbackChanged();

 private:
  QScopedPointer<OxideQQuickMessageHandlerPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickMessageHandler)

#endif // _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_H_
