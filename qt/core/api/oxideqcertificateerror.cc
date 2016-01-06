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

#include <QString>
#include <QtDebug>
#include <QUrl>

#include "base/bind.h"
#include "base/logging.h"
#include "base/macros.h"
#include "net/cert/x509_certificate.h"

#include "shared/browser/oxide_security_types.h"
#include "shared/browser/ssl/oxide_certificate_error.h"

#include "oxideqsslcertificate_p.h"

OxideQCertificateErrorPrivate::OxideQCertificateErrorPrivate(
    scoped_ptr<oxide::CertificateError> error)
    : q_ptr(nullptr),
      certificate_(OxideQSslCertificateData::Create(error->cert())),
      error_(error.Pass()),
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
OxideQCertificateError* OxideQCertificateErrorPrivate::Create(
    scoped_ptr<oxide::CertificateError> error,
    QObject* parent) {
  return new OxideQCertificateError(
      *new OxideQCertificateErrorPrivate(error.Pass()),
      parent);
}

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

OxideQCertificateError::~OxideQCertificateError() {}

QUrl OxideQCertificateError::url() const {
  Q_D(const OxideQCertificateError);

  return QUrl(QString::fromStdString(d->error_->url().spec()));
}

bool OxideQCertificateError::isCancelled() const {
  Q_D(const OxideQCertificateError);

  return d->error_->IsCancelled();
}

bool OxideQCertificateError::isMainFrame() const {
  Q_D(const OxideQCertificateError);

  return d->error_->is_main_frame();
}

bool OxideQCertificateError::isSubresource() const {
  Q_D(const OxideQCertificateError);

  return d->error_->is_subresource();
}

bool OxideQCertificateError::overridable() const {
  Q_D(const OxideQCertificateError);

  return d->error_->overridable();
}

bool OxideQCertificateError::strictEnforcement() const {
  Q_D(const OxideQCertificateError);

  return d->error_->strict_enforcement();
}

OxideQSslCertificate OxideQCertificateError::certificate() const {
  Q_D(const OxideQCertificateError);

  return d->certificate_;
}

OxideQCertificateError::Error OxideQCertificateError::certError() const {
  Q_D(const OxideQCertificateError);

  return static_cast<Error>(d->error_->cert_error());
}

void OxideQCertificateError::allow() {
  Q_D(OxideQCertificateError);

  d->respond(true);
}

void OxideQCertificateError::deny() {
  Q_D(OxideQCertificateError);

  d->respond(false);
}
