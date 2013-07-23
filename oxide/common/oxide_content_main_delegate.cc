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
#include "content/public/common/content_switches.h"
#include "content/public/renderer/content_renderer_client.h"

#include "browser/oxide_browser_process_main.h"

#include "oxide_content_client.h"

namespace {
base::LazyInstance<content::ContentRendererClient> g_content_renderer_client =
    LAZY_INSTANCE_INITIALIZER;
}

static void* module_handle;

namespace oxide {

content::ContentBrowserClient*
ContentMainDelegate::CreateContentBrowserClient() {
  DCHECK(ContentClient::IsBrowser()) <<
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

  ContentClient::SetIsBrowser(
    !CommandLine::ForCurrentProcess()->HasSwitch(switches::kProcessType));

  ContentClient::SetBasicStartupComplete(true);

  return false;
}

void ContentMainDelegate::PreSandboxStartup() {
  // base::FILE_MODULE doesn't work correctly on Linux (it returns the
  // same as base::FILE_EXE, which is not what we want in the browser process)
  // We will use this to find the renderer binary
  Dl_info info;
  int rv = dladdr(module_handle, &info);
  DCHECK_EQ(rv, 0) << "Failed to determine module path";

  PathService::Override(base::FILE_MODULE,
                        base::FilePath(info.dli_fname));
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
  ContentClient::SetBasicStartupComplete(false);

  if (process_type.empty()) {
    BrowserProcessMain::ShutdownBrowserProcess();
  }
}

} // namespace oxide
