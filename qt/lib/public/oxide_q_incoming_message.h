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

#ifndef _OXIDE_QT_LIB_PUBLIC_Q_INCOMING_MESSAGE_H_
#define _OXIDE_QT_LIB_PUBLIC_Q_INCOMING_MESSAGE_H_

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QtQml>

namespace oxide {
class IncomingMessage;
namespace qt {
class QIncomingMessagePrivate;
class QMessageHandlerBasePrivate;
}
}

class Q_DECL_EXPORT OxideQIncomingMessage : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString worldId READ worldId)
  Q_PROPERTY(QString args READ args)

  Q_DECLARE_PRIVATE(oxide::qt::QIncomingMessage)

 public:
  virtual ~OxideQIncomingMessage();

  QString worldId() const;
  QString args() const;

  Q_INVOKABLE void reply(const QString& args);
  Q_INVOKABLE void error(const QString& msg);

 protected:
  friend class oxide::qt::QMessageHandlerBasePrivate;

  OxideQIncomingMessage(oxide::IncomingMessage* message);

 private:
  QScopedPointer<oxide::qt::QIncomingMessagePrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQIncomingMessage)

#endif // _OXIDE_QT_LIB_PUBLIC_Q_INCOMING_MESSAGE_H_
