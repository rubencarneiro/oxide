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

#include "qt/core/browser/oxide_qt_web_view.h"
#include "shared/browser/oxide_security_status.h"

OxideQSecurityStatusPrivate::OxideQSecurityStatusPrivate(
    OxideQSecurityStatus* q)
    : q_ptr(q),
      web_view_(NULL) {}

OxideQSecurityStatusPrivate::~OxideQSecurityStatusPrivate() {}

// static
OxideQSecurityStatusPrivate* OxideQSecurityStatusPrivate::get(
    OxideQSecurityStatus* q) {
  return q->d_func();
}

void OxideQSecurityStatusPrivate::Init(oxide::qt::WebView* view) {
  Q_Q(OxideQSecurityStatus);

  DCHECK(!web_view_);
  web_view_ = view;

  Q_EMIT q->securityLevelChanged();
  Q_EMIT q->securityStyleChanged();
  Q_EMIT q->contentStatusChanged();
  Q_EMIT q->certErrorStatusChanged();
}

void OxideQSecurityStatusPrivate::Update(const oxide::SecurityStatus& old) {
  Q_Q(OxideQSecurityStatus);

  DCHECK(web_view_);
  const oxide::SecurityStatus& status = web_view_->security_status();

  if (old.security_level() != status.security_level()) {
    Q_EMIT q->securityLevelChanged();
  }
  if (old.security_style() != status.security_style()) {
    Q_EMIT q->securityStyleChanged();
  }
  if (old.content_status() != status.content_status()) {
    Q_EMIT q->contentStatusChanged();
  }
  if (old.cert_error_status() != status.cert_error_status()) {
    Q_EMIT q->certErrorStatusChanged();
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
      SecurityStyleUnknown ==
      static_cast<SecurityStyle>(content::SECURITY_STYLE_UNKNOWN),
      security_style_enums_unknown_doesnt_match);
  COMPILE_ASSERT(
      SecurityStyleUnauthenticated ==
      static_cast<SecurityStyle>(content::SECURITY_STYLE_UNAUTHENTICATED),
      security_style_enums_unauthenticated_doesnt_match);
  COMPILE_ASSERT(
      SecurityStyleAuthenticationBroken ==
      static_cast<SecurityStyle>(content::SECURITY_STYLE_AUTHENTICATION_BROKEN),
      security_style_enums_authentication_broken_doesnt_match);
  COMPILE_ASSERT(
      SecurityStyleAuthenticated ==
      static_cast<SecurityStyle>(content::SECURITY_STYLE_AUTHENTICATED),
      security_style_enums_authenticated_doesnt_match);

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
      CertErrorStatusOk ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_OK),
      cert_error_status_enums_ok_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusBadIdentity ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_BAD_IDENTITY),
      cert_error_status_enums_bad_identity_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusDateInvalid ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_DATE_INVALID),
      cert_error_status_enums_date_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusAuthorityInvalid ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_AUTHORITY_INVALID),
      cert_error_status_enums_authority_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusRevocationCheckFailed ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_REVOCATION_CHECK_FAILED),
      cert_error_status_enums_revocation_check_failed_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusRevoked ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_REVOKED),
      cert_error_status_enums_revoked_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusInvalid ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_INVALID),
      cert_error_status_enums_invalid_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusInsecure ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_INSECURE),
      cert_error_status_enums_insecure_doesnt_match);
  COMPILE_ASSERT(
      CertErrorStatusGeneric ==
      static_cast<CertErrorStatus>(oxide::SecurityStatus::CERT_ERROR_STATUS_GENERIC),
      cert_error_status_enums_generic_doesnt_match);
}

OxideQSecurityStatus::~OxideQSecurityStatus() {}

OxideQSecurityStatus::SecurityLevel
OxideQSecurityStatus::securityLevel() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->web_view_) {
    return SecurityLevelNone;
  }

  return static_cast<SecurityLevel>(
      d->web_view_->security_status().security_level());
}

OxideQSecurityStatus::SecurityStyle
OxideQSecurityStatus::securityStyle() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->web_view_) {
    return SecurityStyleUnknown;
  }

  return static_cast<SecurityStyle>(
      d->web_view_->security_status().security_style());
}

OxideQSecurityStatus::ContentStatus
OxideQSecurityStatus::contentStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->web_view_) {
    return ContentStatusNormal;
  }

  return static_cast<ContentStatus>(
      d->web_view_->security_status().content_status());
}

OxideQSecurityStatus::CertErrorStatus
OxideQSecurityStatus::certErrorStatus() const {
  Q_D(const OxideQSecurityStatus);

  if (!d->web_view_) {
    return CertErrorStatusOk;
  }

  return static_cast<CertErrorStatus>(
      d->web_view_->security_status().cert_error_status());
}
