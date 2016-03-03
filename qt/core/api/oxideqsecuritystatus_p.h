// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_API_SECURITY_STATUS_P_H_
#define _OXIDE_QT_CORE_API_SECURITY_STATUS_P_H_

#include <QtGlobal>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/api/oxideqsslcertificate.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
class SecurityStatus;
namespace qt {
class WebView;
}
}

class OxideQSecurityStatus;
class OxideQSslCertificate;

class OXIDE_QTCORE_EXPORT OxideQSecurityStatusPrivate final {
  Q_DECLARE_PUBLIC(OxideQSecurityStatus)

 public:
  ~OxideQSecurityStatusPrivate();

  static OxideQSecurityStatus* Create();

  static OxideQSecurityStatusPrivate* get(OxideQSecurityStatus* q);

  void Update(const oxide::SecurityStatus& old);

  oxide::qt::WebView* view;

 private:
  OxideQSecurityStatusPrivate(OxideQSecurityStatus* q);

  OxideQSecurityStatus* q_ptr;

  mutable OxideQSslCertificate cert_;
};

#endif // _OXIDE_QT_CORE_API_SECURITY_STATUS_P_H_
