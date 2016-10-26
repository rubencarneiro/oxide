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

#ifndef _OXIDE_SHARED_BROWSER_SSL_HOST_STATE_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_SSL_HOST_STATE_DELEGATE_H_

#include <set>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/public/browser/ssl_host_state_delegate.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class OXIDE_SHARED_EXPORT SSLHostStateDelegate
    : public content::SSLHostStateDelegate {
 public:
  SSLHostStateDelegate();
  ~SSLHostStateDelegate();

  // content::SSLHostStateDelegate implementation
  void AllowCert(const std::string&,
                 const net::X509Certificate& cert,
                 net::CertStatus error) override;
  void Clear(
      const base::Callback<bool(const std::string&)>& host_filter) override;
  CertJudgment QueryPolicy(
      const std::string& host,
      const net::X509Certificate& cert,
      net::CertStatus error,
      bool* expired_previous_decision) override;
  void HostRanInsecureContent(const std::string& host,
                              int pid,
                              InsecureContentType content_type) override;
  bool DidHostRunInsecureContent(
      const std::string& host,
      int pid,
      InsecureContentType content_type) const override;
  void RevokeUserAllowExceptions(const std::string& host) override;
  bool HasAllowException(const std::string& host) const override;
  
 private:
  typedef std::pair<std::string, int> BrokenHostEntry;
  std::set<BrokenHostEntry> ran_insecure_content_hosts_;
  std::set<BrokenHostEntry> ran_content_with_cert_errors_hosts_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_HOST_STATE_DELEGATE_H_
