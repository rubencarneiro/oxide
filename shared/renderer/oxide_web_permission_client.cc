// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_web_permission_client.h"

#include "content/public/renderer/document_state.h"
#include "content/public/renderer/navigation_state.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_message_macros.h"
#include "third_party/WebKit/public/web/WebLocalFrame.h"

#include "shared/common/oxide_messages.h"

namespace oxide {

void WebPermissionClient::DidCommitProvisionalLoad(bool is_new_navigation) {
  blink::WebFrame* frame = render_frame()->GetWebFrame();
  if (frame->parent()) {
    return;
  }

  content::DocumentState* ds =
      content::DocumentState::FromDataSource(frame->dataSource());
  content::NavigationState* ns = ds->navigation_state();
  if (!ns->was_within_same_page() &&
      ds->load_type() != content::DocumentState::LINK_LOAD_RELOAD) {
    did_block_displaying_insecure_content_ = false;
    did_block_running_insecure_content_ = false;

    can_display_insecure_content_ = false;
    can_run_insecure_content_ = false;
  }
}

bool WebPermissionClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WebPermissionClient, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SetAllowDisplayingInsecureContent,
                        OnSetAllowDisplayingInsecureContent)
    IPC_MESSAGE_HANDLER(OxideMsg_SetAllowRunningInsecureContent,
                        OnSetAllowRunningInsecureContent)
    IPC_MESSAGE_HANDLER(OxideMsg_ReloadFrame, OnReloadFrame)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

bool WebPermissionClient::allowDisplayingInsecureContent(
    bool enabled_per_settings,
    const blink::WebSecurityOrigin& origin,
    const blink::WebURL& url) {
  if (enabled_per_settings ||
      can_display_insecure_content_ ||
      can_run_insecure_content_) {
    return true;
  }

  if (!did_block_displaying_insecure_content_) {
    did_block_displaying_insecure_content_ = true;
    Send(new OxideHostMsg_DidBlockDisplayingInsecureContent(routing_id()));
  }

  return false;
}

bool WebPermissionClient::allowRunningInsecureContent(
    bool enabled_per_settings,
    const blink::WebSecurityOrigin& origin,
    const blink::WebURL& url) {
  if (enabled_per_settings || can_run_insecure_content_) {
    return true;
  }

  if (!did_block_running_insecure_content_) {
    did_block_running_insecure_content_ = true;
    Send(new OxideHostMsg_DidBlockRunningInsecureContent(routing_id()));
  }

  return false;
}

void WebPermissionClient::OnSetAllowDisplayingInsecureContent(bool allow) {
  can_display_insecure_content_ = allow;
}

void WebPermissionClient::OnSetAllowRunningInsecureContent(bool allow) {
  can_run_insecure_content_ = allow;
}

void WebPermissionClient::OnReloadFrame() {
  DCHECK(!render_frame()->GetWebFrame()->parent());
  render_frame()->GetWebFrame()->reload();
}

WebPermissionClient::WebPermissionClient(content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame),
      content::RenderFrameObserverTracker<WebPermissionClient>(render_frame),
      can_display_insecure_content_(false),
      can_run_insecure_content_(false),
      did_block_displaying_insecure_content_(false),
      did_block_running_insecure_content_(false) {
  render_frame->GetWebFrame()->setPermissionClient(this);

  // Copy settings from the main frame else we end up in an infinite loop
  // in the case where a subframe tries to display or run insecure content
  // (because the reload deletes the subframes)
  // XXX: This probably doesn't work for out of process frames
  content::RenderFrame* main_frame =
      render_frame->GetRenderView()->GetMainRenderFrame();
  if (main_frame != render_frame) {
    WebPermissionClient* main_client = WebPermissionClient::Get(main_frame);
    can_display_insecure_content_ = main_client->can_display_insecure_content_;
    can_run_insecure_content_ = main_client->can_run_insecure_content_;
  }
}

WebPermissionClient::~WebPermissionClient() {}

} // namespace oxide
