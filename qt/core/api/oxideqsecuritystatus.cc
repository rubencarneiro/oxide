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

#include "oxideqsecuritystatus.h"
#include "oxideqsecuritystatus_p.h"

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/cert_store.h"
#include "net/cert/x509_certificate.h"

#include "qt/core/api/oxideqsslcertificate.h"
#include "qt/core/api/oxideqsslcertificate_p.h"
#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_security_status.h"

OxideQSecurityStatusPrivate::OxideQSecurityStatusPrivate(
    OxideQSecurityStatus* q)
    : q_ptr(q),
      web_view_(NULL) {}

OxideQSecurityStatusPrivate::~OxideQSecurityStatusPrivate() {}

// static
OxideQSecurityStatus* OxideQSecurityStatusPrivate::Create(
    oxide::qt::WebView* view,
    QObject* parent) {
  DCHECK(view);

  OxideQSecurityStatus* rv = new OxideQSecurityStatus(parent);
  get(rv)->web_view_ = view;

  return rv;
}

// static
OxideQSecurityStatusPrivate* OxideQSecurityStatusPrivate::get(
    OxideQSecurityStatus* q) {
  return q->d_func();
}

void OxideQSecurityStatusPrivate::Update(const oxide::SecurityStatus& old) {
  Q_Q(OxideQSecurityStatus);

  const oxide::SecurityStatus& status = web_view_->security_status();

  if (old.security_level() != status.security_level()) {
    Q_EMIT q->securityLevelChanged();
  }
  if (old.content_status() != status.content_status()) {
    Q_EMIT q->contentStatusChanged();
  }
  if (old.cert_status() != status.cert_status()) {
    Q_EMIT q->certStatusChanged();
  }
  if (old.cert_id() != status.cert_id()) {
    cert_.reset();
    Q_EMIT q->certificateChanged();
  }
}

OxideQSecurityStatus::OxideQSecurityStatus(QObject* parent)
    : QObject(parent),
      d_ptr(new OxideQSecurityStatusPrivate(this)) {
  COMPILE_ASSERT(
      SecurityLevelNone ==
      static_cast<SecurityLevel>(oxide::SecurityStatus::SECURITY_LEVEL_NONE),
      security_level_enums_none_doesnt_match);
  COMPILE_ASSERT(
      SecurityLevelSecure ==
      static_cast<SecurityLevel>(oxide::SecurityStatus::SECURITY_LEVEL_SECURE),
      security_level_enums_secure_doesnt_match);
  COMPILE_ASSERT(
      SecurityLevelSecureEV ==
      static_cast<SecurityLevel>(oxide::SecurityStatus::SECURITY_LEVEL_SECURE_EV),
      security_level_enums_secure_ev_doesnt_match);
  COMPILE_ASSERT(
      SecurityLevelWarning ==
      static_cast<SecurityLevel>(oxide::SecurityStatus::SECURITY_LEVEL_WARNING),
      security_level_enums_warning_doesnt_match);
  COMPILE_ASSERT(
      SecurityLevelError ==
      static_cast<SecurityLevel>(oxide::SecurityStatus::SECURITY_LEVEL_ERROR),
      security_level_enums_error_doesnt_match);

  COMPILE_ASSERT(
      ContentStatusNormal ==
      static_cast<ContentStatus>(content::SSLStatus::NORMAL_CONTENT),
      content_status_enums_normal_doesnt_match);
  COMPILE_ASSERT(
      ContentStatusDisplayedInsecure ==
      static_cast<ContentStatus>(content::SSLStatus::DISPLAYED_INSECURE_CONTENT),
      content_status_enums_displayed_insecure_doesnt_match);
  COMPILE_ASSERT(
      ContentStatusRanInsecure ==
      static_cast<ContentStatus>(content::SSLStatus::RAN_INSECURE_CONTENT),
      content_status_enums_ran_insecure_doesnt_match);

  COMPILE_ASSERT(
      CertStatusOk ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_OK),
      cert_status_enums_ok_doesnt_match);
  COMPILE_ASSERT(
      CertStatusBadIdentity ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_BAD_IDENTITY),
      cert_status_enums_bad_identity_doesnt_match);
  COMPILE_ASSERT(
      CertStatusExpired ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_EXPIRED),
      cert_status_enums_expired_doesnt_match);
  COMPILE_ASSERT(
      CertStatusDateInvalid ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_DATE_INVALID),
      cert_status_enums_date_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertStatusAuthorityInvalid ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_AUTHORITY_INVALID),
      cert_status_enums_authority_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertStatusRevocationCheckFailed ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_REVOCATION_CHECK_FAILED),
      cert_status_enums_revocation_check_failed_doesnt_match);
  COMPILE_ASSERT(
      CertStatusRevoked ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_REVOKED),
      cert_status_enums_revoked_doesnt_match);
  COMPILE_ASSERT(
      CertStatusInvalid ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_INVALID),
      cert_status_enums_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertStatusInsecure ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_INSECURE),
      cert_status_enums_insecure_doesnt_match);
  COMPILE_ASSERT(
      CertStatusGenericError ==
      static_cast<CertStatus>(oxide::SecurityStatus::CERT_STATUS_GENERIC_ERROR),
      cert_status_enums_generic_error_doesnt_match);
}

OxideQSecurityStatus::~OxideQSecurityStatus() {}

OxideQSecurityStatus::SecurityLevel
OxideQSecurityStatus::securityLevel() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<SecurityLevel>(
      d->web_view_->security_status().security_level());
}

OxideQSecurityStatus::ContentStatus
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<ContentStatus>(
      d->web_view_->security_status().content_status());
}

OxideQSecurityStatus::CertStatus
OxideQSecurityStatus::certStatus() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<CertStatus>(
      d->web_view_->security_status().cert_status());
}

OxideQSslCertificate* OxideQSecurityStatus::certificate() const {
  Q_D(const OxideQSecurityStatus);

  if (d->cert_) {
    return d->cert_.get();
  }

  int cert_id = d->web_view_->security_status().cert_id();
  if (cert_id == 0) {
    return NULL;
  }

  scoped_refptr<net::X509Certificate> cert;
  if (!content::CertStore::GetInstance()->RetrieveCert(cert_id, &cert)) {
    return NULL;
  }

  d->cert_.reset(OxideQSslCertificatePrivate::Create(cert.get()));

  return d->cert_.get();
}
