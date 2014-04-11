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

#include "oxide_script_message_request_impl_browser.h"

#include "content/browser/frame_host/frame_tree_node.h"
#include "content/public/browser/render_frame_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"

namespace oxide {

bool ScriptMessageRequestImplBrowser::DoSendMessage(
    const OxideMsg_SendMessage_Params& params) {
 
  content::FrameTreeNode* node = frame_->GetFrameTreeNode();
  content::RenderFrameHost* rfh = node->current_frame_host();
  return rfh->Send(new OxideMsg_SendMessage(rfh->GetRoutingID(), params));
}

void ScriptMessageRequestImplBrowser::OnReply(const std::string& args) {
  if (!reply_callback_.is_null()) {
    reply_callback_.Run(args);
  }
}

void ScriptMessageRequestImplBrowser::OnError(
    ScriptMessageRequest::Error error,
    const std::string& msg) {
  if (!error_callback_.is_null()) {
    error_callback_.Run(error, msg);
  }
}

ScriptMessageRequestImplBrowser::ScriptMessageRequestImplBrowser(
    WebFrame* frame,
    int serial,
    const GURL& context,
    bool want_reply,
    const std::string& msg_id,
    const std::string& args) :
    ScriptMessageRequest(serial, context, want_reply, msg_id, args),
    frame_(frame->GetWeakPtr()) {
  frame_->AddScriptMessageRequest(this);
}

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
