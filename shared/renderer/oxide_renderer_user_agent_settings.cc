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

#include "content/public/renderer/render_thread.h"
#include "ipc/ipc_message_macros.h"
#include "url/gurl.h"
#include "url/url_constants.h"

#include "shared/common/oxide_messages.h"
#include "shared/common/oxide_user_agent.h"

namespace oxide {

RendererUserAgentSettings::RendererUserAgentSettings()
    : legacy_override_enabled_(false) {}

std::string
RendererUserAgentSettings::GetLegacyUserAgentOverrideForURLFromBrowser(
    const GURL& url) {
  if (!legacy_override_enabled_) {
    return std::string();
  }

  // Strip username / password / fragment identifier if they exist
  GURL::Replacements rep;
  rep.ClearUsername();
  rep.ClearPassword();
  rep.ClearRef();

  GURL u = url.ReplaceComponents(rep);

  std::string user_agent;
  content::RenderThread::Get()->Send(
      new OxideHostMsg_GetUserAgentOverride(
          url.ReplaceComponents(rep),
          &user_agent));

  return user_agent;
}

void RendererUserAgentSettings::OnSetUserAgent(const std::string& user_agent) {
  SetUserAgent(user_agent);
}

void RendererUserAgentSettings::OnUpdateUserAgentOverrides(
    const std::vector<UserAgentOverrideSet::Entry>& overrides) {
  overrides_.SetOverrides(overrides);
}

void RendererUserAgentSettings::OnSetLegacyUserAgentOverrideEnabled(
    bool enabled) {
  legacy_override_enabled_ = enabled;
}

bool RendererUserAgentSettings::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RendererUserAgentSettings, message)
    IPC_MESSAGE_HANDLER(OxideMsg_SetUserAgent, OnSetUserAgent)
    IPC_MESSAGE_HANDLER(OxideMsg_UpdateUserAgentOverrides,
                        OnUpdateUserAgentOverrides)
    IPC_MESSAGE_HANDLER(OxideMsg_SetLegacyUserAgentOverrideEnabled,
                        OnSetLegacyUserAgentOverrideEnabled)
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
