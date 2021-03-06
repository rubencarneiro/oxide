// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_HANDLER_P_P_H_
#define _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_HANDLER_P_P_H_

#include <QJSValue>
#include <QScopedPointer>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_script_message_handler_proxy_client.h"

class OxideQQuickScriptMessageHandler;

namespace oxide {
namespace qt {
class ScriptMessageHandlerProxy;
}
}

class OxideQQuickScriptMessageHandlerPrivate
    : public oxide::qt::ScriptMessageHandlerProxyClient {
  Q_DECLARE_PUBLIC(OxideQQuickScriptMessageHandler)

 public:
  OxideQQuickScriptMessageHandlerPrivate(OxideQQuickScriptMessageHandler* q);

  bool isActive();

  static OxideQQuickScriptMessageHandlerPrivate* get(
      OxideQQuickScriptMessageHandler* message_handler);

 private:
  // oxide::qt::ScriptMessageHandlerProxyClient implementation
  bool ReceiveMessage(oxide::qt::ScriptMessageProxy* message,
                      QVariant* error) override;

  OxideQQuickScriptMessageHandler* q_ptr;

  QScopedPointer<oxide::qt::ScriptMessageHandlerProxy> proxy_;

  QJSValue callback_;

  Q_DISABLE_COPY(OxideQQuickScriptMessageHandlerPrivate);
};

#endif // _OXIDE_QT_QUICK_API_SCRIPT_MESSAGE_HANDLER_P_P_H_
