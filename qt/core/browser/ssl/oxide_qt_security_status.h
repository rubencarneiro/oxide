// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_BROWSER_SSL_SECURITY_STATUS_H_
#define _OXIDE_QT_CORE_BROWSER_SSL_SECURITY_STATUS_H_

#include <memory>
#include <QPointer>
#include <QtGlobal>

#include "base/macros.h"
#include "content/public/browser/ssl_status.h"

#include "qt/core/common/oxide_qt_export.h"
#include "shared/browser/ssl/oxide_security_status.h"
#include "shared/browser/ssl/oxide_security_types.h"

QT_BEGIN_NAMESPACE
class OxideQSecurityStatus;
QT_END_NAMESPACE

namespace content {
class WebContents;
}

namespace net {
class X509Certificate;
}

namespace oxide {
namespace qt {

// A bridge between oxide::SecurityStatus and OxideQSecurityStatus
class OXIDE_QT_EXPORT SecurityStatus {
 public:
  SecurityStatus(OxideQSecurityStatus* api_handle);
  ~SecurityStatus();

  // Initialize with the specified WebContents
  void Init(content::WebContents* contents);

  oxide::SecurityLevel GetSecurityLevel() const;

  content::SSLStatus::ContentStatusFlags GetContentStatus() const;

  oxide::CertStatusFlags GetCertStatus() const;

  net::X509Certificate* GetCert() const;

 private:
  void OnStatusChanged(oxide::SecurityStatus::ChangedFlags flags);

  QPointer<OxideQSecurityStatus> api_handle_;

  oxide::SecurityStatus* security_status_;

  std::unique_ptr<oxide::SecurityStatus::Subscription> change_subscription_;

  DISALLOW_COPY_AND_ASSIGN(SecurityStatus);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SSL_SECURITY_STATUS_H_
