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

#ifndef _OXIDE_SHARED_BROWSER_USER_AGENT_OVERRIDE_PROVIDER_H_
#define _OXIDE_SHARED_BROWSER_USER_AGENT_OVERRIDE_PROVIDER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/browser_message_filter.h"

class GURL;

namespace content {
class RenderProcessHost;
class ResourceContext;
}

namespace oxide {

class UserAgentOverrideProvider FINAL : public content::BrowserMessageFilter {
 public:
  UserAgentOverrideProvider(content::RenderProcessHost* render_process_host);
  ~UserAgentOverrideProvider();

 private:
  bool OnMessageReceived(const IPC::Message& message) FINAL;
  void OnGetUserAgentOverride(const GURL& url,
                              std::string* user_agent,
                              bool* overridden);

  content::ResourceContext* context_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(UserAgentOverrideProvider);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_USER_AGENT_OVERRIDE_PROVIDER_H_
