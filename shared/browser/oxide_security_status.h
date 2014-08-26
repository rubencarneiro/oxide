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

#ifndef _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_
#define _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_

#include "base/compiler_specific.h"
#include "content/public/common/ssl_status.h"

namespace oxide {

class SecurityStatus FINAL {
 public:

  SecurityStatus();
  SecurityStatus(const content::SSLStatus& ssl_status);

  ~SecurityStatus();

  enum SecurityLevel {
    SECURITY_LEVEL_NONE,
    SECURITY_LEVEL_SECURE,
    SECURITY_LEVEL_SECURE_EV,
    SECURITY_LEVEL_WARNING,
    SECURITY_LEVEL_ERROR
  };

  enum CertErrorStatus {
    CERT_ERROR_STATUS_OK = 0,
    CERT_ERROR_STATUS_BAD_IDENTITY = 1 << 0,
    CERT_ERROR_STATUS_DATE_INVALID = 1 << 1,
    CERT_ERROR_STATUS_AUTHORITY_INVALID = 1 << 2,
    CERT_ERROR_STATUS_REVOCATION_CHECK_FAILED = 1 << 3,
    CERT_ERROR_STATUS_REVOKED = 1 << 4,
    CERT_ERROR_STATUS_INVALID = 1 << 5,
    CERT_ERROR_STATUS_INSECURE = 1 << 6,
    CERT_ERROR_STATUS_GENERIC = 1 << 7
  };

  void Update(const content::SSLStatus& ssl_status);

  SecurityLevel security_level() const { return security_level_; }
  content::SSLStatus::ContentStatusFlags content_status() const {
    return content_status_;
  }
  CertErrorStatus cert_error_status() const { return cert_error_status_; }
  int cert_id() const { return cert_id_; }

 private:
  SecurityLevel security_level_;
  content::SSLStatus::ContentStatusFlags content_status_;
  CertErrorStatus cert_error_status_;
  int cert_id_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_
