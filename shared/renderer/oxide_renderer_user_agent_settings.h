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

#ifndef _OXIDE_SHARED_RENDERER_USER_AGENT_SETTINGS_H_
#define _OXIDE_SHARED_RENDERER_USER_AGENT_SETTINGS_H_

#include <string>

#include "base/macros.h"
#include "content/public/renderer/render_thread_observer.h"

#include "shared/common/oxide_user_agent_override_set.h"

class GURL;

namespace oxide {

class ContentRendererClient;

class RendererUserAgentSettings : public content::RenderThreadObserver {
 public:
  ~RendererUserAgentSettings() override;

  std::string GetUserAgentOverrideForURL(const GURL& url);

 private:
  friend class ContentRendererClient; // For constructor

  RendererUserAgentSettings();

  std::string GetLegacyUserAgentOverrideForURLFromBrowser(const GURL& url);

  void OnSetUserAgent(const std::string& user_agent);
  void OnUpdateUserAgentOverrides(
      const std::vector<UserAgentOverrideSet::Entry>& overrides);
  void OnSetLegacyUserAgentOverrideEnabled(bool enabled);

  // content::RenderThreadObserver implementation
  bool OnControlMessageReceived(const IPC::Message& message) override;

  bool legacy_override_enabled_;

  UserAgentOverrideSet overrides_;

  DISALLOW_COPY_AND_ASSIGN(RendererUserAgentSettings);
};

} // namespace oxide

#endif // _OXIDE_SHARED_RENDERER_USER_AGENT_SETTINGS_H_
