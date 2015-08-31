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

#ifndef _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_PROXY_CLIENT_H_

#include <QtGlobal>

#include "qt/core/glue/oxide_qt_proxy_handle.h"

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ScriptMessageProxy;

class ScriptMessageHandlerProxyClient {
 public:
  virtual ~ScriptMessageHandlerProxyClient() {}

  virtual bool ReceiveMessage(ScriptMessageProxy* message,
                              QVariant* error) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_SCRIPT_MESSAGE_HANDLER_PROXY_CLIENT_H_
