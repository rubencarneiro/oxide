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

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "content/public/common/user_agent.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/renderer/oxide_content_renderer_client.h"

#if defined(ENABLE_PLUGINS)
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "content/public/common/pepper_plugin_info.h"
#include "content/public/common/webplugininfo.h"
#include "content/public/common/content_constants.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_paths.h"
#endif

#include "oxide_user_agent.h"

namespace oxide {

namespace {
ContentClient* g_instance;
}

void ContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
#if defined(ENABLE_PLUGINS)
  if (PathService::Get(FILE_PEPPER_FLASH_PLUGIN, &path)) {
    content::PepperPluginInfo pf;

    pf.path = path;
    pf.is_out_of_process = true;
    pf.name = content::kFlashPluginName;
    pf.permissions = ppapi::PERMISSION_DEV |
                        ppapi::PERMISSION_PRIVATE |
                        ppapi::PERMISSION_BYPASS_USER_GESTURE |
                        ppapi::PERMISSION_FLASH;

    pf.description = "Shockwave Flash Pepper Plugin (under Oxide)";
    pf.mime_types.push_back(content::WebPluginMimeType(content::kFlashPluginSwfMimeType,
                                           content::kFlashPluginSwfExtension,
                                           content::kFlashPluginSwfDescription));
    pf.mime_types.push_back(content::WebPluginMimeType(content::kFlashPluginSplMimeType,
                                           content::kFlashPluginSplExtension,
                                           content::kFlashPluginSplDescription));
    plugins->push_back(pf);
  }
#endif
}

std::string ContentClient::GetUserAgent() const {
  return oxide::GetUserAgent();
}

base::string16 ContentClient::GetLocalizedString(int message_id) const {
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece ContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedStaticMemory* ContentClient::GetDataResourceBytes(
    int resource_id) const {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

// static
ContentClient* ContentClient::GetInstance() {
  DCHECK(g_instance);
  return g_instance;
}

ContentClient::ContentClient() {
  DCHECK(!g_instance);
  g_instance = this;
}

ContentClient::~ContentClient() {
  DCHECK_EQ(g_instance, this);
  g_instance = nullptr;
}

} // namespace oxide
