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

#include "oxide_content_client.h"

#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "webkit/common/user_agent/user_agent.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/common/chrome_version.h"
#include "shared/renderer/oxide_content_renderer_client.h"

namespace oxide {

ContentBrowserClient* ContentClient::browser() {
  return static_cast<ContentBrowserClient *>(
      content::ContentClient::browser());
}

ContentRendererClient* ContentClient::renderer() {
  return static_cast<ContentRendererClient *>(
      content::ContentClient::renderer());
}

std::string ContentClient::GetUserAgent() const {
  return webkit_glue::BuildUserAgentFromProduct(
      base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING));
}

// static
ContentClient* ContentClient::GetInstance() {
  // XXX: Port-specific derivates of ContentClient will provide their own
  //      GetInstance() which will be called from here
  return Singleton<ContentClient>::get();
}

} // namespace oxide
