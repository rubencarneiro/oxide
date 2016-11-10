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

#include "oxide_script_message_contents_helper.h"

#include <tuple>
#include <vector>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
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

bool TryDispatchMessageToTarget(ScriptMessageTarget* target,
                                ScriptMessageImplBrowser* message) {
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

void ReturnError(content::RenderFrameHost* render_frame_host,
                 ScriptMessageParams::Error error,
                 const ScriptMessageParams& orig_params) {
  ScriptMessageParams params;
  params.context = orig_params.context;
  params.serial = orig_params.serial;
  params.type = ScriptMessageParams::TYPE_REPLY;
  params.msg_id = orig_params.msg_id;
  params.error = error;

  render_frame_host->Send(
      new OxideMsg_SendMessage(render_frame_host->GetRoutingID(), params));
}

} // namespace

DEFINE_WEB_CONTENTS_USER_DATA_KEY(ScriptMessageContentsHelper);

ScriptMessageContentsHelper::ScriptMessageContentsHelper(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents) {}

void ScriptMessageContentsHelper::OnReceiveScriptMessage(
    const IPC::Message& message,
    content::RenderFrameHost* render_frame_host) {
  OxideHostMsg_SendMessage::Param p;
  if (!OxideHostMsg_SendMessage::Read(&message, &p)) {
    render_frame_host->GetProcess()->ShutdownForBadMessage(
        content::RenderProcessHost::CrashReportMode::GENERATE_CRASH_DUMP);
    return;
  }

  ScriptMessageParams params(std::move(std::get<0>(p)));

  bool is_reply = params.type == ScriptMessageParams::TYPE_REPLY;

  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    ReturnError(render_frame_host,
                ScriptMessageParams::ERROR_NO_HANDLER,
                params);
    return;
  }

  if (!is_reply) {
    scoped_refptr<ScriptMessageImplBrowser> message(
        new ScriptMessageImplBrowser(frame,
                                     params.serial,
                                     params.context,
                                     params.msg_id,
                                     &params.wrapped_payload));
    WebFrame* target = frame;
    WebView* view = frame->GetView();
    if (!view) {
        ReturnError(render_frame_host,
                    ScriptMessageParams::ERROR_NO_HANDLER,
                    params);
        return;
    }

    while (target) {
      DCHECK_EQ(target->GetView(), view);
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

bool ScriptMessageContentsHelper::OnMessageReceived(
    const IPC::Message& message,
    content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ScriptMessageContentsHelper, message)
    IPC_MESSAGE_HANDLER_GENERIC(
        OxideHostMsg_SendMessage,
        OnReceiveScriptMessage(message, render_frame_host))
    IPC_MESSAGE_UNHANDLED(handled = false)
    (void)param__;
  IPC_END_MESSAGE_MAP()

  return handled;
}

ScriptMessageContentsHelper::~ScriptMessageContentsHelper() {}

} // namespace oxide
