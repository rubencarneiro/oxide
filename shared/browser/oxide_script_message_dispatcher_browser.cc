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

#include "oxide_script_message_dispatcher_browser.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_script_message_handler.h"
#include "shared/common/oxide_script_message_request.h"

#include "oxide_script_message_impl_browser.h"
#include "oxide_script_message_request_impl_browser.h"
#include "oxide_script_message_target.h"
#include "oxide_web_frame.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

class MessageReceiver {
 public:
  MessageReceiver(int render_process_id, const IPC::Message& message) {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&MessageReceiver::ReceiveMessageOnUIThread,
                   base::Owned(this),
                   render_process_id,
                   message));
  }

  ~MessageReceiver() {}

  void ReceiveMessageOnUIThread(int render_process_id,
                                const IPC::Message& message) {
    content::RenderFrameHost* rfh =
        content::RenderFrameHost::FromID(render_process_id,
                                         message.routing_id());
    if (!rfh) {
      return;
    }

    OxideHostMsg_SendMessage::Param p;
    if (!OxideHostMsg_SendMessage::Read(&message, &p)) {
      rfh->GetProcess()->ShutdownForBadMessage();
      return;
    }

    ScriptMessageParams params(std::move(base::get<0>(p)));

    bool is_reply = params.type == ScriptMessageParams::TYPE_REPLY;

    WebFrame* frame = WebFrame::FromRenderFrameHost(rfh);
    if (!frame) {
      rfh->GetProcess()->ShutdownForBadMessage();
      return;
    }

    if (!is_reply) {
      scoped_refptr<ScriptMessageImplBrowser> message(
          new ScriptMessageImplBrowser(frame,
                                       params.serial,
                                       params.context,
                                       params.type == ScriptMessageParams::TYPE_MESSAGE,
                                       params.msg_id,
                                       &params.wrapped_payload));
      WebFrame* target = frame;
      WebView* view = frame->view();

      while (target) {
        DCHECK_EQ(target->view(), view);
        if (TryDispatchMessageToTarget(target, message.get())) {
          break;
        }

        target = target->parent();
      }

      if (!target && !TryDispatchMessageToTarget(view, message.get())) {
        message->Error(ScriptMessageParams::ERROR_NO_HANDLER);
      }

      return;
    }

    for (WebFrame::ScriptMessageRequestVector::const_iterator it =
          frame->current_script_message_requests().begin();
         it != frame->current_script_message_requests().end(); ++it) {
      ScriptMessageRequestImplBrowser* request = *it;
      if (request->serial() == params.serial &&
          request->IsWaitingForResponse()) {
        request->OnReceiveResponse(&params.wrapped_payload, params.error);
        return;
      }
    }
  }

 private:
  bool TryDispatchMessageToTarget(
      ScriptMessageTarget* target, ScriptMessageImplBrowser* message) {
    for (size_t i = 0; i < target->GetScriptMessageHandlerCount(); ++i) {
      const ScriptMessageHandler* handler =
          target->GetScriptMessageHandlerAt(i);

      if (!handler->IsValid()) {
        continue;
      }

      if (handler->msg_id() != message->msg_id()) {
        continue;
      }

      const std::vector<GURL>& contexts = handler->contexts();

      for (std::vector<GURL>::const_iterator it = contexts.begin();
           it != contexts.end(); ++it) {
        if ((*it) == message->context()) {
          handler->OnReceiveMessage(message);
          return true;
        }
      }
    }

    return false;
  }
};

} // namespace

void ScriptMessageDispatcherBrowser::OnReceiveScriptMessage(
    const IPC::Message& message) {
  new MessageReceiver(render_process_id_, message);
}

bool ScriptMessageDispatcherBrowser::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ScriptMessageDispatcherBrowser, message)
    IPC_MESSAGE_HANDLER_GENERIC(OxideHostMsg_SendMessage,
                                OnReceiveScriptMessage(message))
    IPC_MESSAGE_UNHANDLED(handled = false)
    (void)param__;
  IPC_END_MESSAGE_MAP()

  return handled;
}

ScriptMessageDispatcherBrowser::ScriptMessageDispatcherBrowser(
    content::RenderProcessHost* render_process_host) :
    content::BrowserMessageFilter(OxideMsgStart),
    render_process_id_(render_process_host->GetID()) {}

ScriptMessageDispatcherBrowser::~ScriptMessageDispatcherBrowser() {}

} // namespace oxide
