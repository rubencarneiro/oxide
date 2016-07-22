// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxide_content_main_delegate.h"

#include <string>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/rtl.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/sys_info.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/content_renderer_client.h"
#include "content/public/utility/content_utility_client.h"
#include "third_party/skia/include/core/SkGraphics.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"

#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_content_browser_client.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_paths.h"
#include "shared/renderer/oxide_content_renderer_client.h"

#include "oxide_platform_delegate.h"

namespace oxide {

ContentMainDelegate::ContentMainDelegate(PlatformDelegate* delegate)
    : delegate_(delegate) {
  CHECK(delegate_);
}

ContentMainDelegate::~ContentMainDelegate() {}

bool ContentMainDelegate::BasicStartupComplete(int* exit_code) {
  content_client_.reset(new ContentClient());
  content::SetContentClient(content_client_.get());
  RegisterPathProvider();

  return false;
}

void ContentMainDelegate::PreSandboxStartup() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  base::FilePath dir_exe;
  PathService::Get(base::DIR_EXE, &dir_exe);

  // Override the default locales/ directory - we have to install these in
  // to a different place to make GN happy
  PathService::Override(ui::DIR_LOCALES,
                        dir_exe.Append(FILE_PATH_LITERAL("chromium_l10n")));

  if (command_line->HasSwitch(switches::kSharedMemoryOverridePath)) {
    base::PathService::Override(
        base::DIR_SHARED_MEMORY,
        command_line->GetSwitchValuePath(switches::kSharedMemoryOverridePath));
  }

  std::string app_locale;
  if (command_line->HasSwitch(switches::kLang)) {
    app_locale = command_line->GetSwitchValueASCII(switches::kLang);
  } else {
    app_locale = delegate_->GetApplicationLocale();
  }
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      base::i18n::GetCanonicalLocale(app_locale),
      nullptr,
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide.pak")),
      ui::SCALE_FACTOR_NONE);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide_100_percent.pak")),
      ui::SCALE_FACTOR_100P);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide_200_percent.pak")),
      ui::SCALE_FACTOR_200P);

  // Limit the Skia font cache to match the Android behaviour rather than
  // Chrome desktop. Having a large cache isn't ideal when we could have half
  // a dozen apps open that embed us
  const int kMB = 1024 * 1024;
  size_t font_cache_limit =
      base::SysInfo::IsLowEndDevice() ? kMB : 8 * kMB;
  SkGraphics::SetFontCacheLimit(font_cache_limit);
}

int ContentMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    // We arrive here if some calls the renderer with no --process-type
    LOG(ERROR) << "The Oxide renderer cannot be used to run a browser process";
    return 1;
  }

  return -1;
}

void ContentMainDelegate::ProcessExiting(const std::string& process_type) {
  DCHECK(!process_type.empty());
}

content::ContentBrowserClient*
ContentMainDelegate::CreateContentBrowserClient() {
  CHECK(BrowserProcessMain::GetInstance()->IsRunning());
  DCHECK(!content_browser_client_);
  content_browser_client_.reset(
      new ContentBrowserClient(delegate_->GetApplicationLocale(),
                               delegate_->CreateBrowserIntegration()));

  return content_browser_client_.get();
}

content::ContentRendererClient*
ContentMainDelegate::CreateContentRendererClient() {
  DCHECK(!content_renderer_client_);
  content_renderer_client_.reset(new ContentRendererClient());

  return content_renderer_client_.get();
}

content::ContentUtilityClient*
ContentMainDelegate::CreateContentUtilityClient() {
  DCHECK(!content_utility_client_);
  content_utility_client_.reset(new content::ContentUtilityClient());

  return content_utility_client_.get();
}

} // namespace oxide
