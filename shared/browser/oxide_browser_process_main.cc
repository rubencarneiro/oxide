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

#include "oxide_browser_process_main.h"

#include "base/logging.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/url_util.h"

#include "shared/common/oxide_content_main_delegate.h"

#include "oxide_browser_context.h"
#include "oxide_io_thread_delegate.h"
#include "oxide_message_pump.h"

namespace oxide {

namespace {
BrowserProcessMain* g_process;
}

// static
scoped_refptr<BrowserProcessMain> BrowserProcessMain::GetInstance() {
  if (!g_process) {
    BrowserProcessMain* process = new BrowserProcessMain();
    if (!process->Init()) {
      // This is the only way to delete the failed BrowserProcessMain
      scoped_refptr<BrowserProcessMain> reaper(process);
    }
  }

  return make_scoped_refptr(g_process);
}

bool BrowserProcessMain::Init() {
  if (!main_runner_ || !main_delegate_) {
    DLOG(ERROR) << "Failed to create main components";
    return false;
  }

  if (main_runner_->Initialize(0, NULL, main_delegate_.get()) != -1) {
    DLOG(ERROR) << "Failed to initialize main runner";
    return false;
  }

  if (main_runner_->Run() != 0) {
    DLOG(ERROR) << "Failed to run main runner";
    return false;
  }

  return true;
}

BrowserProcessMain::BrowserProcessMain() {
  DCHECK(!g_process) << "Should only have one BrowserProcessMain";

  g_process = this;

  main_delegate_.reset(ContentMainDelegate::Create());
  main_runner_.reset(content::ContentMainRunner::Create());
}

BrowserProcessMain::~BrowserProcessMain() {
  DCHECK_EQ(g_process, this);
  CHECK_EQ(BrowserContext::GetAllContexts().size(), static_cast<size_t>(0));

  MessageLoopForUI::current()->Stop();
  main_runner_->Shutdown();

  url_util::Shutdown();
  ui::ResourceBundle::CleanupSharedInstance();

  g_process = NULL;
}

// static
IOThreadDelegate* BrowserProcessMain::io_thread_delegate() {
  return g_process->io_thread_delegate_.get();
}

// static
bool BrowserProcessMain::Exists() {
  return !!g_process;
}

// static
void BrowserProcessMain::CreateIOThreadDelegate() {
  DCHECK(!g_process->io_thread_delegate_);
  g_process->io_thread_delegate_.reset(new IOThreadDelegate());
}

// static
int BrowserProcessMain::RunBrowserProcess(
    const content::MainFunctionParams& main_function_params) {
  if (!g_process) {
    LOG(ERROR) << "Cannot run the Oxide Renderer as a browser!";
    return 1;
  }

  DCHECK(!g_process->browser_main_runner_);

  g_process->browser_main_runner_.reset(content::BrowserMainRunner::Create());
  int rv = g_process->browser_main_runner_->Initialize(main_function_params);
  if (rv != -1) {
    DLOG(ERROR) << "Failed to initialize browser main runner";
    return rv;
  }

  return g_process->browser_main_runner_->Run();
}

// static
void BrowserProcessMain::ShutdownBrowserProcess() {
  if (g_process) {
    g_process->browser_main_runner_->Shutdown();
  }
}

} // namespace oxide
