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

#ifndef _OXIDE_SHARED_BROWSER_CERTIFICATE_ERROR_H_
#define _OXIDE_SHARED_BROWSER_CERTIFICATE_ERROR_H_

#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/common/resource_type.h"
#include "url/gurl.h"

#include "shared/browser/oxide_render_frame_host_id.h"
#include "shared/browser/oxide_security_types.h"

namespace content {
class RenderFrameHost;
}

namespace net {
class SSLInfo;
class X509Certificate;
}

namespace oxide {

class CertificateError;

// This class tracks CertificateErrors
class CertificateErrorManager {
 public:
  CertificateErrorManager();
  ~CertificateErrorManager();

  // Cancels any pending frame errors for |frame|
  void DidStartProvisionalLoadForFrame(content::RenderFrameHost* frame);

  // Cancels any errors for |frame| with the exception of
  // non-overridable frame errors if this is the corresponding error page
  void DidNavigateFrame(content::RenderFrameHost* frame);

  // Cancels any pending frame errors for |frame|
  void DidStopProvisionalLoadForFrame(content::RenderFrameHost* frame);

  // Cancels all errors for frame
  void FrameDetached(content::RenderFrameHost* frame);

 private:
  friend class CertificateError;
  class IteratorGuard;

  // Add a CertificateError
  void AddError(CertificateError* error);

  // Remove a CertificateError, clearing its pointers to this
  // and the frame that generated the error
  void RemoveError(CertificateError* error);

  // Remove empty slots from errors_
  void Compact();

  void CancelPendingFrameErrorsForFrame(content::RenderFrameHost* frame);

  typedef std::vector<CertificateError*> CertErrorVector;

  // Used to indicate that errors_ is being iterated over, and
  // will prevent RemoveError from removing entries from it
  bool iterating_;

  // The list of CertificateErrors
  CertErrorVector errors_;

  base::WeakPtrFactory<CertificateErrorManager> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(CertificateErrorManager);
};

// Represents a SSL certificate error. It provides access to the relevant
// information about the error, as well as methods to accept or deny it
class CertificateError {
 public:
  CertificateError(
      CertificateErrorManager* manager,
      content::RenderFrameHost* frame,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& url,
      content::ResourceType resource_type,
      bool strict_enforcement,
      const base::Callback<void(bool)>& callback);
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

  // Whether the error has been cancelled by Oxide. A cancelled error is no
  // longer relevant. This can happen if the frame navigates or goes away
  bool is_cancelled() const { return is_cancelled_; }

  // Set a callback to be invoked when this error is cancelled
  void SetCancelCallback(const base::Closure& callback);

  // Allow the request that generated the error to continue, ignoring the
  // error. Only possible if overridable() returns true
  void Allow();

  // Cancel the request that generated the error
  void Deny();

 private:
  friend class CertificateErrorManager;

  // Cancel the request that generated the error and run the cancel callback.
  // This is only called from CertificateErrorManager
  void Cancel();

  CertificateErrorManager* manager_;

  // The ID of the frame the generated this error
  RenderFrameHostID frame_id_;

  bool is_main_frame_;
  bool is_subresource_;
  CertError cert_error_;
  scoped_refptr<net::X509Certificate> cert_;
  GURL url_;
  bool overridable_;
  bool strict_enforcement_;

  // The callback provided by Chromium, which we use to respond to the error
  base::Callback<void(bool)> callback_;

  bool is_cancelled_;
  base::Closure cancel_callback_;

  // Subresource errors are cancelled when a frame is committed.
  // However, non-overridable frame errors should persist beyond the error
  // page commit for this failed load and then be cancelled on a subsequent
  // commit. This is used for tracking that
  bool non_overridable_frame_error_committed_;

  DISALLOW_COPY_AND_ASSIGN(CertificateError);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_CERTIFICATE_ERROR_H_
