// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_renderer_user_agent_settings.h"

#include "content/public/common/url_utils.h"
#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "url/gurl.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_agent.h"

namespace oxide {

RendererUserAgentSettings::RendererUserAgentSettings() {}

std::string
RendererUserAgentSettings::GetLegacyUserAgentOverrideForURLFromBrowser(
    const GURL& url) {
  // Strip username / password / fragment identifier if they exist
  GURL::Replacements rep;
  rep.ClearUsername();
  rep.ClearPassword();
  rep.ClearRef();

  GURL u = url.ReplaceComponents(rep);

  // URL's longer than GetMaxURLChars can't be serialized.
  // Strip query if we are above the max number of chars
  if (u.spec().size() > content::GetMaxURLChars() &&
      u.has_query()) {
    GURL::Replacements rep;
    rep.ClearQuery();
    u = u.ReplaceComponents(rep);
  }

  // If we are still over, just send the origin
  if (u.spec().size() > content::GetMaxURLChars()) {
    u = u.GetOrigin();
  }

  if (u.spec().size() > content::GetMaxURLChars()) {
    return std::string();
  }

  std::string user_agent;
  content::RenderThread::Get()->Send(
      new OxideHostMsg_GetUserAgentOverride(u, &user_agent));

  return user_agent;
}

void RendererUserAgentSettings::OnSetUserAgent(const std::string& user_agent) {
  SetUserAgent(user_agent);
}

void RendererUserAgentSettings::OnUpdateUserAgentOverrides(
    const std::vector<UserAgentOverrideSet::Entry>& overrides) {
  overrides_.SetOverrides(overrides);
}

bool RendererUserAgentSettings::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RendererUserAgentSettings, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SetUserAgent, OnSetUserAgent)
    IPC_MESSAGE_HANDLER(OxideMsg_UpdateUserAgentOverrides,
                        OnUpdateUserAgentOverrides)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

RendererUserAgentSettings::~RendererUserAgentSettings() {}

std::string RendererUserAgentSettings::GetUserAgentOverrideForURL(
    const GURL& url) {
  std::string override_ua = GetLegacyUserAgentOverrideForURLFromBrowser(url);
  if (override_ua.empty()) {
    override_ua = overrides_.GetOverrideForURL(url);
  }

  return override_ua;
}

} // namespace oxide
