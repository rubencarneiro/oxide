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

#include "oxide_qt_security_status.h"

#include "base/bind.h"

#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsecuritystatus_p.h"
#include "shared/common/oxide_enum_flags.h"

namespace oxide {
namespace qt {

namespace {
OXIDE_MAKE_ENUM_BITWISE_OPERATORS(oxide::SecurityStatus::ChangedFlags)
}

void SecurityStatus::OnStatusChanged(
    oxide::SecurityStatus::ChangedFlags flags) {
  if (flags & oxide::SecurityStatus::CHANGED_FLAG_SECURITY_LEVEL) {
    Q_EMIT api_handle_->securityLevelChanged();
  }
  if (flags & oxide::SecurityStatus::CHANGED_FLAG_CONTENT_STATUS) {
    Q_EMIT api_handle_->contentStatusChanged();
  }
  if (flags & oxide::SecurityStatus::CHANGED_FLAG_CERT_STATUS) {
    Q_EMIT api_handle_->certStatusChanged();
  }
  if (flags & oxide::SecurityStatus::CHANGED_FLAG_CERT) {
    OxideQSecurityStatusPrivate::get(api_handle_)->InvalidateCertificate();
    Q_EMIT api_handle_->certificateChanged();
  }
}

SecurityStatus::SecurityStatus(OxideQSecurityStatus* api_handle)
    : api_handle_(api_handle),
      security_status_(nullptr) {}

SecurityStatus::~SecurityStatus() {}

void SecurityStatus::Init(content::WebContents* contents) {
  security_status_ = oxide::SecurityStatus::FromWebContents(contents);
  change_subscription_ = security_status_->AddChangeCallback(
      base::Bind(&SecurityStatus::OnStatusChanged,
                 base::Unretained(this)));
}

oxide::SecurityLevel SecurityStatus::GetSecurityLevel() const {
  if (!security_status_) {
    return oxide::SECURITY_LEVEL_NONE;
  }

  return security_status_->security_level();
}

content::SSLStatus::ContentStatusFlags
SecurityStatus::GetContentStatus() const {
  if (!security_status_) {
    return content::SSLStatus::NORMAL_CONTENT;
  }

  return security_status_->content_status();
}

oxide::CertStatusFlags SecurityStatus::GetCertStatus() const {
  if (!security_status_) {
    return oxide::CERT_STATUS_OK;
  }

  return security_status_->cert_status();
}

net::X509Certificate* SecurityStatus::GetCert() const {
  if (!security_status_) {
    return nullptr;
  }

  return security_status_->cert();
}

} // namespace qt
} // namespace oxide
