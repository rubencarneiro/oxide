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

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"

#include "oxide_incoming_message.h"
#include "oxide_message_handler.h"
#include "oxide_message_target.h"
#include "oxide_outgoing_message_request.h"
#include "oxide_web_frame.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

class MessageReceiver {
 public:
  MessageReceiver(int render_process_id, int routing_id) :
      render_process_id_(render_process_id),
      routing_id_(routing_id) {}

  ~MessageReceiver() {}

  void OnReceiveMessage(const OxideMsg_SendMessage_Params& params) {
    params_ = params;

    content::BrowserThread::PostTask(
        content::BrowserThread::UI, FROM_HERE,
        base::Bind(&MessageReceiver::ReceiveMessageOnUIThread,
                   base::Owned(this)));
  }

  void ReceiveMessageOnUIThread() {
    bool is_reply = params_.type == OxideMsg_SendMessage_Type::Reply;

    content::RenderFrameHostImpl* rfh =
        content::RenderFrameHostImpl::FromID(render_process_id_, routing_id_);
    if (!rfh) {
      if (!is_reply) {
        ReturnError(NULL, OxideMsg_SendMessage_Error::INVALID_DESTINATION,
                    "Could not find a RenderFrameHost corresponding to the "
                    "specified routing ID");
      }
      return;
    }

    WebFrame* frame = WebFrame::FromFrameTreeNode(rfh->frame_tree_node());
    if (!frame) {
      if (!is_reply) {
        ReturnError(rfh, OxideMsg_SendMessage_Error::INVALID_DESTINATION,
                    "The RenderFrameHost does not have a corresponding WebFrame");
      }
      return;
    }

    if (!is_reply) {
      if (params_.type != OxideMsg_SendMessage_Type::Message) {
        LOG(ERROR) << "Invalid message type from renderer";
        rfh->GetProcess()->ReceivedBadMessage();
        return;
      }

      WebFrame* target = frame;
      WebView* view = frame->view();

      while (target) {
        DCHECK_EQ(target->view(), view);
        if (TryDispatchMessageToTarget(target, frame)) {
          break;
        }

        target = target->parent();
      }

      if (!target && !TryDispatchMessageToTarget(view, frame)) {
        ReturnError(
            rfh, OxideMsg_SendMessage_Error::NO_HANDLER,
            std::string("Could not find a handler for message"));
      }

      return;
    }

    for (size_t i = 0; i < frame->GetOutgoingMessageRequestCount(); ++i) {
      OutgoingMessageRequest* request = frame->GetOutgoingMessageRequestAt(i);

      if (request->serial() == params_.serial) {
        request->OnReceiveResponse(params_.payload, params_.error);
        return;
      }
    }
  }

 private:
  bool TryDispatchMessageToTarget(MessageTarget* target,
                                  WebFrame* source_frame) {
    GURL context(params_.context);

    for (size_t i = 0; i < target->GetMessageHandlerCount(); ++i) {
      MessageHandler* handler = target->GetMessageHandlerAt(i);

      if (!handler->IsValid()) {
        continue;
      }

      if (handler->msg_id() != params_.msg_id) {
        continue;
      }

      const std::vector<GURL>& contexts = handler->contexts();

      for (std::vector<GURL>::const_iterator it = contexts.begin();
           it != contexts.end(); ++it) {
        if ((*it) == context) {
          handler->OnReceiveMessage(
              new IncomingMessage(source_frame, params_.serial,
                                  context, params_.msg_id,
                                  params_.payload));
          return true;
        }
      }
    }

    return false;
  }

  void ReturnError(content::RenderFrameHost* rfh,
                   OxideMsg_SendMessage_Error::Value type,
                   const std::string& msg) {
    OxideMsg_SendMessage_Params params;
    params.context = params_.context;
    params.serial = params_.serial;
    params.type = OxideMsg_SendMessage_Type::Reply;
    params.msg_id = params_.msg_id;

    params.error = type;
    params.payload = msg;

    content::RenderProcessHost* process = NULL;
    if (rfh) {
      process = rfh->GetProcess();
    }
    if (!process) {
      process = content::RenderProcessHost::FromID(render_process_id_);
    }
    if (!process) {
      return;
    }

    process->Send(new OxideMsg_SendMessage(routing_id_, params));
  }

  OxideMsg_SendMessage_Params params_;

  const int render_process_id_;
  const int routing_id_;
};

} // namespace

MessageDispatcherBrowser::MessageDispatcherBrowser(
    int render_process_id) :
    render_process_id_(render_process_id) {}

MessageDispatcherBrowser::~MessageDispatcherBrowser() {}

bool MessageDispatcherBrowser::OnMessageReceived(const IPC::Message& message,
                                                 bool* message_was_ok) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP_EX(MessageDispatcherBrowser, message, *message_was_ok)
    IPC_MESSAGE_FORWARD(OxideHostMsg_SendMessage,
                        new MessageReceiver(render_process_id_,
                                            message.routing_id()),
                        MessageReceiver::OnReceiveMessage)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  return handled;
}

} // namespace oxide
