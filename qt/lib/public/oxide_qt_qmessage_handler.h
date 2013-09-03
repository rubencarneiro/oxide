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

#ifndef _OXIDE_QT_LIB_PUBLIC_QMESSAGE_HANDLER_H_
#define _OXIDE_QT_LIB_PUBLIC_QMESSAGE_HANDLER_H_

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

namespace oxide {
namespace qt {

class QMessageHandlerPrivate;

class Q_DECL_EXPORT QMessageHandler : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString msgId READ msgId WRITE setMsgId NOTIFY msgIdChanged)

  Q_DECLARE_PRIVATE(QMessageHandler)

 public:
  virtual ~QMessageHandler();

  QString msgId() const;
  void setMsgId(const QString& id);

 Q_SIGNALS:
  void msgIdChanged();

 protected:
  QMessageHandler(QMessageHandlerPrivate& dd,
                  QObject* parent = NULL);

  QScopedPointer<QMessageHandlerPrivate> d_ptr;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_QMESSAGE_HANDLER_H_
