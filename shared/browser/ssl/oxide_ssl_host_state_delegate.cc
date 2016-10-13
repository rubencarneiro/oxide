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

#include "oxide_ssl_host_state_delegate.h"

namespace oxide {

SSLHostStateDelegate::SSLHostStateDelegate() {}

SSLHostStateDelegate::~SSLHostStateDelegate() {}

void SSLHostStateDelegate::AllowCert(const std::string& host,
                                     const net::X509Certificate& cert,
                                     net::CertStatus error) {}

void SSLHostStateDelegate::Clear(
    const base::Callback<bool(const std::string&)>& host_filter) {}

content::SSLHostStateDelegate::CertJudgment SSLHostStateDelegate::QueryPolicy(
    const std::string& host,
    const net::X509Certificate& cert,
    net::CertStatus error,
    bool* expired_previous_decision) {
  return DENIED;
}

void SSLHostStateDelegate::HostRanInsecureContent(
    const std::string& host,
    int pid,
    InsecureContentType content_type) {
  // We need this because SSLPolicy::UpdateEntry uses the response of
  // DidHostRunInsecureContent to set the appropriate content status
  // XXX: We should clear out processes as they die
  switch (content_type) {
    case MIXED_CONTENT:
      ran_insecure_content_hosts_.insert(BrokenHostEntry(host, pid));
      break;
    case CERT_ERRORS_CONTENT:
      ran_content_with_cert_errors_hosts_.insert(BrokenHostEntry(host, pid));
      break;
  }
}

bool SSLHostStateDelegate::DidHostRunInsecureContent(
    const std::string& host,
    int pid,
    InsecureContentType content_type) const {
  switch (content_type) {
    case MIXED_CONTENT:
      return ran_insecure_content_hosts_.count(BrokenHostEntry(host, pid)) != 0;
    case CERT_ERRORS_CONTENT: {
      BrokenHostEntry entry(host, pid);
      return ran_content_with_cert_errors_hosts_.count(entry) != 0;
    }
  }

  NOTREACHED();
  return true;
}

void SSLHostStateDelegate::RevokeUserAllowExceptions(const std::string& host) {}

bool SSLHostStateDelegate::HasAllowException(const std::string& host) const {
  return false;
}

} // namespace oxide
