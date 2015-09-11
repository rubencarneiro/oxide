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

#include "oxide_script_message_request_impl_browser.h"

#include "oxide_web_frame.h"

namespace oxide {

void ScriptMessageRequestImplBrowser::OnReply(const base::Value& payload) {
  if (reply_callback_.is_null()) {
    return;
  }

  reply_callback_.Run(payload);
}

void ScriptMessageRequestImplBrowser::OnError(
    ScriptMessageParams::Error error,
    const base::Value& payload) {
  if (error_callback_.is_null()) {
    return;
  }

  error_callback_.Run(error, payload);
}

ScriptMessageRequestImplBrowser::ScriptMessageRequestImplBrowser(
    WebFrame* frame,
    int serial)
    : ScriptMessageRequest(serial),
      frame_(frame->GetWeakPtr()) {}

ScriptMessageRequestImplBrowser::~ScriptMessageRequestImplBrowser() {
  if (frame_) {
    frame_->RemoveScriptMessageRequest(this);
  }
}

void ScriptMessageRequestImplBrowser::SetReplyCallback(
    const ReplyCallback& callback) {
  reply_callback_ = callback;
}

void ScriptMessageRequestImplBrowser::SetErrorCallback(
    const ErrorCallback& callback) {
  error_callback_ = callback;
}

} // namespace oxide
