// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_certificate_error.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "net/cert/x509_certificate.h"

#include "oxide_certificate_error_proxy.h"

namespace oxide {

CertificateError::CertificateError(bool is_main_frame,
                                   bool is_subresource,
                                   CertError cert_error,
                                   net::X509Certificate* cert,
                                   const GURL& url,
                                   bool strict_enforcement,
                                   bool overridable,
                                   CertificateErrorProxy* proxy)
    : is_main_frame_(is_main_frame),
      is_subresource_(is_subresource),
      cert_error_(cert_error),
      cert_(cert),
      url_(url),
      strict_enforcement_(strict_enforcement),
      overridable_(overridable),
      proxy_(proxy) {}

CertificateError::~CertificateError() {
  if (!proxy_->did_respond() && !proxy_->is_cancelled() && overridable_) {
    proxy_->Deny();
  }
}

bool CertificateError::IsCancelled() const {
  return proxy_->is_cancelled();
}

void CertificateError::SetCancelCallback(const base::Closure& callback) {
  proxy_->set_cancel_callback(callback);
}

void CertificateError::Allow() {
  if (!overridable()) {
    LOG(WARNING) << "Can't override a non-overridable error";
    return;
  }

  if (proxy_->is_cancelled()) {
    LOG(WARNING) << "Can't override an error that's been cancelled";
    return;
  }

  if (proxy_->did_respond()) {
    LOG(WARNING) << "Can't respond more than once to a CertificateError";
    return;
  }

  proxy_->Allow();
}

void CertificateError::Deny() {
  if (proxy_->is_cancelled()) {
    LOG(WARNING) << "Can't override an error that's been cancelled";
    return;
  }

  if (proxy_->did_respond()) {
    LOG(WARNING) << "Can't respond more than once to a CertificateError";
    return;
  }

  proxy_->Deny();
}

// static
std::unique_ptr<CertificateError> CertificateError::CreateForTesting(
    bool is_main_frame,
    bool is_subresource,
    CertError cert_error,
    net::X509Certificate* cert,
    const GURL& url,
    bool strict_enforcement,
    bool overridable,
    const base::Callback<void(bool)>& callback) {
  scoped_refptr<CertificateErrorProxy> proxy(
      new CertificateErrorProxy(callback));
  return base::WrapUnique(new CertificateError(is_main_frame,
                                               is_subresource,
                                               cert_error,
                                               cert,
                                               url,
                                               strict_enforcement,
                                               overridable,
                                               proxy.get()));
}

// static
std::unique_ptr<CertificateError>
CertificateError::CreateForTestingWithPlaceholder(
    content::WebContents* contents,
    bool is_main_frame,
    bool is_subresource,
    CertError cert_error,
    net::X509Certificate* cert,
    const GURL& url,
    bool strict_enforcement,
    bool overridable,
    const base::Callback<void(bool)>& callback) {
  scoped_refptr<CertificateErrorProxy> proxy(
      new CertificateErrorProxy(callback));
  proxy->AttachPlaceholderPage(contents, url);
  return base::WrapUnique(new CertificateError(is_main_frame,
                                               is_subresource,
                                               cert_error,
                                               cert,
                                               url,
                                               strict_enforcement,
                                               overridable,
                                               proxy.get()));
}

void CertificateError::SimulateCancel() {
  proxy_->Cancel();
}

} // namespace oxide
