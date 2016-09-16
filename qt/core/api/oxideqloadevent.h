// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef OXIDE_QTCORE_LOAD_EVENT
#define OXIDE_QTCORE_LOAD_EVENT

#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>

#include <OxideQtCore/oxideqglobal.h>

class OxideQLoadEventData;

class OXIDE_QTCORE_EXPORT OxideQLoadEvent {
  Q_GADGET
  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(Type type READ type CONSTANT)

  Q_PROPERTY(ErrorDomain errorDomain READ errorDomain CONSTANT)
  Q_PROPERTY(QString errorString READ errorString CONSTANT)
  Q_PROPERTY(int errorCode READ errorCode CONSTANT)

  Q_PROPERTY(int httpStatusCode READ httpStatusCode CONSTANT)

  Q_PROPERTY(QUrl originalUrl READ originalUrl CONSTANT)

  Q_PROPERTY(bool isError READ isError CONSTANT)

  Q_ENUMS(Type)
  Q_ENUMS(ErrorDomain)

 public:

  enum Type {
    TypeStarted,
    TypeStopped,
    TypeSucceeded,
    TypeFailed,
    TypeCommitted,
    TypeRedirected
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

  Q_DECL_HIDDEN static OxideQLoadEvent createStarted(const QUrl& url);
  Q_DECL_HIDDEN static OxideQLoadEvent createStopped(const QUrl& url);
  Q_DECL_HIDDEN static OxideQLoadEvent createSucceeded(const QUrl& url,
                                                       int http_status_code);
  Q_DECL_HIDDEN static OxideQLoadEvent createFailed(
      const QUrl& url,
      ErrorDomain error_domain,
      const QString& error_string,
      int error_code,
      int http_status_code);
  Q_DECL_HIDDEN static OxideQLoadEvent createCommitted(const QUrl& url,
                                                       bool is_error,
                                                       int http_status_code);
  Q_DECL_HIDDEN static OxideQLoadEvent createRedirected(
      const QUrl& url,
      const QUrl& original_url,
      int http_status_code);

  OxideQLoadEvent();
  OxideQLoadEvent(const OxideQLoadEvent& other);
  ~OxideQLoadEvent();

  OxideQLoadEvent operator=(const OxideQLoadEvent& other);
  bool operator==(const OxideQLoadEvent& other) const;
  bool operator!=(const OxideQLoadEvent& other) const;

  QUrl url() const;
  Type type() const;
  ErrorDomain errorDomain() const;
  QString errorString() const;
  int errorCode() const;
  int httpStatusCode() const;
  QUrl originalUrl() const;
  bool isError() const;

 private:
  Q_DECL_HIDDEN OxideQLoadEvent(
      const QSharedDataPointer<OxideQLoadEventData>& dd);

  QSharedDataPointer<OxideQLoadEventData> d;
};

Q_DECLARE_METATYPE(OxideQLoadEvent)

#endif // OXIDE_QTCORE_LOAD_EVENT
