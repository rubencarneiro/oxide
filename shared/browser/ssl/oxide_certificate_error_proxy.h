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

#ifndef _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PROXY_H_
#define _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PROXY_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

class GURL;

namespace content {
class WebContents;
}

namespace oxide {

class CertificateErrorPlaceholderPage;

// This class sits between CertificateError and CertificateErrorPlaceholderPage
class CertificateErrorProxy : public base::RefCounted<CertificateErrorProxy> {
 public:
  CertificateErrorProxy(const base::Callback<void(bool)>& callback);

  bool is_cancelled() const { return is_cancelled_; }
  bool did_respond() const { return did_respond_; }

  void set_cancel_callback(const base::Closure& cancel_callback) {
    cancel_callback_ = cancel_callback;
  }

  void Allow();
  void Deny();

  void Cancel();

  void AttachPlaceholderPage(content::WebContents* contents,
                             const GURL& request_url);

 private:
  friend class base::RefCounted<CertificateErrorProxy>;
  ~CertificateErrorProxy();

  base::Callback<void(bool)> callback_;

  bool is_cancelled_;
  bool did_respond_;
  base::Closure cancel_callback_;

  base::WeakPtr<CertificateErrorPlaceholderPage> placeholder_page_;

  DISALLOW_COPY_AND_ASSIGN(CertificateErrorProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PROXY_H_
