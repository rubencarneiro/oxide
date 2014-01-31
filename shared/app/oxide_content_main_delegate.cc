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
#include <vector>

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
#include "shared/common/oxide_content_client.h"
#include "shared/renderer/oxide_content_renderer_client.h"

namespace oxide {

namespace {
base::LazyInstance<oxide::ContentRendererClient> g_content_renderer_client =
    LAZY_INSTANCE_INITIALIZER;
}

ContentMainDelegate::ContentMainDelegate() {}

content::ContentBrowserClient*
ContentMainDelegate::CreateContentBrowserClient() {
  NOTREACHED() << "CreateContentBrowserClient() hasn't been implemented";
  return NULL;
}

content::ContentRendererClient*
ContentMainDelegate::CreateContentRendererClient() {
  return g_content_renderer_client.Pointer();
}

ContentMainDelegate::~ContentMainDelegate() {}

bool ContentMainDelegate::BasicStartupComplete(int* exit_code) {
  content::SetContentClient(CreateContentClient());

  CommandLine* command_line = CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);

  if (process_type.empty()) {
    // We need to override FILE_EXE in the browser process to the path of the
    // renderer, as various bits of Chrome use this to find other resources
    base::FilePath subprocess_exe;
    const char* subprocess_path = getenv("OXIDE_SUBPROCESS_PATH");
    if (subprocess_path) {
      // Make sure that we have a properly formed absolute path
      // there are some load issues if not.
      subprocess_exe =
          base::MakeAbsoluteFilePath(base::FilePath(subprocess_path));
    } else {
      subprocess_exe = base::FilePath(FILE_PATH_LITERAL(OXIDE_SUBPROCESS_PATH));
      if (!subprocess_exe.IsAbsolute()) {
        Dl_info info;
        int rv = dladdr(reinterpret_cast<void *>(BrowserProcessMain::IsRunning),
                        &info);
        DCHECK_NE(rv, 0) << "Failed to determine module path";

        base::FilePath subprocess_rel(subprocess_exe);
        subprocess_exe = base::FilePath(info.dli_fname).DirName();

        std::vector<base::FilePath::StringType> components;
        subprocess_rel.GetComponents(&components);
        for (size_t i = 0; i < components.size(); ++i) {
          subprocess_exe = subprocess_exe.Append(components[i]);
        }
      }
    }

    PathService::Override(base::FILE_EXE, subprocess_exe);

    // Pick the correct subprocess path
    command_line->AppendSwitchASCII(switches::kBrowserSubprocessPath,
                                    subprocess_exe.value().c_str());

    // This is needed so that we can share GL resources with the embedder
    command_line->AppendSwitch(switches::kInProcessGPU);

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
  // The locale passed here doesn't matter, as there aren't any
  // localized resources to load
  ui::ResourceBundle::InitSharedInstanceLocaleOnly("en-US", NULL);

  base::FilePath dir_exe;
  PathService::Get(base::DIR_EXE, &dir_exe);

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide.pak")),
      ui::SCALE_FACTOR_NONE);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      dir_exe.Append(FILE_PATH_LITERAL("oxide_100_percent.pak")),
      ui::SCALE_FACTOR_100P);
}

int ContentMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    return BrowserProcessMain::RunBrowserMain(main_function_params);
  }

  return -1;
}

void ContentMainDelegate::ProcessExiting(const std::string& process_type) {
  if (process_type.empty()) {
    BrowserProcessMain::ShutdownBrowserMain();
  }
}

} // namespace oxide
