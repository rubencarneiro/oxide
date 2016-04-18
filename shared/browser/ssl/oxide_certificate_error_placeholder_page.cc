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

#include "oxide_certificate_error_placeholder_page.h"

#include "base/logging.h"
#include "content/public/browser/interstitial_page.h"

#include "oxide_certificate_error_proxy.h"

namespace oxide {

CertificateErrorPlaceholderPage::~CertificateErrorPlaceholderPage() {}

std::string CertificateErrorPlaceholderPage::GetHTMLContents() {
  return "<html></html>";
}

void CertificateErrorPlaceholderPage::OnDontProceed() {
  weak_ptr_factory_.InvalidateWeakPtrs();
  error_->Cancel();
}

CertificateErrorPlaceholderPage::CertificateErrorPlaceholderPage(
    content::WebContents* contents,
    const GURL& request_url,
    CertificateErrorProxy* error)
    : interstitial_(content::InterstitialPage::Create(contents,
                                                      true,
                                                      request_url,
                                                      this)),
      error_(error),
      weak_ptr_factory_(this) {}

void CertificateErrorPlaceholderPage::Show() {
  interstitial_->Show();
}

void CertificateErrorPlaceholderPage::Proceed() {
  interstitial_->Proceed();
}

void CertificateErrorPlaceholderPage::DontProceed() {
  interstitial_->DontProceed();
}

} // namespace oxide
