// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

namespace oxide {
class SecurityStatus;
namespace qt {
class WebView;
}
}

class OxideQSecurityStatus;

class OxideQSecurityStatusPrivate Q_DECL_FINAL {
  Q_DECLARE_PUBLIC(OxideQSecurityStatus)

 public:
  ~OxideQSecurityStatusPrivate();

  static OxideQSecurityStatusPrivate* get(OxideQSecurityStatus* q);

  void Init(oxide::qt::WebView* view);
  void Update(const oxide::SecurityStatus& old);

 private:
  OxideQSecurityStatusPrivate(OxideQSecurityStatus* q);

  OxideQSecurityStatus* q_ptr;
  oxide::qt::WebView* web_view_; 
};

#endif // _OXIDE_QT_CORE_API_SECURITY_STATUS_P_H_
