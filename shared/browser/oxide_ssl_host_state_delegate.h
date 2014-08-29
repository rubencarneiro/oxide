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

namespace oxide {

class SSLHostStateDelegate FINAL : public content::SSLHostStateDelegate {
 public:
  SSLHostStateDelegate();
  ~SSLHostStateDelegate();

  void DenyCert(const std::string& host,
                net::X509Certificate* cert,
                net::CertStatus error) FINAL;
  void AllowCert(const std::string&,
                 net::X509Certificate* cert,
                 net::CertStatus error) FINAL;
  void Clear() FINAL;

  net::CertPolicy::Judgment QueryPolicy(
      const std::string& host,
      net::X509Certificate* cert,
      net::CertStatus error,
      bool* expired_previous_decision) FINAL;

  void HostRanInsecureContent(const std::string& host, int pid) FINAL;
  bool DidHostRunInsecureContent(const std::string& host,
                                 int pid) const FINAL;

 private:
  typedef std::pair<std::string, int> BrokenHostEntry;
  std::set<BrokenHostEntry> ran_insecure_content_hosts_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_HOST_STATE_DELEGATE_H_
