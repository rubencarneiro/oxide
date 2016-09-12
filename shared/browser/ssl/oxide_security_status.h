// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_
#define _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_

#include <memory>

#include "base/callback.h"
#include "base/callback_list.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/common/ssl_status.h"

#include "shared/browser/ssl/oxide_security_types.h"
#include "shared/common/oxide_shared_export.h"

namespace net {
class X509Certificate;
}

namespace oxide {

// Tracks the security state for a WebContents
class OXIDE_SHARED_EXPORT SecurityStatus
    : public content::WebContentsUserData<SecurityStatus> {
 public:
  ~SecurityStatus();

  static void CreateForWebContents(content::WebContents* contents);
  static SecurityStatus* FromWebContents(content::WebContents* contents);

  // Notification from content
  void VisibleSSLStateChanged();

  enum ChangedFlags {
    CHANGED_FLAG_NONE = 0,
    CHANGED_FLAG_SECURITY_LEVEL = 1 << 0,
    CHANGED_FLAG_CONTENT_STATUS = 1 << 1,
    CHANGED_FLAG_CERT_STATUS = 1 << 2,
    CHANGED_FLAG_CERT = 1 << 3
  };

  using ObserverCallback = base::Callback<void(ChangedFlags)>;
  using Subscription = base::CallbackList<void(ChangedFlags)>::Subscription;

  std::unique_ptr<Subscription> AddChangeCallback(
      const ObserverCallback& callback);

  SecurityLevel security_level() const { return security_level_; }
  content::SSLStatus::ContentStatusFlags content_status() const {
    return content_status_;
  }
  CertStatusFlags cert_status() const { return cert_status_; }
  net::X509Certificate* cert() const { return cert_.get(); }

 private:
  friend class content::WebContentsUserData<SecurityStatus>;

  SecurityStatus(content::WebContents* contents);

  content::WebContents* contents_;

  SecurityLevel security_level_;
  content::SSLStatus::ContentStatusFlags content_status_;
  CertStatusFlags cert_status_;
  scoped_refptr<net::X509Certificate> cert_;

  base::CallbackList<void(ChangedFlags)> callback_list_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SECURITY_STATUS_H_
