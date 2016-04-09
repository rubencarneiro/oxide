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

#ifndef _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_H_
#define _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "url/gurl.h"

#include "shared/browser/oxide_security_types.h"
#include "shared/common/oxide_shared_export.h"

namespace net {
class X509Certificate;
}

namespace oxide {

class CertificateErrorProxy;

// Represents a SSL certificate error. It provides access to the relevant
// information about the error, as well as methods to accept or deny it
class OXIDE_SHARED_EXPORT CertificateError {
 public:
  CertificateError(bool is_main_frame,
                   bool is_subresource,
                   CertError cert_error,
                   net::X509Certificate* cert,
                   const GURL& url,
                   bool strict_enforcement,
                   bool overridable,
                   CertificateErrorProxy* proxy);
  ~CertificateError();

  // If the error is from the main frame
  bool is_main_frame() const { return is_main_frame_; }

  // Whether the error is for a subresource of the frame
  bool is_subresource() const { return is_subresource_; }

  // The certificate error type
  CertError cert_error() const { return cert_error_; }

  // The certificate that this error is for
  net::X509Certificate* cert() const { return cert_.get(); }

  // The request URL that triggered the error
  const GURL& url() const { return url_; }

  // Whether the error is overridable
  bool overridable() const { return overridable_; }

  // Whether the error was generated from a URL that uses HSTS
  bool strict_enforcement() const { return strict_enforcement_; }

  // Whether the error has been cancelled by Oxide. This is only relevant
  // for main frame errors (is_main_frame_ && !is_subresource_) and is used
  // to signal to the application when it should hide its certificate error UI
  bool IsCancelled() const;

  // Set a callback to be invoked when this error is cancelled
  void SetCancelCallback(const base::Closure& callback);

  // Allow the request that generated the error to continue, ignoring the
  // error. Only possible if IsOverridable() returns true
  void Allow();

  // Cancel the request that generated the error
  void Deny();

 private:
  bool is_main_frame_;
  bool is_subresource_;
  CertError cert_error_;
  scoped_refptr<net::X509Certificate> cert_;
  GURL url_;
  bool strict_enforcement_;
  bool overridable_;

  scoped_refptr<CertificateErrorProxy> proxy_;

  DISALLOW_COPY_AND_ASSIGN(CertificateError);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_H_
