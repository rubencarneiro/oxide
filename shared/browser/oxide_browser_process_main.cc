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

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/posix/global_descriptors.h"
#include "base/strings/string_util.h"
#include "content/app/mojo/mojo_init.h"
#include "content/browser/gpu/gpu_process_host.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/utility_process_host_impl.h"
#include "content/common/url_schemes.h"
#include "content/gpu/in_process_gpu_thread.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/utility/content_utility_client.h"
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#if defined(USE_NSS)
#include "crypto/nss_util.h"
#endif
#include "ipc/ipc_descriptors.h"
#include "ui/base/ui_base_paths.h"

#include "shared/app/oxide_content_main_delegate.h"
#include "shared/common/oxide_content_client.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_context.h"
#include "oxide_message_pump.h"

namespace content {

namespace {
base::LazyInstance<content::ContentUtilityClient>
    g_content_utility_client = LAZY_INSTANCE_INITIALIZER;
}

// This is a bit of a hack. We're content::ContentClientInitializer so that
// we can be a friend of ContentClient. This works because the
// ContentClientInitializer impl in ContentMainRunner only has local visibility
class ContentClientInitializer {
 public:
  static void Set(ContentMainDelegate* delegate, bool single_process) {
    ContentClient* content_client = oxide::ContentClient::GetInstance();
    content_client->browser_ = delegate->CreateContentBrowserClient();

    if (single_process) {
      content_client->renderer_ = delegate->CreateContentRendererClient();
      content_client->utility_ = &g_content_utility_client.Get();
    }
  }
};

} // namespace content

namespace oxide {

class BrowserProcessMainImpl : public BrowserProcessMain {
 public:
  BrowserProcessMainImpl();
  virtual ~BrowserProcessMainImpl();

  void StartInternal(scoped_refptr<SharedGLContext> shared_gl_context,
                     scoped_ptr<ContentMainDelegate> delegate,
                     intptr_t native_display,
                     bool display_handle_valid);
  void ShutdownInternal();

  bool IsRunningInternal() {
    return state_ == STATE_STARTED || state_ == STATE_SHUTTING_DOWN;
  }

  SharedGLContext* GetSharedGLContext() const FINAL {
    return shared_gl_context_;
  }
  intptr_t GetNativeDisplay() const FINAL {
    CHECK(native_display_is_valid_);
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
  bool native_display_is_valid_;

  // XXX: Don't change the order of these
  scoped_ptr<ContentMainDelegate> main_delegate_;
  scoped_ptr<base::AtExitManager> exit_manager_;
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
      native_display_(0),
      native_display_is_valid_(false) {}

BrowserProcessMainImpl::~BrowserProcessMainImpl() {
  CHECK(state_ == STATE_NOT_STARTED || state_ == STATE_SHUTDOWN) <<
      "BrowserProcessMain::Shutdown() should be called before process exit";
}

void BrowserProcessMainImpl::StartInternal(
    scoped_refptr<SharedGLContext> shared_gl_context,
    scoped_ptr<ContentMainDelegate> delegate,
    intptr_t native_display,
    bool display_handle_valid) {
  CHECK_EQ(state_, STATE_NOT_STARTED) <<
      "Browser components cannot be started more than once";
  CHECK(delegate) << "No ContentMainDelegate provided";

  state_ = STATE_STARTED;;

  if (!shared_gl_context) {
    DLOG(INFO) << "No shared GL context has been provided. "
               << "Compositing will not work";
  }

  shared_gl_context_ = shared_gl_context;
  native_display_ = native_display;
  native_display_is_valid_ = display_handle_valid;
  main_delegate_ = delegate.Pass();

  base::GlobalDescriptors::GetInstance()->Set(
      kPrimaryIPCChannel,
      kPrimaryIPCChannel + base::GlobalDescriptors::kBaseDescriptor);

  exit_manager_.reset(new base::AtExitManager());
  base::CommandLine::Init(0, NULL);

  int exit_code;
  CHECK(!main_delegate_->BasicStartupComplete(&exit_code));

  content::InitializeMojo();

  content::ContentClientInitializer::Set(
      main_delegate_.get(),
      base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kSingleProcess));

#if defined(USE_NSS)
  crypto::EarlySetupForNSSInit();
#endif

  ui::RegisterPathProvider();
  content::RegisterPathProvider();
  content::RegisterContentSchemes(true);

  CHECK(base::i18n::InitializeICU());

  main_delegate_->PreSandboxStartup();
  main_delegate_->SandboxInitialized(base::EmptyString());

  content::UtilityProcessHostImpl::RegisterUtilityMainThreadFactory(
      content::CreateInProcessUtilityThread);
  content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(
      content::CreateInProcessRendererThread);
  content::GpuProcessHost::RegisterGpuMainThreadFactory(
      content::CreateInProcessGpuThread);

  content::MainFunctionParams main_params(
      *base::CommandLine::ForCurrentProcess());
  CHECK_EQ(main_delegate_->RunProcess(base::EmptyString(), main_params), 0);
}

void BrowserProcessMainImpl::ShutdownInternal() {
  CHECK_EQ(state_, STATE_STARTED);
  state_ = STATE_SHUTTING_DOWN;

  BrowserContext::AssertNoContextsExist();

  // XXX: Better off in BrowserProcessMainParts?
  MessageLoopForUI::current()->Stop();
  main_delegate_->ProcessExiting(base::EmptyString());

  exit_manager_.reset();

  state_ = STATE_SHUTDOWN;
}

BrowserProcessMain::BrowserProcessMain() {}

BrowserProcessMain::~BrowserProcessMain() {}

// static
void BrowserProcessMain::Start(
    scoped_refptr<SharedGLContext> shared_gl_context,
    scoped_ptr<ContentMainDelegate> delegate,
    intptr_t native_display,
    bool display_handle_valid) {
  GetInstance()->StartInternal(shared_gl_context,
                               delegate.Pass(),
                               native_display,
                               display_handle_valid);
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
