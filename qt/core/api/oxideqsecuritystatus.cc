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
  static_assert(
      SecurityLevelNone ==
        static_cast<SecurityLevel>(oxide::SECURITY_LEVEL_NONE),
      "SecurityLevel and oxide::SecurityLevel enums don't match: "
      "SecurityLevelNone");
  static_assert(
      SecurityLevelSecure ==
        static_cast<SecurityLevel>(oxide::SECURITY_LEVEL_SECURE),
      "SecurityLevel and oxide::SecurityLevel enums don't match: "
      "SecurityLevelSecure");
  static_assert(
      SecurityLevelSecureEV ==
        static_cast<SecurityLevel>(oxide::SECURITY_LEVEL_SECURE_EV),
      "SecurityLevel and oxide::SecurityLevel enums don't match: "
      "SecurityLevelSecureEV");
  static_assert(
      SecurityLevelWarning ==
        static_cast<SecurityLevel>(oxide::SECURITY_LEVEL_WARNING),
      "SecurityLevel and oxide::SecurityLevel enums don't match: "
      "SecurityLevelWarning");
  static_assert(
      SecurityLevelError ==
        static_cast<SecurityLevel>(oxide::SECURITY_LEVEL_ERROR),
      "SecurityLevel and oxide::SecurityLevel enums don't match: "
      "SecurityLevelError");

  static_assert(
      ContentStatusNormal ==
        static_cast<ContentStatusFlags>(content::SSLStatus::NORMAL_CONTENT),
      "ContentStatus and content::SSLStatus::ContentStatusFlags enums don't "
      "match: ContentStatusNormal");
  static_assert(
      ContentStatusDisplayedInsecure ==
        static_cast<ContentStatusFlags>(
          content::SSLStatus::DISPLAYED_INSECURE_CONTENT),
      "ContentStatus and content::SSLStatus::ContentStatusFlags enums don't "
      "match: ContentStatusDisplayedInsecure");
  static_assert(
      ContentStatusRanInsecure ==
        static_cast<ContentStatusFlags>(
          content::SSLStatus::RAN_INSECURE_CONTENT),
      "ContentStatus and content::SSLStatus::ContentStatusFlags enums don't "
      "match: ContentStatusRanInsecure");

  static_assert(
      CertStatusOk == static_cast<CertStatusFlags>(oxide::CERT_STATUS_OK),
      "CertStatus and oxide::CertStatus enums don't match: CertStatusOk");
  static_assert(
      CertStatusBadIdentity ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_BAD_IDENTITY),
      "CertStatus and oxide::CertStatus enums don't match: "
      "CertStatusBadIdentity");
  static_assert(
      CertStatusExpired ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_EXPIRED),
      "CertStatus and oxide::CertStatus enums don't match: CertStatusExpired");
  static_assert(
      CertStatusDateInvalid ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_DATE_INVALID),
      "CertStatus and oxide::CertStatus enums don't match: "
      "CertStatusDateInvalid");
  static_assert(
      CertStatusAuthorityInvalid ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_AUTHORITY_INVALID),
      "CertStatus and oxide::CertStatus enums don't match: "
      "CertStatusAuthorityInvalid");
  static_assert(
      CertStatusRevocationCheckFailed ==
        static_cast<CertStatusFlags>(
          oxide::CERT_STATUS_REVOCATION_CHECK_FAILED),
      "CertStatus and oxide::CertStatus enums don't match: "
      "CertStatusRevocationCheckFailed");
  static_assert(
      CertStatusRevoked ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_REVOKED),
      "CertStatus and oxide::CertStatus enums don't match: CertStatusRevoked");
  static_assert(
      CertStatusInvalid ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_INVALID),
      "CertStatus and oxide::CertStatus enums don't match: CertStatusInvalid");
  static_assert(
      CertStatusInsecure ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_INSECURE),
      "CertStatus and oxide::CertStatus enums don't match: CertStatusInsecure");
  static_assert(
      CertStatusGenericError ==
        static_cast<CertStatusFlags>(oxide::CERT_STATUS_GENERIC_ERROR),
      "CertStatus and oxide::CertStatus enums don't match: "
      "CertStatusGenericError");
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

OxideQSecurityStatus::ContentStatusFlags
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->view) {
    return ContentStatusNormal;
  }

  return static_cast<ContentStatusFlags>(
      d->view->GetSecurityStatus().content_status());
}

OxideQSecurityStatus::CertStatusFlags
OxideQSecurityStatus::certStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->view) {
    return CertStatusOk;
  }

  return static_cast<CertStatusFlags>(
      d->view->GetSecurityStatus().cert_status());
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
