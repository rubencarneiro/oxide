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

#include "oxide_content_client.h"

#include <algorithm>

#include "base/logging.h"
#include "content/public/common/user_agent.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/browser/oxide_content_browser_client.h"
#include "shared/renderer/oxide_content_renderer_client.h"

#if defined(ENABLE_PLUGINS)
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/path_service.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "base/version.h"
#include "content/public/common/pepper_plugin_info.h"
#include "content/public/common/webplugininfo.h"
#include "content/public/common/content_constants.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

#include "shared/common/oxide_paths.h"
#endif

#include "oxide_form_factor.h"
#include "oxide_user_agent.h"

namespace oxide {

namespace {

ContentClient* g_instance;

#if defined(ENABLE_PLUGINS)
void GetPepperFlashPluginInfo(
    int key,
    std::vector<content::PepperPluginInfo>* plugins) {
  base::FilePath path;
  if (!PathService::Get(key, &path)) {
    return;
  }
  if (!base::PathExists(path)) {
    return;
  }

  base::FilePath manifest_path =
      path.DirName().Append(FILE_PATH_LITERAL("manifest.json"));
  if (!base::PathExists(manifest_path)) {
    return;
  }

  std::string manifest_contents;
  if (!base::ReadFileToString(manifest_path, &manifest_contents)) {
    return;
  }

  scoped_ptr<base::Value> manifest_json =
      base::JSONReader::Read(manifest_contents,
                             base::JSON_ALLOW_TRAILING_COMMAS);
  if (!manifest_json) {
    return;
  }

  base::DictionaryValue* manifest_dict = nullptr;
  if (!manifest_json->GetAsDictionary(&manifest_dict)) {
    return;
  }

  std::string version;
  if (!manifest_dict->GetString("version", &version)) {
    return;
  }

  content::PepperPluginInfo info;

  info.path = path;
  info.is_out_of_process = true;
  info.name = content::kFlashPluginName;
  info.permissions = ppapi::PERMISSION_DEV |
                     ppapi::PERMISSION_PRIVATE |
                     ppapi::PERMISSION_BYPASS_USER_GESTURE |
                     ppapi::PERMISSION_FLASH;

  info.mime_types.push_back(
      content::WebPluginMimeType(content::kFlashPluginSwfMimeType,
                                 content::kFlashPluginSwfExtension,
                                 content::kFlashPluginSwfDescription));
  info.mime_types.push_back(
      content::WebPluginMimeType(content::kFlashPluginSplMimeType,
                                 content::kFlashPluginSplExtension,
                                 content::kFlashPluginSplDescription));

  // Copied from Chrome
  std::vector<std::string> flash_version_numbers =
      base::SplitString(version, ".",
                        base::TRIM_WHITESPACE,
                        base::SPLIT_WANT_NONEMPTY);
  if (flash_version_numbers.size() < 1) {
    flash_version_numbers.push_back("11");
  }
  if (flash_version_numbers.size() < 2) {
    flash_version_numbers.push_back("2");
  }
  if (flash_version_numbers.size() < 3) {
    flash_version_numbers.push_back("999");
  }
  if (flash_version_numbers.size() < 4) {
    flash_version_numbers.push_back("999");
  }
  info.description =
      info.name + " " +
      flash_version_numbers[0] + "." + flash_version_numbers[1] +
      " r" + flash_version_numbers[2];
  info.version = base::JoinString(flash_version_numbers, ".");

  plugins->push_back(info);
}

const std::vector<content::PepperPluginInfo>::const_iterator FindNewestPlugin(
    const std::vector<content::PepperPluginInfo>& plugins) {
  return std::max_element(
      plugins.begin(),
      plugins.end(),
      [](const content::PepperPluginInfo& x,
         const content::PepperPluginInfo& y) {
        Version version_x(x.version);
        DCHECK(version_x.IsValid());
        return version_x.IsOlderThan(y.version);
      });
}
#endif

}

void ContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
#if defined(ENABLE_PLUGINS)
  std::vector<content::PepperPluginInfo> flash_plugins;
  GetPepperFlashPluginInfo(FILE_SYSTEM_PEPPER_FLASH_PLUGIN, &flash_plugins);
  GetPepperFlashPluginInfo(FILE_CHROME_PEPPER_FLASH_PLUGIN, &flash_plugins);

  auto it = FindNewestPlugin(flash_plugins);
  if (it != flash_plugins.end()) {
    plugins->push_back(*it);
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
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
      resource_id);
}

bool ContentClient::ShouldOptimizeForMemoryUsage() const {
  if (GetFormFactorHint() == FORM_FACTOR_DESKTOP) {
    return false;
  }

  return true;
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
