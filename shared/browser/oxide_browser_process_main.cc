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
#include "oxide_message_pump.h"

namespace oxide {

class BrowserProcessMainImpl : public BrowserProcessMain {
 public:
  BrowserProcessMainImpl();
  virtual ~BrowserProcessMainImpl();

  void StartInternal(scoped_refptr<SharedGLContext> shared_gl_context,
                     scoped_ptr<ContentMainDelegate> delegate,
                     intptr_t native_display);
  void ShutdownInternal();

  bool IsRunningInternal() {
    return state_ == STATE_STARTED || state_ == STATE_SHUTTING_DOWN;
  }

  SharedGLContext* GetSharedGLContext() const FINAL {
    return shared_gl_context_;
  }
  intptr_t GetNativeDisplay() const FINAL {
    return native_display_;
  }

 private:
  int RunBrowserMain(
      const content::MainFunctionParams& main_function_params) FINAL;
  void ShutdownBrowserMain() FINAL;

  enum State {
    STATE_NOT_STARTED,
    STATE_STARTED,
    STATE_SHUTTING_DOWN,
    STATE_SHUTDOWN
  };
  State state_;

  scoped_refptr<SharedGLContext> shared_gl_context_;
  intptr_t native_display_;

  // XXX: Don't change the order of these
  scoped_ptr<ContentMainDelegate> main_delegate_;
  scoped_ptr<content::ContentMainRunner> main_runner_;
  scoped_ptr<content::BrowserMainRunner> browser_main_runner_;
};

namespace {
BrowserProcessMainImpl* GetInstance() {
  static BrowserProcessMainImpl g_instance;
  return &g_instance;
}
}

int BrowserProcessMainImpl::RunBrowserMain(
    const content::MainFunctionParams& main_function_params) {
  CHECK(!browser_main_runner_);

  browser_main_runner_.reset(content::BrowserMainRunner::Create());
  int rv = browser_main_runner_->Initialize(main_function_params);
  if (rv != -1) {
    LOG(ERROR) << "Failed to initialize BrowserMainRunner";
    return rv;
  }

  return browser_main_runner_->Run();
}

void BrowserProcessMainImpl::ShutdownBrowserMain() {
  CHECK(browser_main_runner_);
  browser_main_runner_->Shutdown();
}

BrowserProcessMainImpl::BrowserProcessMainImpl()
    : state_(STATE_NOT_STARTED),
      native_display_(0) {}

BrowserProcessMainImpl::~BrowserProcessMainImpl() {
  CHECK(state_ == STATE_NOT_STARTED || state_ == STATE_SHUTDOWN) <<
      "BrowserProcessMain::Shutdown() should be called before process exit";
}

void BrowserProcessMainImpl::StartInternal(
    scoped_refptr<SharedGLContext> shared_gl_context,
    scoped_ptr<ContentMainDelegate> delegate,
    intptr_t native_display) {
  CHECK_EQ(state_, STATE_NOT_STARTED) <<
      "Browser components cannot be started more than once";
  CHECK(delegate) << "No ContentMainDelegate provided";
  CHECK_NE(native_display, 0) << "Invalid native display handle";

  state_ = STATE_STARTED;;

  if (!shared_gl_context) {
    DLOG(INFO) << "No shared GL context has been provided. "
               << "Compositing will not work";
  }

  shared_gl_context_ = shared_gl_context;
  native_display_ = native_display;
  main_delegate_ = delegate.Pass();

  main_runner_.reset(content::ContentMainRunner::Create());
  CHECK(main_runner_.get()) << "Failed to create ContentMainRunner";

  content::ContentMainParams params(main_delegate_.get());
  CHECK_EQ(main_runner_->Initialize(params), -1) <<
      "Failed to initialize ContentMainRunner";

  CHECK_EQ(main_runner_->Run(), 0) << "Failed to run ContentMainRunner";
}

void BrowserProcessMainImpl::ShutdownInternal() {
  CHECK_EQ(state_, STATE_STARTED);
  state_ = STATE_SHUTTING_DOWN;

  BrowserContext::AssertNoContextsExist();

  // XXX: Better off in BrowserProcessMainParts?
  MessageLoopForUI::current()->Stop();
  main_runner_->Shutdown();

  state_ = STATE_SHUTDOWN;
}

BrowserProcessMain::BrowserProcessMain() {}

BrowserProcessMain::~BrowserProcessMain() {}

// static
void BrowserProcessMain::Start(
    scoped_refptr<SharedGLContext> shared_gl_context,
    scoped_ptr<ContentMainDelegate> delegate,
    intptr_t native_display) {
  GetInstance()->StartInternal(shared_gl_context,
                               delegate.Pass(),
                               native_display);
}

// static
void BrowserProcessMain::Shutdown() {
  GetInstance()->ShutdownInternal();
}

// static
bool BrowserProcessMain::IsRunning() {
  return GetInstance()->IsRunningInternal();
}

// static
BrowserProcessMain* BrowserProcessMain::instance() {
  CHECK(IsRunning()) <<
      "Cannot access BrowserProcessMain singleton when not running";
  return GetInstance();
}

} // namespace oxide
