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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_

#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/common/oxide_script_message_request.h"

class GURL;

namespace oxide {

class WebFrame;

class ScriptMessageRequestImplBrowser final : public ScriptMessageRequest {
 public:
  typedef base::Callback<void(const std::string&)> ReplyCallback;
  typedef base::Callback<void(ScriptMessageRequest::Error, const std::string&)> ErrorCallback;

  ScriptMessageRequestImplBrowser(WebFrame* frame,
                                  int serial,
                                  const GURL& context,
                                  bool want_reply,
                                  const std::string& msg_id,
                                  const std::string& args);
  virtual ~ScriptMessageRequestImplBrowser();

  void SetReplyCallback(const ReplyCallback& callback);
  void SetErrorCallback(const ErrorCallback& callback);

 private:
  bool DoSendMessage(const OxideMsg_SendMessage_Params& params) final;

  void OnReply(const std::string& args) final;
  void OnError(Error error, const std::string& msg) final;

  base::WeakPtr<WebFrame> frame_;
  ReplyCallback reply_callback_;
  ErrorCallback error_callback_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(ScriptMessageRequestImplBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
