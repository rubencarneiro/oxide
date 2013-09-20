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

#include <dlfcn.h>
#include <string>

#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/content_renderer_client.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"

#include "shared/browser/oxide_browser_process_main.h"
#include "shared/renderer/oxide_content_renderer_client.h"

#include "oxide_content_client.h"

namespace oxide {

namespace {
base::LazyInstance<oxide::ContentRendererClient> g_content_renderer_client =
    LAZY_INSTANCE_INITIALIZER;
}

content::ContentBrowserClient*
ContentMainDelegate::CreateContentBrowserClient() {
  DCHECK(BrowserProcessMain::Exists()) <<
    "Creating a browser client in a non-browser process";
  return CreateContentBrowserClientImpl();
}

content::ContentRendererClient*
ContentMainDelegate::CreateContentRendererClient() {
  return g_content_renderer_client.Pointer();
}

ContentMainDelegate::~ContentMainDelegate() {}

bool ContentMainDelegate::BasicStartupComplete(int* exit_code) {
  content::SetContentClient(ContentClient::GetInstance());

  CommandLine* command_line = CommandLine::ForCurrentProcess();

  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type.empty()) {
    const char* renderer_cmd_prefix = getenv("OXIDE_RENDERER_CMD_PREFIX");
    if (renderer_cmd_prefix) {
      command_line->AppendSwitchASCII(switches::kRendererCmdPrefix,
                                      renderer_cmd_prefix);
    }

    if (getenv("OXIDE_NO_SANDBOX")) {
      command_line->AppendSwitch(switches::kNoSandbox);
    }
  }

  return false;
}

void ContentMainDelegate::PreSandboxStartup() {
  base::FilePath resource_dir;
  const char* resource_path = getenv("OXIDE_RESOURCE_PATH");
  if (resource_path) {
    resource_dir = base::FilePath(resource_path);
  } else {
    Dl_info info;
    int rv = dladdr(reinterpret_cast<void *>(BrowserProcessMain::Exists),
                    &info);
    DCHECK_NE(rv, 0) << "Failed to determine module path";

    resource_dir =
        base::FilePath(info.dli_fname).DirName().Append(
          FILE_PATH_LITERAL(OXIDE_RESOURCE_SUBPATH));
  }

  base::FilePath renderer =
      resource_dir.Append(FILE_PATH_LITERAL(OXIDE_SUBPROCESS));

  std::string process_type =
      CommandLine::ForCurrentProcess()->
        GetSwitchValueASCII(switches::kProcessType);
  if (process_type.empty()) {
    PathService::Override(base::FILE_EXE, renderer);
  }

  // The locale passed here doesn't matter, as there aren't any
  // localized resources to load
  ui::ResourceBundle::InitSharedInstanceLocaleOnly("en-US", NULL);

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      resource_dir.Append(FILE_PATH_LITERAL("oxide.pak")),
      ui::SCALE_FACTOR_NONE);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      resource_dir.Append(FILE_PATH_LITERAL("oxide_100_percent.pak")),
      ui::SCALE_FACTOR_100P);
}

int ContentMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    return BrowserProcessMain::RunBrowserProcess(main_function_params);
  }

  return -1;
}

void ContentMainDelegate::ProcessExiting(const std::string& process_type) {
  if (process_type.empty()) {
    BrowserProcessMain::ShutdownBrowserProcess();
  }
}

} // namespace oxide
