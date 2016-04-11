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

#include "oxide_certificate_error_proxy.h"

#include "base/logging.h"

#include "oxide_certificate_error_placeholder_page.h"

namespace oxide {

CertificateErrorProxy::~CertificateErrorProxy() {
  DCHECK(did_respond_ || is_cancelled_);
}

CertificateErrorProxy::CertificateErrorProxy(
    const base::Callback<void(bool)>& callback)
    : callback_(callback),
      is_cancelled_(false),
      did_respond_(false),
      placeholder_page_(nullptr) {}

void CertificateErrorProxy::Allow() {
  DCHECK(!did_respond_);
  DCHECK(!is_cancelled_);
  DCHECK(!callback_.is_null());

  did_respond_ = true;

  callback_.Run(true);
  callback_.Reset();

  if (placeholder_page_) {
    placeholder_page_->Proceed();
    placeholder_page_ = nullptr;
  }
}

void CertificateErrorProxy::Deny() {
  DCHECK(!did_respond_);
  DCHECK(!is_cancelled_);

  did_respond_ = true;

  if (!callback_.is_null()) {
    callback_.Run(false);
    callback_.Reset();
  }

  if (placeholder_page_) {
    placeholder_page_->DontProceed();
    placeholder_page_ = nullptr;
  }
}

void CertificateErrorProxy::Cancel() {
  DCHECK(!is_cancelled_);

  if (did_respond_) {
    // Calling Deny() ends up re-entering here via
    // InterstitialPageDelegate::OnDontProceed
    return;
  }

  is_cancelled_ = true;
  placeholder_page_ = nullptr;

  if (!callback_.is_null()) {
    callback_.Run(false);
    callback_.Reset();
  }

  if (!cancel_callback_.is_null()) {
    cancel_callback_.Run();
    cancel_callback_.Reset();
  }
}

void CertificateErrorProxy::SetPlaceholderPage(
    CertificateErrorPlaceholderPage* placeholder) {
  DCHECK(!did_respond_);
  DCHECK(!is_cancelled_);
  placeholder_page_ = placeholder;
}

} // namespace oxide
