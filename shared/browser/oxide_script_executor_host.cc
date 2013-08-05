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

#include "base/logging.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "shared/common/oxide_messages.h"

#include "oxide_script_executor_host.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {
int g_next_request_id = 0;
}

void ScriptExecutorHost::OnExecuteScriptFinished(
    int request_id,
    const std::string& error,
    const base::ListValue& results) {
  PendingRequestMap::iterator it = pending_requests_.find(request_id);
  if (it == pending_requests_.end()) {
    DLOG(WARNING) << "Got an unexpected result";
    return;
  }

  // XXX: Do something with the results?
  it->second.Run(!error.empty(), error);

  pending_requests_.erase(request_id);
}

ScriptExecutorHost::ScriptExecutorHost() :
    content::WebContentsObserver(),
    web_contents_(NULL) {}

void ScriptExecutorHost::BeginObserving(content::WebContents* web_contents) {
  DCHECK(!web_contents_);

  Observe(web_contents);
  web_contents_ = web_contents;
}

void ScriptExecutorHost::ExecuteScript(
    const std::string& code,
    bool all_frames,
    int run_at,
    bool in_main_world,
    const std::string& isolated_world_name,
    const ExecuteScriptCallback& callback) {
  if (in_main_world && !isolated_world_name.empty()) {
    callback.Run(true, "Shouldn't specify an isolated world name when running "
                 "in main world");
    return;
  }

  OxideMsg_ExecuteScript_Params params;
  params.request_id = g_next_request_id++; // XXX: Overflow?
  params.page_id =
      web_contents_->GetController().GetActiveEntry()->GetPageID();
  params.code = code;
  params.all_frames = all_frames;
  params.run_at = run_at;
  params.in_main_world = in_main_world;
  params.isolated_world_name = isolated_world_name;

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh->Send(new OxideMsg_ExecuteScript(rvh->GetRoutingID(), params))) {
    callback.Run(true, "Failed to send message to renderer");
    return;
  }

  pending_requests_[params.request_id] = callback;
}

bool ScriptExecutorHost::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ScriptExecutorHost, message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_ExecuteScriptFinished, OnExecuteScriptFinished)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

} // namespace oxide
