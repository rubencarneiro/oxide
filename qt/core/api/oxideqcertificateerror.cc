// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxideqcertificateerror.h"
#include "oxideqcertificateerror_p.h"

#include <utility>

#include <QString>
#include <QtDebug>
#include <QUrl>

#include "base/bind.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "net/cert/x509_certificate.h"

#include "shared/browser/ssl/oxide_security_types.h"
#include "shared/browser/ssl/oxide_certificate_error.h"

#include "oxideqsslcertificate_p.h"

namespace {

void SendTestResponse(const std::function<void(bool)>& callback,
                      bool response) {
  callback(response);
}

}

OxideQCertificateErrorPrivate::OxideQCertificateErrorPrivate(
    std::unique_ptr<oxide::CertificateError> error)
    : q_ptr(nullptr),
      certificate_(OxideQSslCertificateData::Create(error->cert())),
      error_(std::move(error)),
      did_respond_(false) {}

void OxideQCertificateErrorPrivate::OnCancel() {
  Q_Q(OxideQCertificateError);

  DCHECK(!did_respond_);

  Q_EMIT q->cancelled();
}

void OxideQCertificateErrorPrivate::respond(bool accept) {
  if (!error_->overridable()) {
    qWarning() << "Cannot respond to a non-overridable request";
    return;
  }

  if (did_respond_) {
    qWarning() << "Cannot respond to a CertificateError more than once";
    return;
  }

  if (error_->IsCancelled()) {
    qWarning() << "Cannot respond to a CertificateError that has been cancelled";
    return;
  }

  did_respond_ = true;
  if (accept) {
    error_->Allow();
  } else {
    error_->Deny();
  }
}

OxideQCertificateErrorPrivate::~OxideQCertificateErrorPrivate() {}

// static
OxideQCertificateErrorPrivate* OxideQCertificateErrorPrivate::get(
    OxideQCertificateError* q) {
  return q->d_func();
}

// static
std::unique_ptr<OxideQCertificateError> OxideQCertificateErrorPrivate::Create(
    std::unique_ptr<oxide::CertificateError> error,
    QObject* parent) {
  return base::WrapUnique(
      new OxideQCertificateError(
          *new OxideQCertificateErrorPrivate(std::move(error)),
          parent));
}

// static
std::unique_ptr<OxideQCertificateError>
OxideQCertificateErrorPrivate::CreateForTesting(
    bool is_main_frame,
    bool is_subresource,
    OxideQCertificateError::Error error,
    const OxideQSslCertificate& cert,
    const QUrl& url,
    bool strict_enforcement,
    bool overridable,
    const std::function<void(bool)>& callback) {
  std::unique_ptr<oxide::CertificateError> cert_error =
      oxide::CertificateError::CreateForTesting(
          is_main_frame,
          is_subresource,
          static_cast<oxide::CertError>(error),
          OxideQSslCertificateData::GetX509Certificate(cert),
          GURL(url.toString().toStdString()),
          strict_enforcement,
          overridable,
          base::Bind(&SendTestResponse, callback));
  return Create(std::move(cert_error));                                                
}

void OxideQCertificateErrorPrivate::SimulateCancel() {
  error_->SimulateCancel();
}

/*!
\class OxideQCertificateError
\inheaderfile oxideqcertificateerror.h
\inmodule OxideQtCore

\brief Represents a certificate error

OxideQCertificateError represents an individual certificate error. This can't be
constructed by applications.

\l{url} indicates the URL of the connection that the error represents, and an
error code is given by certError.

For certificate errors that are overridable (\l{overridable} is true), calling
\l{allow} will tell Oxide to proceed with the connection. Calling \l{deny} will
tell Oxide to abort the connection.

For non-overridable certificate errors (\l{overridable} is false), calling
\l{allow} or \l{deny} will have no effect. The associated connection will
already have been aborted (for main-frame document errors) or failed with an
error (for sub-frame or subresource errors).

For main-frame document errors (where isMainFrame is true and
isSubresource is false), a blank transient page will be loaded for the life
of the error, with the URL pointing to \l{url}. This is to ensure that
navigation actions work correctly. It is assumed that an application wil
display its own error UI over this. The transient page will be destroyed after a
call to \l{allow} or \l{deny} for overridable errors, and will be destroyed if
the error instance is deleted.

Main-frame document errors can also be cancelled by Oxide (eg, if another
navigation is started). In this case, the \l{cancelled} signal will be emitted
and \l{isCancelled} will change to true. The associated transient page will be
destroyed automatically when this happens.
*/

/*!
\enum OxideQCertificateError::Error

\omitvalue OK

\value ErrorBadIdentity
The identity of the certificate does not match the identity of the site.

\value ErrorExpired
The certificate has expired.

\value ErrorDateInvalid
The certificate has a date that is invalid, eg, its start date is in the future.

\value ErrorAuthorityInvalid
The certificate is signed by an authority that isn't trusted.

\value ErrorRevoked
The certificate has been revoked.

\value ErrorInvalid
The certificate is invalid, eg, it has errors.

\value ErrorInsecure
The certificate is insecure, eg, it uses a weak signature algorithm or has a
weak public key.

\value ErrorGeneric
This is used for all other unspecified errors.
*/

OxideQCertificateError::OxideQCertificateError(
    OxideQCertificateErrorPrivate& dd,
    QObject* parent)
    : QObject(parent),
      d_ptr(&dd) {
  Q_D(OxideQCertificateError);

  d->q_ptr = this;

  d->error_->SetCancelCallback(
      base::Bind(&OxideQCertificateErrorPrivate::OnCancel,
                 // The callback cannot run after |d| is deleted, as it
                 // excusively owns |error_|
                 base::Unretained(d)));
  
  static_assert(
      OK == static_cast<Error>(oxide::CERT_OK),
      "Error and oxide::CertError enums dont match: OK");
  static_assert(
      ErrorBadIdentity == static_cast<Error>(oxide::CERT_ERROR_BAD_IDENTITY),
      "Error and oxide::CertError enums dont match: ErrorBadIdentity");
  static_assert(
      ErrorExpired == static_cast<Error>(oxide::CERT_ERROR_EXPIRED),
      "Error and oxide::CertError enums dont match: ErrorExpired");
  static_assert(
      ErrorDateInvalid == static_cast<Error>(oxide::CERT_ERROR_DATE_INVALID),
      "Error and oxide::CertError enums dont match: ErrorDateInvalid");
  static_assert(
      ErrorAuthorityInvalid ==
        static_cast<Error>(oxide::CERT_ERROR_AUTHORITY_INVALID),
      "Error and oxide::CertError enums dont match: ErrorAuthorityInvalid");
  static_assert(
      ErrorRevoked == static_cast<Error>(oxide::CERT_ERROR_REVOKED),
      "Error and oxide::CertError enums dont match: ErrorRevoked");
  static_assert(
      ErrorInvalid == static_cast<Error>(oxide::CERT_ERROR_INVALID),
      "Error and oxide::CertError enums dont match: ErrorInvalid");
  static_assert(
      ErrorInsecure == static_cast<Error>(oxide::CERT_ERROR_INSECURE),
      "Error and oxide::CertError enums dont match: ErrorInsecure");
  static_assert(
      ErrorGeneric == static_cast<Error>(oxide::CERT_ERROR_GENERIC),
      "Error and oxide::CertError enums dont match: ErrorGeneric");
}

/*!
Destroy the certificate error. If this is a main-frame document error
(isMainFrame is true and isSubresource is false), this will destroy the
associated transient page if it still exists. If the error is overridable and
the application hasn't already responded by calling \l{allow} or \l{deny}, then
the connection will be aborted automatically.
*/

OxideQCertificateError::~OxideQCertificateError() {}

/*!
\property OxideQCertificateError::url

The URL associated with the certificate error.
*/

QUrl OxideQCertificateError::url() const {
  Q_D(const OxideQCertificateError);

  return QUrl(QString::fromStdString(d->error_->url().spec()));
}

/*!
\property OxideQCertificateError::isCancelled

Indicates whether this error has been cancelled by Oxide. This could occur if
the application starts another navigation before responding to this error. If
the application is displaying an error UI, it should hide it upon cancellation.
*/

bool OxideQCertificateError::isCancelled() const {
  Q_D(const OxideQCertificateError);

  return d->error_->IsCancelled();
}

/*!
\property OxideQCertificateError::isMainFrame

Indicates whether this error originates from the main frame.
*/

bool OxideQCertificateError::isMainFrame() const {
  Q_D(const OxideQCertificateError);

  return d->error_->is_main_frame();
}

/*!
\property OxideQCertificateError::isSubresource

Indicates whether this error originates from a subresource within its frame.
*/

bool OxideQCertificateError::isSubresource() const {
  Q_D(const OxideQCertificateError);

  return d->error_->is_subresource();
}

/*!
\property OxideQCertificateError::overridable

Indicates whether this error is overridable. Only errors from main-frame
document loads (where isMainFrame is true and isSubresource is false) can
be overridable. Certain types of errors are never overridable.

If the error is not overridable, calls to \l{allow} or \l{deny} are ignored.
*/

bool OxideQCertificateError::overridable() const {
  Q_D(const OxideQCertificateError);

  return d->error_->overridable();
}

/*!
\property OxideQCertificateError::strictEnforcement

Indicates whether this error is for a connection which is required to be secure
due to HSTS policy. If this is true, \l{overridable} will be false.
*/

bool OxideQCertificateError::strictEnforcement() const {
  Q_D(const OxideQCertificateError);

  return d->error_->strict_enforcement();
}

/*!
\property OxideQCertificateError::certificate

The certificate associated with this error.
*/

OxideQSslCertificate OxideQCertificateError::certificate() const {
  Q_D(const OxideQCertificateError);

  return d->certificate_;
}

/*!
\property OxideQCertificateError::certError

The error code for this certificate error.
*/

OxideQCertificateError::Error OxideQCertificateError::certError() const {
  Q_D(const OxideQCertificateError);

  return static_cast<Error>(d->error_->cert_error());
}

/*!
If this error is overridable (\l{overridable} is true), then calling this will
allow the connection to proceed. Calling this has no effect if the error is not
overridable.

\sa deny
*/

void OxideQCertificateError::allow() {
  Q_D(OxideQCertificateError);

  d->respond(true);
}

/*!
If this error is overridable (\l{overridable} is true), then calling this will
abort the connection. Calling this has no effect if the error is not
overridable.

\sa allow
*/

void OxideQCertificateError::deny() {
  Q_D(OxideQCertificateError);

  d->respond(false);
}
