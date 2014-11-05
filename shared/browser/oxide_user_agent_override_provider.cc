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

#include "oxide_user_agent_override_provider.h"

#include "content/public/browser/render_process_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"

namespace oxide {

bool UserAgentOverrideProvider::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(UserAgentOverrideProvider, message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_GetUserAgentOverride, OnGetUserAgentOverride)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void UserAgentOverrideProvider::OnGetUserAgentOverride(const GURL& url,
                                                       std::string* user_agent,
                                                       bool* overridden) {
  scoped_refptr<BrowserContextDelegate> delegate(
      BrowserContextIOData::FromResourceContext(context_)->GetDelegate());
  if (!delegate.get()) {
    *overridden = false;
    return;
  }

  *overridden = delegate->GetUserAgentOverride(url, user_agent);
}

UserAgentOverrideProvider::UserAgentOverrideProvider(
    content::RenderProcessHost* render_process_host) :
    content::BrowserMessageFilter(OxideMsgStart),
    context_(render_process_host->GetBrowserContext()->GetResourceContext()) {}

UserAgentOverrideProvider::~UserAgentOverrideProvider() {}

} // namespace oxide
