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

#include "base/strings/stringprintf.h"
#include "content/public/common/user_agent.h"
#include "ui/base/l10n/l10n_util.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/common/chrome_version.h"
#include "shared/renderer/oxide_content_renderer_client.h"

namespace oxide {

namespace {
ContentClient* g_inst;
}

std::string ContentClient::GetUserAgent() const {
  return content::BuildUserAgentFromProduct(
      base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING));
}

base::string16 ContentClient::GetLocalizedString(int message_id) const {
  return l10n_util::GetStringUTF16(message_id);
}

ContentClient::ContentClient() {
  DCHECK(!g_inst);
  g_inst = this;
}

// static
ContentClient* ContentClient::instance() {
  DCHECK(g_inst);
  return g_inst;
}

ContentClient::~ContentClient() {
  DCHECK_EQ(g_inst, this);
  g_inst = NULL;
}

ContentBrowserClient* ContentClient::browser() {
  return static_cast<ContentBrowserClient *>(
      content::ContentClient::browser());
}

ContentRendererClient* ContentClient::renderer() {
  return static_cast<ContentRendererClient *>(
      content::ContentClient::renderer());
}

} // namespace oxide
