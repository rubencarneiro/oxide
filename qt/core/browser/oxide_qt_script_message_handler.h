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

#ifndef _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_
#define _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_

#include <string>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/glue/oxide_qt_script_message_handler_proxy.h"
#include "shared/common/oxide_script_message_handler.h"

namespace base {
class Value;
}

namespace oxide {

class ScriptMessage;

namespace qt {

class ScriptMessageHandlerProxyClient;

class ScriptMessageHandler : public ScriptMessageHandlerProxy {
 public:
  ScriptMessageHandler(ScriptMessageHandlerProxyClient* client,
                       QObject* handle);
  ~ScriptMessageHandler() override;

  const oxide::ScriptMessageHandler* handler() const { return &handler_; }

 private:
  bool ReceiveMessageCallback(oxide::ScriptMessage* message,
                              scoped_ptr<base::Value>* error_payload);

  // ScriptMessageHandlerProxy implementation
  QString msgId() const override;
  void setMsgId(const QString& id) override;
  QList<QUrl> contexts() const override;
  void setContexts(const QList<QUrl>& contexts) override;
  void attachHandler() override;
  void detachHandler() override;

  ScriptMessageHandlerProxyClient* client_;
  oxide::ScriptMessageHandler handler_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageHandler);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_SCRIPT_MESSAGE_HANDLER_H_
