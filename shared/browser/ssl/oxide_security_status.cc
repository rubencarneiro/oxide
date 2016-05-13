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
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/security_style.h"
#include "net/cert/cert_status_flags.h"
#include "net/cert/x509_certificate.h"

#include "shared/common/oxide_enum_flags.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(SecurityStatus);

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(content::SSLStatus::ContentStatusFlags)
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(CertStatusFlags)
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(SecurityStatus::ChangedFlags)

inline SecurityLevel CalculateSecurityLevel(
    const content::SSLStatus& ssl_status,
    net::X509Certificate* cert) {
  if (ssl_status.security_style == content::SECURITY_STYLE_UNKNOWN ||
      ssl_status.security_style == content::SECURITY_STYLE_UNAUTHENTICATED) {
    return SECURITY_LEVEL_NONE;
  }

  if (ssl_status.security_style ==
      content::SECURITY_STYLE_AUTHENTICATION_BROKEN) {
    return SECURITY_LEVEL_ERROR;
  }

  DCHECK_EQ(ssl_status.security_style, content::SECURITY_STYLE_AUTHENTICATED);
  CHECK(!(ssl_status.content_status &
          content::SSLStatus::RAN_INSECURE_CONTENT)) <<
      "Invalid SSLStatus - RAN_INSECURE_CONTENT and SECURITY_STYLE_AUTHENTICATED "
      "are meant to be mutually exclusive!";

  if (ssl_status.content_status &
      content::SSLStatus::DISPLAYED_INSECURE_CONTENT) {
    return SECURITY_LEVEL_WARNING;
  }

  DCHECK_EQ(
      static_cast<content::SSLStatus::ContentStatusFlags>(
        ssl_status.content_status),
      content::SSLStatus::NORMAL_CONTENT);

  if (net::IsCertStatusError(ssl_status.cert_status)) {
    CHECK(net::IsCertStatusMinorError(ssl_status.cert_status)) <<
        "Invalid SSLStatus - Non-minor cert status error and "
        "SECURITY_STYLE_AUTHENTICATED are meant to be mutually exclusive!";
    return SECURITY_LEVEL_WARNING;
  }

  if ((ssl_status.cert_status & net::CERT_STATUS_IS_EV) && cert) {
    return SECURITY_LEVEL_SECURE_EV;
  }

  return SECURITY_LEVEL_SECURE;
}

inline CertStatusFlags CalculateCertStatus(net::CertStatus cert_status,
                                           net::X509Certificate* cert) {
  CertStatusFlags rv = CERT_STATUS_OK;

  // Handle flags that have a direct mapping to CertErrorStatus first
  if (cert_status & net::CERT_STATUS_COMMON_NAME_INVALID) {
    rv |= CERT_STATUS_BAD_IDENTITY;
    cert_status &= ~net::CERT_STATUS_COMMON_NAME_INVALID;
  }
  if (cert_status & net::CERT_STATUS_DATE_INVALID) {
    if (cert && cert->HasExpired()) {
      rv |= CERT_STATUS_EXPIRED;
    } else {
      // The date could be in the future or issuer certificates could
      // have expired. In the latter case, perhaps make this
      // CERT_STATUS_EXPIRED too?
      rv |= CERT_STATUS_DATE_INVALID;
    }
    cert_status &= ~net::CERT_STATUS_DATE_INVALID;
  }
  if (cert_status & net::CERT_STATUS_AUTHORITY_INVALID) {
    rv |= CERT_STATUS_AUTHORITY_INVALID;
    cert_status &= ~net::CERT_STATUS_AUTHORITY_INVALID;
  }
  if (cert_status & net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION) {
    rv |= CERT_STATUS_REVOCATION_CHECK_FAILED;
    cert_status &= ~net::CERT_STATUS_UNABLE_TO_CHECK_REVOCATION;
  }
  if (cert_status & net::CERT_STATUS_REVOKED) {
    rv |= CERT_STATUS_REVOKED;
    cert_status &= ~net::CERT_STATUS_REVOKED;
  }
  if (cert_status & net::CERT_STATUS_INVALID) {
    rv |= CERT_STATUS_INVALID;
    cert_status &= ~net::CERT_STATUS_INVALID;
  }
  if (cert_status & net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM) {
    rv |= CERT_STATUS_INSECURE;
    cert_status &= ~net::CERT_STATUS_WEAK_SIGNATURE_ALGORITHM;
  }
  if (cert_status & net::CERT_STATUS_WEAK_KEY) {
    rv |= CERT_STATUS_INSECURE;
    cert_status &= ~net::CERT_STATUS_WEAK_KEY;
  }

  // For flags that don't have a direct mapping to CertErrorStatus,
  // set the generic flag if any non-minor error bits are set
  if (net::IsCertStatusError(cert_status) &&
      !net::IsCertStatusMinorError(cert_status)) {
    rv |= CERT_STATUS_GENERIC_ERROR;
  }

  return rv;
}

}

SecurityStatus::SecurityStatus(content::WebContents* contents)
    : contents_(contents),
      security_level_(SECURITY_LEVEL_NONE),
      content_status_(content::SSLStatus::NORMAL_CONTENT),
      cert_status_(CERT_STATUS_OK),
      cert_store_for_testing_(nullptr) {
  VisibleSSLStateChanged();
}

content::CertStore* SecurityStatus::GetCertStore() const {
  if (cert_store_for_testing_) {
    return cert_store_for_testing_;
  }

  return content::CertStore::GetInstance();
}

SecurityStatus::~SecurityStatus() {}

// static
void SecurityStatus::CreateForWebContents(content::WebContents* contents) {
  content::WebContentsUserData<SecurityStatus>::CreateForWebContents(contents);
}

// static
SecurityStatus* SecurityStatus::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<SecurityStatus>::FromWebContents(contents);
}

void SecurityStatus::VisibleSSLStateChanged() {
  SecurityLevel old_security_level = security_level_;
  content::SSLStatus::ContentStatusFlags old_content_status = content_status_;
  CertStatusFlags old_cert_status = cert_status_;
  scoped_refptr<net::X509Certificate> old_cert = cert_;

  content::NavigationEntry* entry =
      contents_->GetController().GetVisibleEntry();
  content::SSLStatus status = entry ? entry->GetSSL() : content::SSLStatus();

  cert_ = nullptr;
  GetCertStore()->RetrieveCert(status.cert_id, &cert_);

  security_level_ = CalculateSecurityLevel(status, cert_.get());
  content_status_ = static_cast<content::SSLStatus::ContentStatusFlags>(
      status.content_status);
  cert_status_ = CalculateCertStatus(status.cert_status, cert_.get());

  ChangedFlags flags = CHANGED_FLAG_NONE;
  if (old_security_level != security_level_) {
    flags |= CHANGED_FLAG_SECURITY_LEVEL;
  }
  if (old_content_status != content_status_) {
    flags |= CHANGED_FLAG_CONTENT_STATUS;
  }
  if (old_cert_status != cert_status_) {
    flags |= CHANGED_FLAG_CERT_STATUS;
  }
  if (old_cert != cert_) {
    flags |= CHANGED_FLAG_CERT;
  }

  if (flags == CHANGED_FLAG_NONE) {
    return;
  }

  callback_list_.Notify(flags);
}

std::unique_ptr<SecurityStatus::Subscription>
SecurityStatus::AddChangeCallback(const ObserverCallback& callback) {
  return callback_list_.Add(callback);
}

void SecurityStatus::SetCertStoreForTesting(content::CertStore* cert_store) {
  cert_store_for_testing_ = cert_store;
}

} // namespace oxide
