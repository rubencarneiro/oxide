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

/*!
\class OxideQSecurityStatus
\inheaderfile oxideqsecuritystatus.h
\inmodule OxideQtCore

\brief Security status for a webview

OxideQSecurityStatus provides security status information for a webview. It
provides an overall security level indication via securityLevel.

Details about the X.509 certificate for the current site can be accessed via
\l{certificate} and certStatus.
*/

/*!
\enum OxideQSecurityStatus::SecurityLevel

Represents an overall security level for a webview.

\value SecurityLevelNone
The current page was loaded via an insecure connection (eg, http).

\value SecurityLevelSecure
The current page was loaded via a secure connection.

\value SecurityLevelSecureEV
The current page was loaded via a secure connection and the supplied certificate
is an EV certificate.

\value SecurityLevelWarning
The current page was loaded via a secure connection, but the security of the
page is degraded. This could be because some passive content (eg, images) were
not loaded via a secure connection, or the supplied certificate has an error
that is considered to be minor (eg, revocation check failed). A web browser
would generally represent this status using a padlock icon with a warning
triangle.

\value SecurityLevelError
The security of the current page is broken. This could be because of a
certificate error permitted by the application via
OxideQCertificateError::allow, or because some active content (eg, scripts or
CSS) were loaded via an insecure connection. A web browser would generally
represent this status using a broken padlock icon.
*/

/*!
\enum OxideQSecurityStatus::ContentStatusFlags

Represents the status of the currently displayed content.

\value ContentStatusNormal
All elements on the current page were loaded via a secure connection.

\value ContentStatusDisplayedInsecure
The current page contains passive elements such as images or videos that were
loaded over an insecure connection.

\value ContentStatusRanInsecure
The current page contains active elements such as scripts or CSS that were
loaded over an insecure connection.
*/

/*!
\enum OxideQSecurityStatus::CertStatusFlags

Represents the status of the current X.509 certificate.

\value CertStatusOk
The current certificate has no errors.

\value CertStatusBadIdentity
The identity of the certificate does not match the identity of the site.

\value CertStatusExpired
The certificate has expired.

\value CertStatusDateInvalid
The certificate has a date that is invalid, eg, its start date is in the future.

\value CertStatusAuthorityInvalid
The certificate is signed by an authority that isn't trusted.

\value CertStatusRevocationCheckFailed
The revocation status of the certificate could not be determined.

\value CertStatusRevoked
The certificate has been revoked.

\value CertStatusInvalid
The certificate is invalid, eg, it has errors.

\value CertStatusInsecure
The certificate is insecure, eg, it uses a weak signature algorithm or has a
weak public key.

\value CertStatusGenericError
This is used for all other unspecified certificate errors.
*/

OxideQSecurityStatus::OxideQSecurityStatus()
    : d_ptr(new OxideQSecurityStatusPrivate(this)) {}

/*!
\internal
*/

OxideQSecurityStatus::~OxideQSecurityStatus() {}

/*!
\property OxideQSecurityStatus::securityLevel

The current security level. This is useful for displaying a security hint (such
as a padlock icon) to the user.
*/

OxideQSecurityStatus::SecurityLevel
OxideQSecurityStatus::securityLevel() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<SecurityLevel>(d->proxy_->GetSecurityLevel());
}

/*!
\property OxideQSecurityStatus::contentStatus

The current content status. This can be used to determine if the page has loaded
any insecure content. If securityLevel is SecurityLevelNone, this will be
ContentStatusNormal.
*/

OxideQSecurityStatus::ContentStatus
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  content::SSLStatus::ContentStatusFlags status = d->proxy_->GetContentStatus();
  ContentStatus rv = ContentStatusNormal;

  if (status & content::SSLStatus::DISPLAYED_INSECURE_CONTENT) {
    rv |= ContentStatusDisplayedInsecure;
  }
  if (status & content::SSLStatus::RAN_INSECURE_CONTENT) {
    rv |= ContentStatusRanInsecure;
  }

  return rv;
}

/*!
\property OxideQSecurityStatus::certStatus

The status of the current certificate - this can be used to determine any
errors that affect it. If securityLevel is SecurityLevelNone, this will be
CertStatusOk.
*/

OxideQSecurityStatus::CertStatus OxideQSecurityStatus::certStatus() const {
  Q_D(const OxideQSecurityStatus);

  return static_cast<CertStatus>(d->proxy_->GetCertStatus());
}

/*!
\property OxideQSecurityStatus::certificate

The X.509 certificate for the currently displayed page. This will be a valid
OxideQSslCertificate if securityLevel is not SecurityLevelNone, else it will be
a null variant.
*/

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
