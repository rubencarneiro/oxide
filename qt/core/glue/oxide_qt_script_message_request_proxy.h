// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_REQUEST_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_REQUEST_PROXY_H_

#include "qt/core/glue/oxide_qt_proxy_handle.h"

namespace oxide {
namespace qt {

class ScriptMessageRequest;
class ScriptMessageRequestProxyClient;

class Q_DECL_EXPORT ScriptMessageRequestProxy {
  OXIDE_Q_DECL_PROXY_FOR(ScriptMessageRequest);
 public:
  static ScriptMessageRequestProxy* create(
      ScriptMessageRequestProxyClient* client);
  virtual ~ScriptMessageRequestProxy();
};

OXIDE_Q_DECL_PROXY_HANDLE(ScriptMessageRequestProxy);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_REQUEST_PROXY_H_
