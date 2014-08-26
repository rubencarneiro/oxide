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

#include "oxide_security_status.h"

#include "base/logging.h"
#include "content/public/browser/cert_store.h"
#include "content/public/common/security_style.h"
#include "net/cert/cert_status_flags.h"

#include "shared/base/oxide_enum_flags.h"

namespace oxide {

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(content::SSLStatus::ContentStatusFlags)
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(SecurityStatus::CertStatus)

inline SecurityStatus::SecurityLevel CalculateSecurityLevel(
    const content::SSLStatus& ssl_status) {
  if (ssl_status.security_style == content::SECURITY_STYLE_UNKNOWN ||
      ssl_status.security_style == content::SECURITY_STYLE_UNAUTHENTICATED) {
    return SecurityStatus::SECURITY_LEVEL_NONE;
  }

  if (ssl_status.security_style ==
      content::SECURITY_STYLE_AUTHENTICATION_BROKEN) {
    return SecurityStatus::SECURITY_LEVEL_ERROR;
  }

  DCHECK_EQ(ssl_status.security_style, content::SECURITY_STYLE_AUTHENTICATED);
  CHECK(!(ssl_status.content_status &
          content::SSLStatus::RAN_INSECURE_CONTENT)) <<
      "Invalid SSLStatus - RAN_INSECURE_CONTENT and SECURITY_STYLE_AUTHENTICATED "
      "are meant to be mutually exclusive!";

  if (ssl_status.content_status &
      content::SSLStatus::DISPLAYED_INSECURE_CONTENT) {
    return SecurityStatus::SECURITY_LEVEL_WARNING;
  }

  DCHECK_EQ(
      static_cast<content::SSLStatus::ContentStatusFlags>(
        ssl_status.content_status),
      content::SSLStatus::NORMAL_CONTENT);

  if (net::IsCertStatusError(ssl_status.cert_status)) {
    CHECK(net::IsCertStatusMinorError(ssl_status.cert_status)) <<
        "Invalid SSLStatus - Non-minor cert status error and "
        "SECURITY_STYLE_AUTHENTICATED are meant to be mutually exclusive!";
    return SecurityStatus::SECURITY_LEVEL_WARNING;
  }

  if ((ssl_status.cert_status & net::CERT_STATUS_IS_EV) &&
      content::CertStore::GetInstance()->RetrieveCert(ssl_status.cert_id,
                                                      NULL)) {
    return SecurityStatus::SECURITY_LEVEL_SECURE_EV;
  }

  return SecurityStatus::SECURITY_LEVEL_SECURE;
}

inline SecurityStatus::CertStatus CalculateCertStatus(
    net::CertStatus cert_status) {
  SecurityStatus::CertStatus rv = SecurityStatus::CERT_STATUS_OK;

  // Handle flags that have a direct mapping to CertErrorStatus first
  if (cert_status & net::CERT_STATUS_COMMON_NAME_INVALID) {
    rv |= SecurityStatus::CERT_STATUS_BAD_IDENTITY;
    cert_status &= ~net::CERT_STATUS_COMMON_NAME_INVALID;
  }
  if (cert_status & net::CERT_STATUS_DATE_INVALID) {
    rv |= SecurityStatus::CERT_STATUS_DATE_INVALID;
    cert_status &= ~net::CERT_STATUS_DATE_INVALID;
  }
  if (cert_status & net::CERT_STATUS_AUTHORITY_INVALID) {
    rv |= SecurityStatus::CERT_STATUS_AUTHORITY_INVALID;
    cert_status &= ~net::CERT_STATUS_AUTHORITY_INVALID;
  }
  if (cert_status & net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION) {
    rv |= SecurityStatus::CERT_STATUS_REVOCATION_CHECK_FAILED;
    cert_status &= ~net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;
  }
  if (cert_status & net::CERT_STATUS_REVOKED) {
    rv |= SecurityStatus::CERT_STATUS_REVOKED;
    cert_status &= ~net::CERT_STATUS_REVOKED;
  }
  if (cert_status & net::CERT_STATUS_INVALID) {
    rv |= SecurityStatus::CERT_STATUS_INVALID;
    cert_status &= ~net::CERT_STATUS_INVALID;
  }
  if (cert_status & net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM) {
    rv |= SecurityStatus::CERT_STATUS_INSECURE;
    cert_status &= ~net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM;
  }
  if (cert_status & net::CERT_STATUS_WEAK_KEY) {
    rv |= SecurityStatus::CERT_STATUS_INSECURE;
    cert_status &= ~net::CERT_STATUS_WEAK_KEY;
  }

  // For flags that don't have a direct mapping to CertErrorStatus,
  // set the generic flag if any non-minor error bits are set
  if (net::IsCertStatusError(cert_status) &&
      !net::IsCertStatusMinorError(cert_status)) {
    rv |= SecurityStatus::CERT_STATUS_GENERIC;
  }

  return rv;
}

}

SecurityStatus::SecurityStatus()
    : security_level_(SECURITY_LEVEL_NONE),
      content_status_(content::SSLStatus::NORMAL_CONTENT),
      cert_status_(CERT_STATUS_OK) {}

SecurityStatus::SecurityStatus(const content::SSLStatus& ssl_status)
    : security_level_(SECURITY_LEVEL_NONE),
      content_status_(content::SSLStatus::NORMAL_CONTENT),
      cert_status_(CERT_STATUS_OK) {
  Update(ssl_status);
}

SecurityStatus::~SecurityStatus() {}

void SecurityStatus::Update(const content::SSLStatus& ssl_status) {
  security_level_ = CalculateSecurityLevel(ssl_status);

  content_status_ = static_cast<content::SSLStatus::ContentStatusFlags>(
      ssl_status.content_status);

  cert_status_ = CalculateCertStatus(ssl_status.cert_status);

  cert_id_ = ssl_status.cert_id;
}

} // namespace oxide
