// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include <QtDebug>

#include "base/bind.h"
#include "base/logging.h"
#include "base/macros.h"

#include "shared/browser/oxide_permission_request.h"
#include "shared/browser/oxide_security_types.h"

#include "oxideqsslcertificate.h"

OxideQCertificateErrorPrivate::OxideQCertificateErrorPrivate(
    const QUrl& url,
    bool is_main_frame,
    bool is_subresource,
    bool strict_enforcement,
    scoped_ptr<OxideQSslCertificate> certificate,
    OxideQCertificateError::Error cert_error,
    scoped_ptr<oxide::SimplePermissionRequest> request)
    : url_(url),
      is_main_frame_(is_main_frame),
      is_subresource_(is_subresource),
      strict_enforcement_(strict_enforcement),
      certificate_(certificate.Pass()),
      cert_error_(cert_error),
      request_(request.Pass()),
      did_respond_(false),
      is_cancelled_(false) {}

void OxideQCertificateErrorPrivate::OnCancel() {
  Q_Q(OxideQCertificateError);

  DCHECK(!is_cancelled_ && !did_respond_);
  is_cancelled_ = true;

  Q_EMIT q->cancelled();
}

void OxideQCertificateErrorPrivate::respond(bool accept) {
  if (!request_) {
    qWarning() << "Cannot respond to a non-overridable request";
    return;
  }

  if (did_respond_) {
    qWarning() << "Cannot respond to a CertificateError more than once";
    return;
  }

  if (is_cancelled_) {
    qWarning() << "Cannot respond to a CertificateError that has been cancelled";
    return;
  }

  did_respond_ = true;
  if (accept) {
    request_->Allow();
  } else {
    request_->Deny();
  }
}

OxideQCertificateErrorPrivate::~OxideQCertificateErrorPrivate() {}

// static
OxideQCertificateError* OxideQCertificateErrorPrivate::Create(
    const QUrl& url,
    bool is_main_frame,
    bool is_subresource,
    bool strict_enforcement,
    scoped_ptr<OxideQSslCertificate> certificate,
    OxideQCertificateError::Error cert_error,
    scoped_ptr<oxide::SimplePermissionRequest> request,
    QObject* parent) {
  return new OxideQCertificateError(
      *new OxideQCertificateErrorPrivate(
        url,
        is_main_frame,
        is_subresource,
        strict_enforcement,
        certificate.Pass(),
        cert_error,
        request.Pass()),
      parent);
}

OxideQCertificateError::OxideQCertificateError(
    OxideQCertificateErrorPrivate& dd,
    QObject* parent)
    : QObject(parent),
      d_ptr(&dd) {
  Q_D(OxideQCertificateError);

  d->q_ptr = this;
  d->request_->SetCancelCallback(
      base::Bind(&OxideQCertificateErrorPrivate::OnCancel,
                 base::Unretained(d)));
  
  COMPILE_ASSERT(
      OK == static_cast<Error>(oxide::CERT_OK),
      error_enums_ok_doesnt_match);
  COMPILE_ASSERT(
      ErrorBadIdentity == static_cast<Error>(oxide::CERT_ERROR_BAD_IDENTITY),
      error_enums_bad_identity_doesnt_match);
  COMPILE_ASSERT(
      ErrorExpired == static_cast<Error>(oxide::CERT_ERROR_EXPIRED),
      error_enums_expired_doesnt_match);
  COMPILE_ASSERT(
      ErrorDateInvalid == static_cast<Error>(oxide::CERT_ERROR_DATE_INVALID),
      error_enums_date_invalid_doesnt_match);
  COMPILE_ASSERT(
      ErrorAuthorityInvalid ==
        static_cast<Error>(oxide::CERT_ERROR_AUTHORITY_INVALID),
      error_enums_authority_invalid_doesnt_match);
  COMPILE_ASSERT(
      ErrorRevoked == static_cast<Error>(oxide::CERT_ERROR_REVOKED),
      error_enums_revoked_doesnt_match);
  COMPILE_ASSERT(
      ErrorInvalid == static_cast<Error>(oxide::CERT_ERROR_INVALID),
      error_enums_invalid_doesnt_match);
  COMPILE_ASSERT(
      ErrorInsecure == static_cast<Error>(oxide::CERT_ERROR_INSECURE),
      error_enums_insecure_doesnt_match);
  COMPILE_ASSERT(
      ErrorGeneric == static_cast<Error>(oxide::CERT_ERROR_GENERIC),
      error_enums_generic_doesnt_match);
}

OxideQCertificateError::~OxideQCertificateError() {}

QUrl OxideQCertificateError::url() const {
  Q_D(const OxideQCertificateError);

  return d->url_;
}

bool OxideQCertificateError::isCancelled() const {
  Q_D(const OxideQCertificateError);

  return d->is_cancelled_;
}

bool OxideQCertificateError::isMainFrame() const {
  Q_D(const OxideQCertificateError);

  return d->is_main_frame_;
}

bool OxideQCertificateError::isSubresource() const {
  Q_D(const OxideQCertificateError);

  return d->is_subresource_;
}

bool OxideQCertificateError::overridable() const {
  Q_D(const OxideQCertificateError);

  return !!d->request_;
}

bool OxideQCertificateError::strictEnforcement() const {
  Q_D(const OxideQCertificateError);

  return d->strict_enforcement_;
}

OxideQSslCertificate* OxideQCertificateError::certificate() const {
  Q_D(const OxideQCertificateError);

  return d->certificate_.get();
}

OxideQCertificateError::Error OxideQCertificateError::certError() const {
  Q_D(const OxideQCertificateError);

  return d->cert_error_;
}

void OxideQCertificateError::allow() {
  Q_D(OxideQCertificateError);

  d->respond(true);
}

void OxideQCertificateError::deny() {
  Q_D(OxideQCertificateError);

  d->respond(false);
}
