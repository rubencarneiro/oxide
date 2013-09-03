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

#ifndef _OXIDE_SHARED_BROWSER_MESSAGE_HANDLER_H_
#define _OXIDE_SHARED_BROWSER_MESSAGE_HANDLER_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"

#include "oxide_message_dispatcher_browser.h"

namespace oxide {

class IncomingMessage;

class MessageHandler FINAL {
 public:
  typedef base::Callback<void(IncomingMessage*)> HandlerCallback;

  MessageHandler();

  std::string msg_id() const {
    return msg_id_;
  }
  void set_msg_id(const std::string& id) {
    msg_id_ = id;
  }

  void SetCallback(const HandlerCallback& callback);

  void OnReceiveMessage(const MessageDispatcherBrowser::V8Message& message);

 private:
  std::string msg_id_;
  HandlerCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MESSAGE_HANDLER_H_
