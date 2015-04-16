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
  Q_PROPERTY(HttpStatusCode httpStatusCode READ httpStatusCode CONSTANT)

  Q_PROPERTY(QUrl originalUrl READ originalUrl CONSTANT)

  Q_PROPERTY(bool isError READ isError CONSTANT REVISION 1)

  Q_ENUMS(Type)
  Q_ENUMS(ErrorDomain)
  Q_ENUMS(HttpStatusCode)

  Q_DECLARE_PRIVATE(OxideQLoadEvent)
  Q_DISABLE_COPY(OxideQLoadEvent)

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

  enum HttpStatusCode {
    HttpStatusCodeContinue,
    HttpStatusCodeSwitchingProtocols,
    HttpStatusCodeOK,
    HttpStatusCodeCreated,
    HttpStatusCodeAccepted,
    HttpStatusCodeNonAuthoritativeInformation,
    HttpStatusCodeNoContent,
    HttpStatusCodeResetContent,
    HttpStatusCodePartialContent,
    HttpStatusCodeMultipleChoices,
    HttpStatusCodeMovedPermanently,
    HttpStatusCodeFound,
    HttpStatusCodeSeeOther,
    HttpStatusCodeNotModified,
    HttpStatusCodeUseProxy,
    HttpStatusCodeTemporaryRedirect,
    HttpStatusCodeBadRequest,
    HttpStatusCodeUnauthorized,
    HttpStatusCodePaymentRequired,
    HttpStatusCodeForbidden,
    HttpStatusCodeNotFound,
    HttpStatusCodeMethodNotAllowed,
    HttpStatusCodeNotAcceptable,
    HttpStatusCodeProxyAuthenticationRequired,
    HttpStatusCodeRequestTimeOut,
    HttpStatusCodeConflict,
    HttpStatusCodeGone,
    HttpStatusCodeLengthRequired,
    HttpStatusCodePreconditionFailed,
    HttpStatusCodeRequestEntityTooLarge,
    HttpStatusCodeRequestURITooLarge,
    HttpStatusCodeUnsupportedMediaType,
    HttpStatusCodeRequestedRangeNotSatisfiable,
    HttpStatusCodeExpectationFailed,
    HttpStatusCodeInternalServerError,
    HttpStatusCodeNotImplemented,
    HttpStatusCodeBadGateway,
    HttpStatusCodeServiceUnavailable,
    HttpStatusCodeGatewayTimeOut,
    HttpStatusCodeHTTPVersionNotSupported,
    HttpStatusCodeUnknown
  };

  Q_DECL_HIDDEN OxideQLoadEvent(const QUrl& url,
                                Type type,
                                bool is_error = false,
                                HttpStatusCode http_status_code = HttpStatusCodeOK);
  Q_DECL_HIDDEN OxideQLoadEvent(const QUrl& url,
                                ErrorDomain error_domain,
                                const QString& error_string,
                                int error_code);
  Q_DECL_HIDDEN OxideQLoadEvent(const QUrl& url,
                                const QUrl& original_url);
  virtual ~OxideQLoadEvent();

  QUrl url() const;
  Type type() const;
  ErrorDomain errorDomain() const;
  QString errorString() const;
  int errorCode() const;
  HttpStatusCode httpStatusCode() const;
  QUrl originalUrl() const;
  bool isError() const;

 private:
  QScopedPointer<OxideQLoadEventPrivate> d_ptr;
};

#endif // OXIDE_Q_LOAD_EVENT_H
