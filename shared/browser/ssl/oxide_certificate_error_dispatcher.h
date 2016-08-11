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

#ifndef _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_DISPATCHER_H_
#define _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_DISPATCHER_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/common/resource_type.h"

#include "shared/common/oxide_shared_export.h"

class GURL;

namespace net {
class SSLInfo;
}

namespace oxide {

class CertificateError;

// A helper class for dispatching certificate errors from Chromium to
// the provided callback
class OXIDE_SHARED_EXPORT CertificateErrorDispatcher
    : public content::WebContentsUserData<CertificateErrorDispatcher> {
 public:
  ~CertificateErrorDispatcher();

  static CertificateErrorDispatcher* FromWebContents(
      content::WebContents* contents);

  static void CreateForWebContents(content::WebContents* contents);

  // Entry point from Chromium
  static void AllowCertificateError(
      content::WebContents* contents,
      bool is_main_frame,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      const base::Callback<void(content::CertificateRequestResultType)>& callback);

  using Callback = base::Callback<void(std::unique_ptr<CertificateError>)>;

  void SetCallback(const Callback& callback);

 private:
  CertificateErrorDispatcher();

  bool CanDispatch() const;
  void Dispatch(std::unique_ptr<CertificateError> error);

  Callback callback_;

  DISALLOW_COPY_AND_ASSIGN(CertificateErrorDispatcher);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_DISPATCHER_H_
