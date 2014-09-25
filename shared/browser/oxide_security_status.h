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
#include "base/memory/ref_counted.h"
#include "content/public/common/ssl_status.h"

#include "shared/browser/oxide_security_types.h"

namespace net {
class X509Certificate;
}

namespace oxide {

class SecurityStatus FINAL {
 public:

  SecurityStatus();
  SecurityStatus(const content::SSLStatus& ssl_status);

  ~SecurityStatus();

  void Update(const content::SSLStatus& ssl_status);

  SecurityLevel security_level() const { return security_level_; }
  content::SSLStatus::ContentStatusFlags content_status() const {
    return content_status_;
  }
  CertStatus cert_status() const { return cert_status_; }
  scoped_refptr<net::X509Certificate> cert() const;

 private:
  SecurityLevel security_level_;
  content::SSLStatus::ContentStatusFlags content_status_;
  CertStatus cert_status_;
  scoped_refptr<net::X509Certificate> cert_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_
