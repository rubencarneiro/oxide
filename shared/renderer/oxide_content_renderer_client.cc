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

#include "oxide_content_renderer_client.h"

#include "oxide_message_dispatcher_renderer.h"
#include "oxide_process_observer.h"
#include "oxide_user_script_scheduler.h"
#include "oxide_user_script_slave.h"

namespace oxide {

ContentRendererClient::ContentRendererClient() {}

ContentRendererClient::~ContentRendererClient() {}

void ContentRendererClient::RenderThreadStarted() {
  process_observer_.reset(new ProcessObserver());
  user_script_slave_.reset(new UserScriptSlave());
}

void ContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new MessageDispatcherRenderer(render_frame);
}

void ContentRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  new UserScriptScheduler(render_view);
}

void ContentRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame,
    v8::Handle<v8::Context> context,
    int extension_group,
    int world_id) {
  MessageDispatcherRenderer::FromWebFrame(
      frame)->DidCreateScriptContext(context, world_id);
}

void ContentRendererClient::WillReleaseScriptContext(
    blink::WebFrame* frame,
    v8::Handle<v8::Context> context,
    int world_id) {
  MessageDispatcherRenderer::FromWebFrame(
      frame)->WillReleaseScriptContext(context, world_id);
}

} // namespace oxide
