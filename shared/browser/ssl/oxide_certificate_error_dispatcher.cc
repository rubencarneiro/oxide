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

#include "oxide_certificate_error_dispatcher.h"

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/certificate_request_result_type.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "net/base/net_errors.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_info.h"
#include "url/gurl.h"

#include "shared/browser/oxide_security_types.h"

#include "oxide_certificate_error.h"
#include "oxide_certificate_error_dispatcher_client.h"
#include "oxide_certificate_error_placeholder_page.h"
#include "oxide_certificate_error_proxy.h"

namespace oxide {

namespace {

CertError ToCertError(int error, net::X509Certificate* cert) {
  DCHECK(net::IsCertificateError(error));

  switch (error) {
    case net::ERR_CERT_COMMON_NAME_INVALID:
      return CERT_ERROR_BAD_IDENTITY;
    case net::ERR_CERT_DATE_INVALID: {
      if (cert && cert->HasExpired()) {
        return CERT_ERROR_EXPIRED;
      }
      return CERT_ERROR_DATE_INVALID;
    }
    case net::ERR_CERT_AUTHORITY_INVALID:
      return CERT_ERROR_AUTHORITY_INVALID;
    case net::ERR_CERT_CONTAINS_ERRORS:
    case net::ERR_CERT_INVALID:
    case net::ERR_CERT_VALIDITY_TOO_LONG:
      return CERT_ERROR_INVALID;
    case net::ERR_CERT_REVOKED:
      return CERT_ERROR_REVOKED;
    case net::ERR_CERT_WEAK_SIGNATURE_ALGORITHM:
    case net::ERR_CERT_WEAK_KEY:
      return CERT_ERROR_INSECURE;
    //case net::ERR_CERT_NO_REVOCATION_MECHANISM:
    //case net::ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
    //case net::ERR_CERT_NON_UNIQUE_NAME:
    //case net::ERR_CERT_NAME_CONSTRAINT_VIOLATION:
    default:
      return CERT_ERROR_GENERIC;
  }
}

}

DEFINE_WEB_CONTENTS_USER_DATA_KEY(CertificateErrorDispatcher);

CertificateErrorDispatcher::CertificateErrorDispatcher()
    : client_(nullptr) {}

bool CertificateErrorDispatcher::CanDispatch() const {
  return !!client_;
}

void CertificateErrorDispatcher::Dispatch(scoped_ptr<CertificateError> error) {
  client_->OnCertificateError(error.Pass());
}

CertificateErrorDispatcher::~CertificateErrorDispatcher() {}

// static
void CertificateErrorDispatcher::CreateForWebContents(
    content::WebContents* contents) {
  DCHECK(contents);
  if (!FromWebContents(contents)) {
    contents->SetUserData(UserDataKey(), new CertificateErrorDispatcher());
  }
}

// static
void CertificateErrorDispatcher::AllowCertificateError(
    int render_process_id,
    int render_frame_id,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(!strict_enforcement || !overridable);

  // Note, CANCEL will stop the resource load associated with the error, and
  // DENY will fail it, resulting in an error page being loaded if it's
  // for the document request

  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!rfh) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  content::WebContents* contents =
      content::WebContents::FromRenderFrameHost(rfh);
  if (!contents) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  CertificateErrorDispatcher* dispatcher = FromWebContents(contents);

  if (!dispatcher->CanDispatch()) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
    return;
  }

  if (resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    // Only errors for main frame document resources are overridable, as it's
    // the only case where we can guarantee that the security status for the
    // webview is correct (see https://launchpad.net/bugs/1368385), and it's
    // the only case where we trust the application to be able to display a
    // meaningful UI
    overridable = false;
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
  } else if (!overridable) {
    // Don't load an error page for main frame document errors, as we're going
    // to load a placeholder transient page
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
  }


  scoped_refptr<CertificateErrorProxy> proxy =
      new CertificateErrorProxy(
        overridable ? callback : base::Callback<void(bool)>());
  scoped_ptr<CertificateError> error(
      new CertificateError(!rfh->GetParent(),
                           !content::IsResourceTypeFrame(resource_type),
                           ToCertError(cert_error, ssl_info.cert.get()),
                           ssl_info.cert.get(),
                           request_url,
                           strict_enforcement,
                           overridable,
                           proxy.get()));

  dispatcher->Dispatch(error.Pass());

  if (proxy->did_respond()) {
    // If the application responded explicitly or by CertificateError being
    // destroyed, there's nothing more to do
    return;
  }

  if (resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    // If this error isn't for a main frame document resource, there's nothing
    // more to do
    return;
  }

  // For main frame document resources, we insert a placeholder transient page
  // as long as CertificateError is alive and not responded to. This is
  // intended to help the embedder when it displays a certificate error UI by
  // ensuring the visible URL is correct, navigation history is consistent, the
  // webview is marked as not loading and the back button behaves correctly
  // (should dismiss the error via the cancel callback and go back to the last
  // committed page)

  // This gets owned by the |contents|
  new CertificateErrorPlaceholderPage(contents,
                                      request_url,
                                      proxy.get());
}

} // namespace oxide
