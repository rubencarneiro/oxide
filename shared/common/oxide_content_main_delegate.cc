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
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_paths.h"
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

  return false;
}

void ContentMainDelegate::PreSandboxStartup() {
  // base::FILE_MODULE doesn't work correctly on Linux (it returns the
  // same as base::FILE_EXE, which is not what we want in the browser process)
  // We will use this to find the renderer binary
  Dl_info info;
  int rv = dladdr(reinterpret_cast<void *>(BrowserProcessMain::Exists),
                  &info);
  DCHECK_NE(rv, 0) << "Failed to determine module path";

  PathService::Override(base::FILE_MODULE,
                        base::FilePath(info.dli_fname));

  // We assume that the renderer and other resources are at
  // |./oxide-<port_name>/| relative to the public library
  base::FilePath support_path;
  PathService::Get(base::DIR_MODULE, &support_path);

  support_path =
      support_path.Append(FILE_PATH_LITERAL(OXIDE_RESOURCE_SUBPATH));
  PathService::Override(
      content::CHILD_PROCESS_EXE,
      support_path.Append(FILE_PATH_LITERAL(OXIDE_SUBPROCESS)));

  // The locale passed here doesn't matter, as there aren't any
  // localized resources to load
  ui::ResourceBundle::InitSharedInstanceLocaleOnly("en-US", NULL);

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      support_path.Append(FILE_PATH_LITERAL("oxide.pak")),
      ui::SCALE_FACTOR_NONE);
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      support_path.Append(FILE_PATH_LITERAL("oxide_100_percent.pak")),
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
