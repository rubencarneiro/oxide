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

#include "shared/app/oxide_content_main_delegate.h"

#include "oxide_browser_context.h"
#include "oxide_io_thread_delegate.h"
#include "oxide_message_pump.h"

namespace oxide {

namespace {
scoped_ptr<BrowserProcessMain> g_process;
}

BrowserProcessMain::BrowserProcessMain(int flags) :
    did_shutdown_(false),
    flags_(flags),
    main_delegate_(ContentMainDelegate::Create()),
    main_runner_(content::ContentMainRunner::Create()) {
  CHECK(!g_process) << "Should only have one BrowserProcessMain";
}

// static
int BrowserProcessMain::RunBrowserMain(
    const content::MainFunctionParams& main_function_params) {
  if (!g_process) {
    return 1;
  }

  DCHECK(!g_process->browser_main_runner_);

  g_process->browser_main_runner_.reset(content::BrowserMainRunner::Create());
  int rv = g_process->browser_main_runner_->Initialize(main_function_params);
  if (rv != -1) {
    LOG(ERROR) << "Failed to initialize browser main runner";
    return rv;
  }

  return g_process->browser_main_runner_->Run();
}

// static
void BrowserProcessMain::ShutdownBrowserMain() {
  if (g_process) {
    g_process->browser_main_runner_->Shutdown();
  }
}

bool BrowserProcessMain::Init() {
  static bool initialized = false;
  if (initialized) {
    DLOG(ERROR) << "Cannot restart the main components";
    return false;
  }

  initialized = true;

  if (!main_runner_ || !main_delegate_) {
    LOG(ERROR) << "Failed to create main components";
    return false;
  }

  if (main_runner_->Initialize(0, NULL, main_delegate_.get()) != -1) {
    LOG(ERROR) << "Failed to initialize main runner";
    return false;
  }

  if (main_runner_->Run() != 0) {
    LOG(ERROR) << "Failed to run main runner";
    return false;
  }

  return true;
}

void BrowserProcessMain::Shutdown() {
  if (did_shutdown_) {
    return;
  }

  did_shutdown_ = true;

  BrowserContext::AssertNoContextsExist();

  // XXX: Better off in BrowserProcessMainParts?
  MessageLoopForUI::current()->Stop();
  main_runner_->Shutdown();
}

BrowserProcessMain::~BrowserProcessMain() {
  Shutdown();
}

/* static */
bool BrowserProcessMain::Run(int flags) {
  if (!g_process) {
    g_process.reset(new BrowserProcessMain(flags));
    if (!g_process->Init()) {
      g_process.reset();
      return false;
    }
  }

  DCHECK(GetFlags() == flags) <<
    "Can't call BrowserProcessMain::Run() multiple times with different flags";

  return true;
}

/* static */
void BrowserProcessMain::Quit() {
  if (g_process) {
    g_process->Shutdown();
    g_process.reset();
  }
}

int BrowserProcessMain::GetFlags() {
  DCHECK(g_process);
  return g_process->flags_;
}

bool BrowserProcessMain::IsRunning() {
  return g_process != NULL;
}

// static
IOThreadDelegate* BrowserProcessMain::io_thread_delegate() {
  DCHECK(g_process);
  return g_process->io_thread_delegate_.get();
}

// static
void BrowserProcessMain::CreateIOThreadDelegate() {
  CHECK(!g_process->io_thread_delegate_);
  g_process->io_thread_delegate_.reset(new IOThreadDelegate());
}

} // namespace oxide
