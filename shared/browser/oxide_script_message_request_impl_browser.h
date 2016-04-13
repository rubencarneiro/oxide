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

#ifndef _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
#define _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"

#include "shared/common/oxide_script_message_params.h"
#include "shared/common/oxide_script_message_request.h"
#include "shared/common/oxide_shared_export.h"

class GURL;

namespace base {
class Value;
}

namespace oxide {

class WebFrame;

class OXIDE_SHARED_EXPORT ScriptMessageRequestImplBrowser
    : public ScriptMessageRequest {
 public:
  typedef base::Callback<void(const base::Value&)> ReplyCallback;
  typedef base::Callback<void(ScriptMessageParams::Error, const base::Value&)>
      ErrorCallback;

  ScriptMessageRequestImplBrowser(WebFrame* frame,
                                  int serial);
  ~ScriptMessageRequestImplBrowser() override;

  void SetReplyCallback(const ReplyCallback& callback);
  void SetErrorCallback(const ErrorCallback& callback);

 private:
  void OnReply(const base::Value& payload) override;
  void OnError(ScriptMessageParams::Error error,
               const base::Value& payload) override;

  base::WeakPtr<WebFrame> frame_;
  ReplyCallback reply_callback_;
  ErrorCallback error_callback_;

  DISALLOW_COPY_AND_ASSIGN(ScriptMessageRequestImplBrowser);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_SCRIPT_MESSAGE_REQUEST_H_
