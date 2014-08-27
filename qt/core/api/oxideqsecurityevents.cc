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

#include "oxideqsecurityevents.h"
#include "oxideqsecurityevents_p.h"

#include <QtDebug>

#include "base/logging.h"
#include "base/macros.h"

#include "shared/browser/oxide_security_types.h"

#include "oxideqsslcertificate.h"

OxideQCertificateErrorPrivate::OxideQCertificateErrorPrivate(
    const QUrl& url,
    bool is_main_frame,
    bool is_subresource,
    bool overridable,
    bool strict_enforcement,
    scoped_ptr<OxideQSslCertificate> certificate,
    OxideQCertificateError::CertError cert_error,
    const base::Callback<void(bool)>& callback)
    : url_(url),
      is_main_frame_(is_main_frame),
      is_subresource_(is_subresource),
      overridable_(overridable),
      strict_enforcement_(strict_enforcement),
      certificate_(certificate.Pass()),
      cert_error_(cert_error),
      callback_(callback) {}

void OxideQCertificateErrorPrivate::respond(bool accept) {
  if (callback_.is_null()) {
    qWarning() << "Cannot respond to a CertificateError more than once";
    return;
  }

  callback_.Run(accept);
  callback_.Reset();
}

OxideQCertificateErrorPrivate::~OxideQCertificateErrorPrivate() {}

// static
OxideQCertificateError* OxideQCertificateErrorPrivate::Create(
    const QUrl& url,
    bool is_main_frame,
    bool is_subresource,
    bool overridable,
    bool strict_enforcement,
    scoped_ptr<OxideQSslCertificate> certificate,
    OxideQCertificateError::CertError cert_error,
    const base::Callback<void(bool)>& callback,
    QObject* parent) {
  DCHECK(!callback.is_null());
  return new OxideQCertificateError(
      *new OxideQCertificateErrorPrivate(
        url,
        is_main_frame,
        is_subresource,
        overridable,
        strict_enforcement,
        certificate.Pass(),
        cert_error,
        callback),
      parent);
}

OxideQCertificateError::OxideQCertificateError(
    OxideQCertificateErrorPrivate& dd,
    QObject* parent)
    : QObject(parent),
      d_ptr(&dd) {
  COMPILE_ASSERT(
      CertOK == static_cast<CertError>(oxide::CERT_OK),
      cert_error_enums_ok_doesnt_match);
  COMPILE_ASSERT(
      CertErrorBadIdentity ==
        static_cast<CertError>(oxide::CERT_ERROR_BAD_IDENTITY),
      cert_error_enums_bad_identity_doesnt_match);
  COMPILE_ASSERT(
      CertErrorExpired == static_cast<CertError>(oxide::CERT_ERROR_EXPIRED),
      cert_error_enums_expired_doesnt_match);
  COMPILE_ASSERT(
      CertErrorDateInvalid ==
        static_cast<CertError>(oxide::CERT_ERROR_DATE_INVALID),
      cert_error_enums_date_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorAuthorityInvalid ==
        static_cast<CertError>(oxide::CERT_ERROR_AUTHORITY_INVALID),
      cert_error_enums_authority_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorRevoked == static_cast<CertError>(oxide::CERT_ERROR_REVOKED),
      cert_error_enums_revoked_doesnt_match);
  COMPILE_ASSERT(
      CertErrorInvalid == static_cast<CertError>(oxide::CERT_ERROR_INVALID),
      cert_error_enums_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorInsecure == static_cast<CertError>(oxide::CERT_ERROR_INSECURE),
      cert_error_enums_insecure_doesnt_match);
  COMPILE_ASSERT(
      CertErrorGeneric == static_cast<CertError>(oxide::CERT_ERROR_GENERIC),
      cert_error_enums_generic_doesnt_match);
}

OxideQCertificateError::~OxideQCertificateError() {
  Q_D(OxideQCertificateError);

  if (!d->callback_.is_null()) {
    d->callback_.Run(false);
  }
}

QUrl OxideQCertificateError::url() const {
  Q_D(const OxideQCertificateError);

  return d->url_;
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

  return d->overridable_;
}

bool OxideQCertificateError::strictEnforcement() const {
  Q_D(const OxideQCertificateError);

  return d->strict_enforcement_;
}

OxideQSslCertificate* OxideQCertificateError::certificate() const {
  Q_D(const OxideQCertificateError);

  return d->certificate_.get();
}

OxideQCertificateError::CertError OxideQCertificateError::certError() const {
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
