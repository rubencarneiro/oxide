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
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"

#include "shared/app/oxide_content_main_delegate.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_context.h"
#include "oxide_io_thread_globals.h"
#include "oxide_message_pump.h"

namespace oxide {

namespace {
scoped_ptr<BrowserProcessMain> g_instance;
}

BrowserProcessMain::BrowserProcessMain(
    int flags,
    scoped_refptr<oxide::SharedGLContext> shared_gl_context) :
    did_shutdown_(false),
    flags_(flags),
    shared_gl_context_(shared_gl_context),
    main_delegate_(ContentMainDelegate::Create()),
    main_runner_(content::ContentMainRunner::Create()) {
  CHECK(!g_instance) << "Should only have one BrowserProcessMain";
}

int BrowserProcessMain::RunBrowserMain(
    const content::MainFunctionParams& main_function_params) {
  CHECK(!browser_main_runner_);

  browser_main_runner_.reset(content::BrowserMainRunner::Create());
  int rv = browser_main_runner_->Initialize(main_function_params);
  if (rv != -1) {
    LOG(ERROR) << "Failed to initialize the Oxide browser main runner";
    return rv;
  }

  return browser_main_runner_->Run();
}

void BrowserProcessMain::ShutdownBrowserMain() {
  CHECK(browser_main_runner_);
  browser_main_runner_->Shutdown();
}

bool BrowserProcessMain::Init() {
  static bool initialized = false;
  if (initialized) {
    LOG(ERROR) <<
        "Cannot restart the Oxide main components once they have been shut down";
    return false;
  }

  initialized = true;

  if (!shared_gl_context_) {
    DLOG(INFO) << "No shared GL context has been created. "
               << "Compositing will not work";
  }

  if (!main_runner_ || !main_delegate_) {
    LOG(ERROR) << "Failed to create the Oxide main components";
    return false;
  }

  content::ContentMainParams params(main_delegate_.get());
  if (main_runner_->Initialize(params) != -1) {
    LOG(ERROR) << "Failed to initialize Oxide main runner";
    return false;
  }

  if (main_runner_->Run() != 0) {
    LOG(ERROR) << "Failed to run the Oxide main runner";
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
  CHECK(did_shutdown_) <<
      "BrowserProcessMain is being deleted without calling Quit()";
}

// static
bool BrowserProcessMain::StartIfNotRunning(
    int flags,
    scoped_refptr<oxide::SharedGLContext> shared_gl_context) {
  if (g_instance) {
    CHECK_EQ(g_instance->flags(), flags) <<
        "BrowserProcessMain::StartIfNotRunning() called more than once with "
        "different flags";
    CHECK_EQ(g_instance->shared_gl_context(), shared_gl_context) <<
        "BrowserProcessMain::StartIfNotRunning() called more than once with "
        "a different shared GL context";
    return true;
  }

  g_instance.reset(new BrowserProcessMain(flags, shared_gl_context));
  if (!g_instance->Init()) {
    g_instance.reset();
  }

  return Exists();
}

// static
void BrowserProcessMain::ShutdownIfRunning() {
  if (g_instance) {
    g_instance->Shutdown();
    g_instance.reset();
  }
}

// static
bool BrowserProcessMain::Exists() {
  return g_instance != NULL;
}

// static
BrowserProcessMain* BrowserProcessMain::instance() {
  CHECK(g_instance) << "BrowserProcessMain instance hasn't been created yet";
  return g_instance.get();
}

void BrowserProcessMain::CreateIOThreadGlobals() {
  CHECK(!io_thread_globals_) <<
      "BrowserProcessMain::CreateIOThreadGlobals() called more than once";
  io_thread_globals_.reset(new IOThreadGlobals());
}

} // namespace oxide
