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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_MESSAGE_HANDLER_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_MESSAGE_HANDLER_ADAPTER_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_message_handler.h"

namespace oxide {
namespace qt {

class MessageHandlerAdapter;

class MessageHandlerAdapterPrivate FINAL {
 public:
  static MessageHandlerAdapterPrivate* Create(MessageHandlerAdapter* adapter);

  const oxide::MessageHandler& handler() const {
    return handler_;
  }
  oxide::MessageHandler& handler() {
    return handler_;
  }

  base::WeakPtr<MessageHandlerAdapter> GetWeakPtr();

 private:
  MessageHandlerAdapterPrivate(MessageHandlerAdapter* adapter);

  oxide::MessageHandler handler_;
  base::WeakPtrFactory<MessageHandlerAdapter> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(MessageHandlerAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_MESSAGE_HANDLER_ADAPTER_H_
