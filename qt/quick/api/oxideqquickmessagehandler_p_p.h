// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_
#define _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_

#include <QJSValue>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_message_handler.h"

class OxideQIncomingMessage;
class OxideQQuickMessageHandler;

namespace oxide {
class IncomingMessage;
}

class OxideQQuickMessageHandlerPrivate FINAL {
  Q_DECLARE_PUBLIC(OxideQQuickMessageHandler)

 public:
  static OxideQQuickMessageHandlerPrivate* Create(
      OxideQQuickMessageHandler* q);

  const oxide::MessageHandler* handler() const {
    return &handler_;
  }
  oxide::MessageHandler* handler() {
    return &handler_;
  }

  void attachHandler();
  void disconnectHandler();

  void removeFromCurrentOwner();

  static OxideQQuickMessageHandlerPrivate* get(
      OxideQQuickMessageHandler* message_handler);

  QJSValue callback;

 private:
  OxideQQuickMessageHandlerPrivate(OxideQQuickMessageHandler* q);
  void ReceiveMessageCallback(oxide::IncomingMessage* message,
                              bool* delivered,
                              bool* error,
                              std::string* error_desc);

  oxide::MessageHandler handler_;
  base::WeakPtrFactory<OxideQQuickMessageHandlerPrivate> weak_factory_;

  OxideQQuickMessageHandler* q_ptr;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OxideQQuickMessageHandlerPrivate);
};

#endif // _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_
