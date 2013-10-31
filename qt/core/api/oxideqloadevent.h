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

#ifndef OXIDE_Q_LOAD_EVENT_H
#define OXIDE_Q_LOAD_EVENT_H

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

class OxideQLoadEventPrivate;
class OxideQQuickWebViewPrivate;

class Q_DECL_EXPORT OxideQLoadEvent : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url)
  Q_PROPERTY(Type type READ type)
  Q_PROPERTY(ErrorCode error READ error)
  Q_PROPERTY(QString errorString READ errorString)

  Q_ENUMS(Type)
  Q_ENUMS(ErrorCode)

  Q_DECLARE_PRIVATE(OxideQLoadEvent)

 public:
  virtual ~OxideQLoadEvent();

  enum Type {
    TypeStarted,
    TypeStopped,
    TypeSucceeded,
    TypeFailed
  };

  enum ErrorCode {
    ErrorNone,
    ErrorUnexpected,
    ErrorNameNotResolved,
    ErrorFailed = 1000
  };

  QUrl url() const;
  Type type() const;
  ErrorCode error() const;
  QString errorString() const;

 protected:
  friend class OxideQQuickWebViewPrivate;

  OxideQLoadEvent(const QUrl& url,
                  Type type,
                  int error_code = 0,
                  const QString& error_string = QString());

 private:
  QScopedPointer<OxideQLoadEventPrivate> d_ptr;
};

#endif // OXIDE_Q_LOAD_EVENT_H
