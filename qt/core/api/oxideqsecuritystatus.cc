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

#include "oxideqsecuritystatus.h"
#include "oxideqsecuritystatus_p.h"

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "net/cert/x509_certificate.h"

#include "qt/core/browser/ssl/oxide_qt_security_status.h"
#include "shared/browser/ssl/oxide_security_types.h"

#include "oxideqsslcertificate.h"
#include "oxideqsslcertificate_p.h"

#define STATIC_ASSERT_MATCHING_ENUM(a, b) \
  static_assert(static_cast<int>(a) == static_cast<int>(b), \
                "Mismatched enum values");

STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::SecurityLevelNone,
                            oxide::SECURITY_LEVEL_NONE)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::SecurityLevelSecure,
                            oxide::SECURITY_LEVEL_SECURE)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::SecurityLevelSecureEV,
                            oxide::SECURITY_LEVEL_SECURE_EV)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::SecurityLevelWarning,
                            oxide::SECURITY_LEVEL_WARNING)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::SecurityLevelError,
                            oxide::SECURITY_LEVEL_ERROR)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::ContentStatusNormal,
                            content::SSLStatus::NORMAL_CONTENT)
STATIC_ASSERT_MATCHING_ENUM(
    OxideQSecurityStatus::ContentStatusDisplayedInsecure,
    content::SSLStatus::DISPLAYED_INSECURE_CONTENT)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::ContentStatusRanInsecure,
                            content::SSLStatus::RAN_INSECURE_CONTENT)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusOk,
                            oxide::CERT_STATUS_OK)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusBadIdentity,
                            oxide::CERT_STATUS_BAD_IDENTITY)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusExpired,
                            oxide::CERT_STATUS_EXPIRED)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusDateInvalid,
                            oxide::CERT_STATUS_DATE_INVALID)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusAuthorityInvalid,
                            oxide::CERT_STATUS_AUTHORITY_INVALID)
STATIC_ASSERT_MATCHING_ENUM(
    OxideQSecurityStatus::CertStatusRevocationCheckFailed,
    oxide::CERT_STATUS_REVOCATION_CHECK_FAILED)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusRevoked,
                            oxide::CERT_STATUS_REVOKED)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusInvalid,
                            oxide::CERT_STATUS_INVALID)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusInsecure,
                            oxide::CERT_STATUS_INSECURE)
STATIC_ASSERT_MATCHING_ENUM(OxideQSecurityStatus::CertStatusGenericError,
                            oxide::CERT_STATUS_GENERIC_ERROR)

OxideQSecurityStatusPrivate::OxideQSecurityStatusPrivate(
    OxideQSecurityStatus* q)
    : q_ptr(q),
      proxy_(new oxide::qt::SecurityStatus(q)),
      cert_invalidated_(true) {}

OxideQSecurityStatusPrivate::~OxideQSecurityStatusPrivate() {}

// static
OxideQSecurityStatus* OxideQSecurityStatusPrivate::Create() {
  return new OxideQSecurityStatus();
}

// static
OxideQSecurityStatusPrivate* OxideQSecurityStatusPrivate::get(
    OxideQSecurityStatus* q) {
  return q->d_func();
}

void OxideQSecurityStatusPrivate::InvalidateCertificate() {
  cert_invalidated_ = true;
  cert_ = OxideQSslCertificate();
}

OxideQSecurityStatus::OxideQSecurityStatus()
    : d_ptr(new OxideQSecurityStatusPrivate(this)) {}

OxideQSecurityStatus::~OxideQSecurityStatus() {}

OxideQSecurityStatus::SecurityLevel
OxideQSecurityStatus::securityLevel() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<SecurityLevel>(d->proxy_->GetSecurityLevel());
}

OxideQSecurityStatus::ContentStatus
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<ContentStatus>(d->proxy_->GetContentStatus());
}

OxideQSecurityStatus::CertStatus OxideQSecurityStatus::certStatus() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<CertStatus>(d->proxy_->GetCertStatus());
}

QVariant OxideQSecurityStatus::certificate() const {
  Q_D(const OxideQSecurityStatus);

  if (d->cert_invalidated_) {
    d->cert_invalidated_ = false;
    scoped_refptr<net::X509Certificate> cert = d->proxy_->GetCert();
    if (cert) {
      d->cert_ = OxideQSslCertificateData::Create(cert.get());
    }
  }

  if (!d->cert_.isValid()) {
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  return QVariant::fromValue(d->cert_);
}
