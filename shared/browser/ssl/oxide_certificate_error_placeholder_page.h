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

#ifndef _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PLACEHOLDER_PAGE_H_
#define _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PLACEHOLDER_PAGE_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/interstitial_page_delegate.h"

#include "shared/common/oxide_shared_export.h"

class GURL;

namespace content {
class InterstitialPage;
class WebContents;
}

namespace oxide {

class CertificateErrorProxy;

// This class inserts a placeholder transient page in to a WebContents'
// navigation list whilst an application displays a certificate error UI
class CertificateErrorPlaceholderPage
    : public content::InterstitialPageDelegate {
 public:
  CertificateErrorPlaceholderPage(content::WebContents* contents,
                                  const GURL& request_url,
                                  CertificateErrorProxy* error);

  void Show();

  void Proceed();
  void DontProceed();

  base::WeakPtr<CertificateErrorPlaceholderPage> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  OXIDE_SHARED_EXPORT static void SetDontCreateViewForTesting(
      bool dont_create_view);

 private:
  ~CertificateErrorPlaceholderPage() override;

  // content::InterstitialPageDelegate implementation
  std::string GetHTMLContents() override;
  void OnDontProceed() override;

  content::InterstitialPage* interstitial_; // This owns us

  scoped_refptr<CertificateErrorProxy> error_;

  base::WeakPtrFactory<CertificateErrorPlaceholderPage> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(CertificateErrorPlaceholderPage);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SSL_CERTIFICATE_ERROR_PLACEHOLDER_PAGE_H_
