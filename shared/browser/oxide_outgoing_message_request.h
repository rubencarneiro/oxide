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

#ifndef _OXIDE_SHARED_BROWSER_OUTGOING_MESSAGE_REQUEST_H_
#define _OXIDE_SHARED_BROWSER_OUTGOING_MESSAGE_REQUEST_H_

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_message_dispatcher_browser.h"

namespace oxide {

class OutgoingMessageRequest FINAL {
 public:
  typedef base::Callback<void(const std::string&)> ReplyCallback;
  typedef base::Callback<void(int, const std::string&)> ErrorCallback;

  OutgoingMessageRequest();

  int serial() const {
    return serial_;
  }
  void set_serial(int serial) {
    serial_ = serial;
  }

  bool IsWaiting() const;

  void SetReplyCallback(const ReplyCallback& callback);
  void SetErrorCallback(const ErrorCallback& callback);

  void OnReceiveResponse(const MessageDispatcherBrowser::V8Response& response);
  void SendError(int error, const std::string& msg);

 private:
  int serial_;
  bool had_response_;
  ReplyCallback reply_callback_;
  ErrorCallback error_callback_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_OUTGOING_MESSAGE_REQUEST_H_
