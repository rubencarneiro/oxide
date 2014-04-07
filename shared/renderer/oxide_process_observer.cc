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

#include "oxide_process_observer.h"

#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "net/base/net_module.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_net_resource_provider.h"

namespace oxide {

namespace {
bool g_is_off_the_record = true;
}

void ProcessObserver::OnSetIsIncognitoProcess(bool incognito) {
  g_is_off_the_record = incognito;
}

ProcessObserver::ProcessObserver() {
  net::NetModule::SetResourceProvider(NetResourceProvider);
  content::RenderThread::Get()->AddObserver(this);
}

bool ProcessObserver::OnControlMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ProcessObserver, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SetIsIncognitoProcess, OnSetIsIncognitoProcess)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ProcessObserver::OnRenderProcessShutdown() {
  content::RenderThread::Get()->RemoveObserver(this);
}

// static
bool ProcessObserver::IsOffTheRecord() {
  return g_is_off_the_record;
}

} // namespace oxide
