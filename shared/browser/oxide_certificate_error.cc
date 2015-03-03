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

#include <algorithm>

#include "base/logging.h"
#include "net/base/net_errors.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/ssl_info.h"

#include "oxide_web_frame.h"

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

class CertificateErrorManager::IteratorGuard {
 public:
  IteratorGuard(CertificateErrorManager* manager);
  ~IteratorGuard();

 private:
  base::WeakPtr<CertificateErrorManager> manager_;
  bool iterating_original_;

  DISALLOW_COPY_AND_ASSIGN(IteratorGuard);
};

CertificateErrorManager::IteratorGuard::IteratorGuard(
    CertificateErrorManager* manager)
    : manager_(manager->weak_factory_.GetWeakPtr()),
      iterating_original_(manager->iterating_) {
  manager->iterating_ = true;
}

CertificateErrorManager::IteratorGuard::~IteratorGuard() {
  if (!manager_) {
    return;
  }

  manager_->iterating_ = iterating_original_;
  if (!manager_->iterating_) {
    manager_->Compact();
  }
}

void CertificateErrorManager::AddError(CertificateError* error) {
  DCHECK_EQ(error->manager_, this);
  DCHECK(std::find(errors_.begin(), errors_.end(), error) == errors_.end());

  errors_.push_back(error);
}

void CertificateErrorManager::RemoveError(CertificateError* error) {
  auto it = std::find(errors_.begin(), errors_.end(), error);
  DCHECK(it != errors_.end());
  if (iterating_) {
    *it = nullptr;
  } else {
    errors_.erase(it);
  }

  error->manager_ = nullptr;
  error->frame_ = nullptr;
}

void CertificateErrorManager::Compact() {
  DCHECK(!iterating_);

  errors_.erase(
      std::remove(errors_.begin(), errors_.end(), nullptr),
      errors_.end());
}

void CertificateErrorManager::CancelPendingFrameErrorsForFrame(
    WebFrame* frame) {
  IteratorGuard guard(this);

  for (auto it = errors_.begin(); it != errors_.end(); ++it) {
    CertificateError* error = *it;
    if (!error) {
      continue;
    }

    if (error->frame_ != frame) {
      continue;
    }

    if (error->is_subresource()) {
      continue;
    }

    if (error->non_overridable_frame_error_committed_) {
      continue;
    }

    RemoveError(error);
    error->Cancel();
  }
}

CertificateErrorManager::CertificateErrorManager()
    : iterating_(false),
      weak_factory_(this) {}

CertificateErrorManager::~CertificateErrorManager() {
  IteratorGuard guard(this);
  for (auto it = errors_.begin(); it != errors_.end(); ++it) {
    CertificateError* error = *it;
    if (!error) {
      continue;
    }

    RemoveError(error);
    error->Cancel();
  }
}

void CertificateErrorManager::DidStartProvisionalLoadForFrame(
    WebFrame* frame) {
  DCHECK(frame);
  CancelPendingFrameErrorsForFrame(frame);
}

void CertificateErrorManager::DidNavigateFrame(WebFrame* frame) {
  DCHECK(frame);

  IteratorGuard guard(this);

  for (auto it = errors_.begin(); it != errors_.end(); ++it) {
    CertificateError* error = *it;
    if (!error) {
      continue;
    }

    if (error->frame_ != frame) {
      continue;
    }

    if (!error->is_subresource() &&
        !error->non_overridable_frame_error_committed_) {
      error->non_overridable_frame_error_committed_ = true;
      continue;
    }

    RemoveError(error);
    error->Cancel();
  }
}

void CertificateErrorManager::DidStopProvisionalLoadForFrame(WebFrame* frame) {
  DCHECK(frame);
  CancelPendingFrameErrorsForFrame(frame);
}

void CertificateErrorManager::FrameDetached(WebFrame* frame) {
  DCHECK(frame);

  IteratorGuard guard(this);

  for (auto it = errors_.begin(); it != errors_.end(); ++it) {
    CertificateError* error = *it;
    if (!error) {
      continue;
    }

    if (error->frame_ != frame) {
      continue;
    }

    RemoveError(error);
    error->Cancel();
  }
}

void CertificateError::Cancel() {
  DCHECK(!is_cancelled_);
  DCHECK(!callback_.is_null() || !overridable_);

  is_cancelled_ = true;

  if (!callback_.is_null()) {
    callback_.Run(false);
    callback_.Reset();
  }

  if (!cancel_callback_.is_null()) {
    cancel_callback_.Run();
    cancel_callback_.Reset();
  }
}

CertificateError::CertificateError(
    CertificateErrorManager* manager,
    WebFrame* frame,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& url,
    content::ResourceType resource_type,
    bool strict_enforcement,
    const base::Callback<void(bool)>& callback)
    : manager_(manager),
      frame_(frame),
      is_main_frame_(!frame->parent()),
      is_subresource_(!content::IsResourceTypeFrame(resource_type)),
      cert_error_(ToCertError(cert_error, ssl_info.cert.get())),
      cert_(ssl_info.cert),
      url_(url),
      overridable_(!callback.is_null()),
      strict_enforcement_(strict_enforcement),
      callback_(callback),
      is_cancelled_(false),
      non_overridable_frame_error_committed_(false) {
  DCHECK(manager_);
  DCHECK(frame_);
  CHECK(!overridable_ || !strict_enforcement_) <<
      "overridable and strict_enforcement are expected to be mutually exclusive";

  manager_->AddError(this);
}

CertificateError::~CertificateError() {
  if (!callback_.is_null()) {
    Deny();
  } else if (manager_) {
    manager_->RemoveError(this);
  }
}

void CertificateError::SetCancelCallback(const base::Closure& callback) {
  DCHECK(!is_cancelled_);
  cancel_callback_ = callback;
}

void CertificateError::Allow() {
  DCHECK(!callback_.is_null());

  callback_.Run(true);
  callback_.Reset();

  manager_->RemoveError(this);
}

void CertificateError::Deny() {
  DCHECK(!callback_.is_null());

  callback_.Run(false);
  callback_.Reset();

  manager_->RemoveError(this);
}

} // namespace oxide
