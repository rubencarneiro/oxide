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

#include "oxide/common/oxide_content_main_delegate.h"

#include "oxide_io_thread_delegate.h"

namespace {
oxide::ContentMainDelegateFactory* g_main_delegate_factory;
oxide::BrowserProcessMain* g_process;
}

namespace oxide {

// static
BrowserProcessMain* BrowserProcessMain::GetInstance() {
  if (!g_process) {
    Create();
  }

  return g_process;
}

// static
void BrowserProcessMain::Create() {
  BrowserProcessMain* tmp = new BrowserProcessMain();
  if (!tmp->Init()) {
    delete tmp;
  }
}

bool BrowserProcessMain::Init() {
  // XXX: Normally this comes from main() and takes some arguments.
  //      Should we pass any default args here?
  if (main_runner_->Initialize(0, NULL, main_delegate_.get()) != -1) {
    return false;
  }

  if (main_runner_->Run() != 0) {
    return false;
  }

  return true;
}

// static
int BrowserProcessMain::RunBrowserProcess(
    const content::MainFunctionParams& main_function_params) {
  if (!g_process) {
    LOG(ERROR) << "Running in browser mode is not supported";
    return 1;
  }

  g_process->browser_main_runner_.reset(content::BrowserMainRunner::Create());
  int rv = g_process->browser_main_runner_->Initialize(main_function_params);
  if (rv != -1) {
    return rv;
  }

  return g_process->browser_main_runner_->Run();
}

// static
void BrowserProcessMain::ShutdownBrowserProcess() {
  g_process->browser_main_runner_->Shutdown();
}

// static
void BrowserProcessMain::PreCreateThreads() {
  g_process->io_thread_delegate_.reset(new IOThreadDelegate());
}

// static
void BrowserProcessMain::InitContentMainDelegateFactory(
    ContentMainDelegateFactory* factory) {
  DCHECK(!g_process || g_main_delegate_factory == factory);
  g_main_delegate_factory = factory;
}

BrowserProcessMain::BrowserProcessMain() {
  DCHECK(g_main_delegate_factory) <<
      "Implementation needs to specify a ContentMainDelegate factory";
  main_delegate_.reset(g_main_delegate_factory());
  main_runner_.reset(content::ContentMainRunner::Create());
}

BrowserProcessMain::~BrowserProcessMain() {
  main_runner_->Shutdown();
}

// static
IOThreadDelegate* BrowserProcessMain::io_thread_delegate() {
  return g_process->io_thread_delegate_.get();
}

// static
bool BrowserProcessMain::Exists() {
  return !!g_process;
}

} // namespace oxide
