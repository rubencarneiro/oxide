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

#include "oxide_script_message_impl_browser.h"

#include "base/memory/weak_ptr.h"
#include "content/browser/frame_host/frame_tree_node.h"
#include "content/public/browser/render_frame_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_web_frame.h"

namespace oxide {

namespace {

const char kSourceFrameKey[] = "source_frame";

class SourceFrameData : public base::SupportsUserData::Data {
 public:
  SourceFrameData(WebFrame* frame) :
      source_frame_(frame->GetWeakPtr()) {}

  WebFrame* source_frame() const { return source_frame_.get(); }

 private:
  base::WeakPtr<WebFrame> source_frame_;
};

}

void ScriptMessageImplBrowser::MakeParams(
    OxideMsg_SendMessage_Params* params) {
  params->context = context().spec();
  params->serial = serial();
  params->type = OxideMsg_SendMessage_Type::Reply;
}

void ScriptMessageImplBrowser::SendResponse(
    const OxideMsg_SendMessage_Params& params) {
  // Check that the frame hasn't gone away
  if (!GetSourceFrame()) {
    return;
  }

  content::RenderFrameHost* rfh =
      GetSourceFrame()->frame_tree_node()->current_frame_host();
  rfh->Send(new OxideMsg_SendMessage(rfh->GetRoutingID(), params));
}

void ScriptMessageImplBrowser::DoSendReply(const std::string& args) {
  OxideMsg_SendMessage_Params params;
  MakeParams(&params);
  params.error = ScriptMessageRequest::ERROR_OK;
  params.payload = args;

  SendResponse(params);
}

void ScriptMessageImplBrowser::DoSendError(ScriptMessageRequest::Error code,
                                           const std::string& msg) {
  OxideMsg_SendMessage_Params params;
  MakeParams(&params);
  params.error = code;
  params.payload = msg;

  SendResponse(params);
}

ScriptMessageImplBrowser::ScriptMessageImplBrowser(WebFrame* source_frame,
                                                   int serial,
                                                   const GURL& context,
                                                   const std::string& msg_id,
                                                   const std::string& args) :
    ScriptMessage(serial, context, msg_id, args) {
  SetUserData(kSourceFrameKey, new SourceFrameData(source_frame));
}

WebFrame* ScriptMessageImplBrowser::GetSourceFrame() const {
  return static_cast<SourceFrameData *>(
      GetUserData(kSourceFrameKey))->source_frame();
}

} // namespace oxide
