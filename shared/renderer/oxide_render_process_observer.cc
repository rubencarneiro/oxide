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

#include "oxide_render_process_observer.h"

#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "net/base/net_module.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_net_resource_provider.h"

namespace {
bool g_inject_oxide_js_in_main_world = false;
}

namespace oxide {

void RenderProcessObserver::OnSetUserAgent(const std::string& user_agent) {
  ContentClient::instance()->SetUserAgent(user_agent);
}

void RenderProcessObserver::OnInjectOxideJsExtensionsInMainWorld(
      bool inject_oxide_js_in_main_world) {
  g_inject_oxide_js_in_main_world = inject_oxide_js_in_main_world;
}

bool RenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RenderProcessObserver, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SetUserAgent, OnSetUserAgent)
    IPC_MESSAGE_HANDLER(OxideMsg_InjectOxideJsExtensionsInMainWorld,
			OnInjectOxideJsExtensionsInMainWorld)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void RenderProcessObserver::OnRenderProcessShutdown() {
  content::RenderThread::Get()->RemoveObserver(this);
}

RenderProcessObserver::RenderProcessObserver() {
  net::NetModule::SetResourceProvider(NetResourceProvider);
  content::RenderThread::Get()->AddObserver(this);
}

// static
bool RenderProcessObserver::InjectOxideJsExtensionsInMainWorld() {
  return g_inject_oxide_js_in_main_world;
}

} // namespace oxide
