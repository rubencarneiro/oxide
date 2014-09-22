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

class Q_DECL_EXPORT OxideQLoadEvent : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(Type type READ type CONSTANT)
  Q_PROPERTY(ErrorDomain errorDomain READ errorDomain CONSTANT)
  Q_PROPERTY(QString errorString READ errorString CONSTANT)
  Q_PROPERTY(int errorCode READ errorCode CONSTANT)

  Q_ENUMS(Type)
  Q_ENUMS(ErrorDomain)

  Q_DECLARE_PRIVATE(OxideQLoadEvent)
  Q_DISABLE_COPY(OxideQLoadEvent)

 public:

  enum Type {
    TypeStarted,
    TypeStopped,
    TypeSucceeded,
    TypeFailed,
    TypeCommitted
  };

  enum ErrorDomain {
    ErrorDomainNone,
    ErrorDomainInternal,
    ErrorDomainConnection,
    ErrorDomainCertificate,
    ErrorDomainHTTP,
    ErrorDomainCache,
    ErrorDomainFTP,
    ErrorDomainDNS
  };

  Q_DECL_HIDDEN OxideQLoadEvent(const QUrl& url,
                                Type type,
                                ErrorDomain error_domain = ErrorDomainNone,
                                const QString& error_string = QString(),
                                int error_code = 0);
  virtual ~OxideQLoadEvent();

  QUrl url() const;
  Type type() const;
  ErrorDomain errorDomain() const;
  QString errorString() const;
  int errorCode() const;

 private:
  QScopedPointer<OxideQLoadEventPrivate> d_ptr;
};

#endif // OXIDE_Q_LOAD_EVENT_H
