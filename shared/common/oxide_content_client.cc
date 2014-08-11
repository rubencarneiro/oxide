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
#include "shared/common/chrome_version.h"
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

namespace oxide {

ContentClient::ContentClient() {}

void ContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
#if defined(ENABLE_PLUGINS)
  base::CommandLine& cmd_line = *base::CommandLine::ForCurrentProcess();
  bool enable_gtalk = cmd_line.HasSwitch(switches::kEnableGoogleTalkPlugin);
  base::FilePath path;
  if (enable_gtalk && PathService::Get(FILE_O1D_PLUGIN, &path)) {
    content::PepperPluginInfo o1d;
    o1d.path = path;
    o1d.name = "Google Talk Plugin Video Renderer";
    o1d.is_out_of_process = true;
    o1d.is_sandboxed = false;
    o1d.permissions = ppapi::PERMISSION_PRIVATE | ppapi::PERMISSION_DEV;
    content::WebPluginMimeType o1d_mime_type("application/o1d",
                                             "",
                                             "Google Talk Plugin Video Renderer");
    o1d.mime_types.push_back(o1d_mime_type);

    plugins->push_back(o1d);
  }

  if (enable_gtalk && PathService::Get(FILE_GTALK_PLUGIN, &path)) {
    content::PepperPluginInfo gtalk;
    gtalk.path = path;
    gtalk.name = "Google Talk Plugin";
    gtalk.is_out_of_process = true;
    gtalk.is_sandboxed = false;
    gtalk.permissions = ppapi::PERMISSION_PRIVATE | ppapi::PERMISSION_DEV;
    content::WebPluginMimeType gtalk_mime_type("application/googletalk",
                                               ".googletalk",
                                               "Google Talk Plugin");
    gtalk.mime_types.push_back(gtalk_mime_type);

    plugins->push_back(gtalk);
  }

  // Pepper Flash
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
  return user_agent_;
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
  return Singleton<ContentClient>::get();
}

// static
ContentClient* ContentClient::instance() {
  return GetInstance();
}

ContentClient::~ContentClient() {}

ContentBrowserClient* ContentClient::browser() {
  return static_cast<ContentBrowserClient *>(
      content::ContentClient::browser());
}

ContentRendererClient* ContentClient::renderer() {
  return static_cast<ContentRendererClient *>(
      content::ContentClient::renderer());
}

void ContentClient::SetUserAgent(const std::string& user_agent) {
  user_agent_ = user_agent;
}

} // namespace oxide
