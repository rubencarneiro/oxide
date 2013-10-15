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

#include "oxide_message_dispatcher_browser.h"

#include "content/public/browser/web_contents.h"

#include "shared/common/oxide_messages.h"

#include "oxide_message_handler.h"
#include "oxide_message_target.h"
#include "oxide_outgoing_message_request.h"
#include "oxide_web_frame.h"
#include "oxide_web_view.h"

namespace oxide {

MessageDispatcherBrowser::V8Message::V8Message(
    WebView* web_view,
    MessageDispatcherBrowser* dispatcher,
    const OxideMsg_SendMessage_Params& params) :
    frame(web_view->FindFrameWithID(params.frame_id)),
    world_id(params.world_id),
    serial(params.serial),
    msg_id(params.msg_id),
    args(params.args),
    dispatcher(dispatcher) {}

MessageDispatcherBrowser::V8Response::V8Response(
    const OxideMsg_SendMessage_Params& params) :
    error(params.error),
    param(params.args) {}

void MessageDispatcherBrowser::SendErrorForMessage(
    const V8Message& message,
    OxideMsg_SendMessage_Error::Value error_code,
    const std::string& error_desc) {
  OxideMsg_SendMessage_Params params;
  params.frame_id = message.frame->identifier();
  params.world_id = message.world_id;
  params.serial = message.serial;
  params.type = OxideMsg_SendMessage_Type::Reply;
  params.error = error_code;
  params.msg_id = message.msg_id;
  params.args = error_desc;

  Send(new OxideMsg_SendMessage(routing_id(), params));
}

bool MessageDispatcherBrowser::TryDispatchToTarget(MessageTarget* target,
                                                   const V8Message& message) {
  for (size_t i = 0; i < target->GetMessageHandlerCount(); ++i) {
    MessageHandler* handler = target->GetMessageHandlerAt(i);

    if (!handler->IsValid()) {
      continue;
    }

    if (handler->msg_id() != message.msg_id) {
      continue;
    }

    const std::vector<std::string>& world_ids = handler->world_ids();

    for (std::vector<std::string>::const_iterator it = world_ids.begin();
         it != world_ids.end(); ++it) {
      std::string world_id = *it;

      if (world_id == message.world_id) {
        handler->OnReceiveMessage(message);
        return true;
      }
    }
  }

  return false;
}

void MessageDispatcherBrowser::OnReceiveMessage(
    const OxideMsg_SendMessage_Params& params) {
  content::WebContents* web_contents =
      content::WebContents::FromRenderViewHost(render_view_host());
  DCHECK(web_contents);

  WebView* web_view = WebView::FromWebContents(web_contents);

  if (params.type == OxideMsg_SendMessage_Type::Message) {
    V8Message message(web_view, this, params);

    if (web_contents->GetRenderViewHost() != render_view_host()) {
      SendErrorForMessage(message,
                          OxideMsg_SendMessage_Error::UNDELIVERABLE,
                          "RenderViewHost is not current");
      return;
    }

    if (!message.frame) {
      SendErrorForMessage(message,
                          OxideMsg_SendMessage_Error::INVALID_DESTINATION,
                          "Invalid frame ID");
      return;
    }

    if (!TryDispatchToTarget(message.frame, message)) {
      if (!TryDispatchToTarget(message.frame->GetView(), message)) {
        SendErrorForMessage(message,
                            OxideMsg_SendMessage_Error::NO_HANDLER,
                            "No handler was found for message");
      }
    }

    return; 
  }

  WebFrame* frame = web_view->FindFrameWithID(params.frame_id);
  if (!frame) {
    return;
  }

  for (size_t i = 0; i < frame->GetOutgoingMessageRequestCount(); ++i) {
    OutgoingMessageRequest* request = frame->GetOutgoingMessageRequestAt(i);

    if (request->serial() == params.serial) {
      V8Response response(params);
      request->OnReceiveResponse(response);
      return;
    }
  }
}

MessageDispatcherBrowser::MessageDispatcherBrowser(
    content::RenderViewHost* rvh) :
    content::RenderViewHostObserver(rvh),
    weak_factory_(this) {}

base::WeakPtr<MessageDispatcherBrowser> MessageDispatcherBrowser::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

bool MessageDispatcherBrowser::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(MessageDispatcherBrowser, message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_SendMessage, OnReceiveMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

} // namespace oxide
