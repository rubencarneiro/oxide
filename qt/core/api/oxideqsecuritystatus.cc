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
#include "content/public/browser/cert_store.h"
#include "net/cert/x509_certificate.h"

#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_security_status.h"
#include "shared/browser/oxide_security_types.h"

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
    : view(nullptr),
      q_ptr(q) {}

OxideQSecurityStatusPrivate::~OxideQSecurityStatusPrivate() {}

// static
OxideQSecurityStatusPrivate* OxideQSecurityStatusPrivate::get(
    OxideQSecurityStatus* q) {
  return q->d_func();
}

void OxideQSecurityStatusPrivate::Update(const oxide::SecurityStatus& old) {
  Q_Q(OxideQSecurityStatus);

  const oxide::SecurityStatus& status = view->GetSecurityStatus();

  if (old.security_level() != status.security_level()) {
    Q_EMIT q->securityLevelChanged();
  }
  if (old.content_status() != status.content_status()) {
    Q_EMIT q->contentStatusChanged();
  }
  if (old.cert_status() != status.cert_status()) {
    Q_EMIT q->certStatusChanged();
  }
  if (old.cert() != status.cert()) {
    cert_ = OxideQSslCertificate();
    Q_EMIT q->certificateChanged();
  }
}

OxideQSecurityStatus::OxideQSecurityStatus()
    : d_ptr(new OxideQSecurityStatusPrivate(this)) {
}

OxideQSecurityStatus::~OxideQSecurityStatus() {}

OxideQSecurityStatus::SecurityLevel
OxideQSecurityStatus::securityLevel() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->view) {
    return SecurityLevelNone;
  }

  return static_cast<SecurityLevel>(
      d->view->GetSecurityStatus().security_level());
}

OxideQSecurityStatus::ContentStatus
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->view) {
    return ContentStatusNormal;
  }

  return static_cast<ContentStatus>(
      d->view->GetSecurityStatus().content_status());
}

OxideQSecurityStatus::CertStatus OxideQSecurityStatus::certStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->view) {
    return CertStatusOk;
  }

  return static_cast<CertStatus>(d->view->GetSecurityStatus().cert_status());
}

QVariant OxideQSecurityStatus::certificate() const {
  Q_D(const OxideQSecurityStatus);

  if (d->cert_.isValid()) {
    return QVariant::fromValue(d->cert_);
  }

  if (!d->view) {
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  scoped_refptr<net::X509Certificate> cert =
      d->view->GetSecurityStatus().cert();
  if (!cert.get()) {
    return QVariant(static_cast<QVariant::Type>(QMetaType::VoidStar));
  }

  d->cert_ = OxideQSslCertificateData::Create(cert.get());

  return QVariant::fromValue(d->cert_);
}
