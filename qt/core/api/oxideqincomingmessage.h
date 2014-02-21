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

#ifndef OXIDE_Q_INCOMING_MESSAGE_H
#define OXIDE_Q_INCOMING_MESSAGE_H

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

class OxideQIncomingMessagePrivate;

class Q_DECL_EXPORT OxideQIncomingMessage : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl context READ context)
  Q_PROPERTY(QVariant args READ args)

  Q_DECLARE_PRIVATE(OxideQIncomingMessage)
  Q_DISABLE_COPY(OxideQIncomingMessage)

 public:
  Q_DECL_HIDDEN OxideQIncomingMessage();
  virtual ~OxideQIncomingMessage();

  QUrl context() const;
  QVariant args() const;

  Q_INVOKABLE void reply(const QVariant& args);
  Q_INVOKABLE void error(const QString& msg);

 private:
  QScopedPointer<OxideQIncomingMessagePrivate> d_ptr;
};

#endif // OXIDE_Q_INCOMING_MESSAGE_H
