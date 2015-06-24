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

#ifndef _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_H_
#define _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class OxideQQuickScriptMessagePrivate;
class OxideQQuickWebFrame;

class Q_DECL_EXPORT OxideQQuickScriptMessage : public QObject {
  Q_OBJECT
  Q_PROPERTY(OxideQQuickWebFrame* frame READ frame CONSTANT)
  Q_PROPERTY(QUrl context READ context CONSTANT)
  Q_PROPERTY(QString id READ msgId CONSTANT)
  Q_PROPERTY(QVariant args READ payload CONSTANT)
  Q_PROPERTY(QVariant payload READ payload CONSTANT)

  Q_DECLARE_PRIVATE(OxideQQuickScriptMessage)
  Q_DISABLE_COPY(OxideQQuickScriptMessage)

 public:
  virtual ~OxideQQuickScriptMessage();

  OxideQQuickWebFrame* frame() const;
  QUrl context() const;
  QString msgId() const;
  QVariant payload() const;

  Q_INVOKABLE void reply(const QVariant& payload);
  Q_INVOKABLE void error(const QVariant& payload);

 private:
  OxideQQuickScriptMessage();

  QScopedPointer<OxideQQuickScriptMessagePrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_P_H_
