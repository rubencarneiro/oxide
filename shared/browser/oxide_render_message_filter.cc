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

#include "oxide_render_message_filter.h"

#include "content/public/browser/render_process_host.h"

#include "shared/common/oxide_messages.h"

#include "oxide_browser_context.h"
#include "oxide_browser_context_delegate.h"

namespace oxide {

void RenderMessageFilter::OnGetUserAgentOverride(
    const GURL& url,
    std::string* user_agent) {
  scoped_refptr<BrowserContextDelegate> delegate(
      BrowserContextIOData::FromResourceContext(context_)->GetDelegate());
  if (!delegate.get()) {
    return;
  }

  *user_agent = delegate->GetUserAgentOverride(url);
}

bool RenderMessageFilter::OnMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RenderMessageFilter, message)
    IPC_MESSAGE_HANDLER(OxideHostMsg_GetUserAgentOverride, OnGetUserAgentOverride)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

RenderMessageFilter::RenderMessageFilter(
    content::RenderProcessHost* render_process_host)
    : content::BrowserMessageFilter(OxideMsgStart),
      context_(
          render_process_host->GetBrowserContext()->GetResourceContext()) {}

RenderMessageFilter::~RenderMessageFilter() {}

} // namespace oxide
