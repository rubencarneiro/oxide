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

#include "oxide_content_main_delegate.h"

#include <string>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/content_renderer_client.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_paths.h"
#include "shared/renderer/oxide_content_renderer_client.h"

namespace oxide {

namespace {
base::LazyInstance<oxide::ContentRendererClient> g_content_renderer_client =
    LAZY_INSTANCE_INITIALIZER;
}

ContentMainDelegate::ContentMainDelegate() {}

ContentMainDelegate::~ContentMainDelegate() {}

SharedGLContext* ContentMainDelegate::GetSharedGLContext() const {
  return NULL;
}

bool ContentMainDelegate::GetNativeDisplay(intptr_t* handle) const {
  return false;
}

blink::WebScreenInfo ContentMainDelegate::GetDefaultScreenInfo() const {
  return blink::WebScreenInfo();
}

#if defined(USE_NSS)
base::FilePath ContentMainDelegate::GetNSSDbPath() const {
  return base::FilePath();
}
#endif

bool ContentMainDelegate::IsPlatformX11() const {
  return false;
}

bool ContentMainDelegate::BasicStartupComplete(int* exit_code) {
  content::SetContentClient(ContentClient::GetInstance());
  RegisterPathProvider();

  return false;
}

void ContentMainDelegate::PreSandboxStartup() {
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      std::string(), NULL,
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);

  base::FilePath dir_exe;
  PathService::Get(base::DIR_EXE, &dir_exe);

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide.pak")),
      ui::SCALE_FACTOR_NONE);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide_100_percent.pak")),
      ui::SCALE_FACTOR_100P);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide_200_percent.pak")),
      ui::SCALE_FACTOR_200P);
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
  NOTREACHED() << "CreateContentBrowserClient() hasn't been implemented";
  return NULL;
}

content::ContentRendererClient*
ContentMainDelegate::CreateContentRendererClient() {
  return g_content_renderer_client.Pointer();
}

} // namespace oxide
