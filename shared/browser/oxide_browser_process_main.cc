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

#include <dlfcn.h>
#include <signal.h>
#include <string>
#include <vector>

#include "base/at_exit.h"
#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/i18n/icu_util.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/posix/global_descriptors.h"
#include "base/strings/string_util.h"
#include "cc/base/switches.h"
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
#include "ui/base/ui_base_switches.h"
#include "ui/native_theme/native_theme_switches.h"

#include "shared/app/oxide_content_main_delegate.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_browser_context.h"
#include "oxide_form_factor.h"
#include "oxide_message_pump.h"
#include "oxide_platform_integration.h"

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

  void Start(scoped_ptr<ContentMainDelegate> delegate,
             PlatformIntegration* platform) final;
  void Shutdown() final;

  bool IsRunning() const final {
    return state_ == STATE_STARTED || state_ == STATE_SHUTTING_DOWN;
  }

  SharedGLContext* GetSharedGLContext() const final {
    return shared_gl_context_.get();
  }
  intptr_t GetNativeDisplay() const final {
    CHECK(native_display_is_valid_);
    return native_display_;
  }

 private:

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

BrowserProcessMainImpl* GetBrowserProcessMainInstance() {
  static BrowserProcessMainImpl g_instance;
  return &g_instance;
}

bool IsEnvironmentOptionEnabled(const char* option) {
  std::string name("OXIDE_");
  name += option;

  const char* val = getenv(name.c_str());
  if (!val) {
    return false;
  }

  std::string v(val);

  return !v.empty() && v == "1";
}

const char* GetEnvironmentOption(const char* option) {
  std::string name("OXIDE_");
  name += option;

  return getenv(name.c_str());
}

void SetupAndVerifySignalHandlers() {
  // Ignoring SIGCHLD will break base::GetTerminationStatus. CHECK that the
  // application hasn't done this
  struct sigaction sigact;
  CHECK(sigaction(SIGCHLD, NULL, &sigact) == 0);
  CHECK(sigact.sa_handler != SIG_IGN) << "SIGCHLD should not be ignored";
  CHECK((sigact.sa_flags & SA_NOCLDWAIT) == 0) <<
      "SA_NOCLDWAIT should not be set";

  // We don't want SIGPIPE to terminate the process so set the SIGPIPE action
  // to SIG_IGN if it is currently SIG_DFL, else leave it as the application
  // set it - if the application has set a handler that terminates the process,
  // then tough luck
  CHECK(sigaction(SIGPIPE, NULL, &sigact) == 0);
  if (sigact.sa_handler == SIG_DFL) {
    sigact.sa_handler = SIG_IGN;
    CHECK(sigaction(SIGPIPE, &sigact, NULL) == 0);
  }
}

base::FilePath GetSubprocessPath() {
  const char* subprocess_path = GetEnvironmentOption("SUBPROCESS_PATH");
  if (subprocess_path) {
    // Make sure that we have a properly formed absolute path
    // there are some load issues if not.
    return base::MakeAbsoluteFilePath(base::FilePath(subprocess_path));
  }

  base::FilePath subprocess_exe =
      base::FilePath(FILE_PATH_LITERAL(OXIDE_SUBPROCESS_PATH));
  if (subprocess_exe.IsAbsolute()) {
    return subprocess_exe;
  }

  Dl_info info;
  int rv = dladdr(reinterpret_cast<void *>(BrowserProcessMain::GetInstance),
                  &info);
  DCHECK_NE(rv, 0) << "Failed to determine module path";

  base::FilePath subprocess_rel(subprocess_exe);
  subprocess_exe = base::FilePath(info.dli_fname).DirName();

  std::vector<base::FilePath::StringType> components;
  subprocess_rel.GetComponents(&components);
  for (size_t i = 0; i < components.size(); ++i) {
    subprocess_exe = subprocess_exe.Append(components[i]);
  }

  return subprocess_exe;
}

void InitializeCommandLine(const base::FilePath& subprocess_path) {
  CHECK(base::CommandLine::Init(0, NULL)) <<
      "CommandLine already exists. Did you call BrowserProcessMain::Start "
      "in a child process?";

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  // Pick the correct subprocess path
  command_line->AppendSwitchASCII(switches::kBrowserSubprocessPath,
                                  subprocess_path.value().c_str());

  // This is needed so that we can share GL resources with the embedder
  command_line->AppendSwitch(switches::kInProcessGPU);

  // Remove this when we have a working GPU blacklist
  command_line->AppendSwitch(switches::kDisableGpuRasterization);

  command_line->AppendSwitch(switches::kUIPrioritizeInGpuProcess);
  command_line->AppendSwitch(switches::kEnableSmoothScrolling);

  FormFactor form_factor = GetFormFactorHint();
  if (form_factor == FORM_FACTOR_PHONE || form_factor == FORM_FACTOR_TABLET) {
    command_line->AppendSwitch(switches::kEnableViewport);
    command_line->AppendSwitch(switches::kEnableViewportMeta);
    command_line->AppendSwitch(switches::kMainFrameResizesAreOrientationChanges);
    command_line->AppendSwitch(switches::kEnablePinch);
    if (IsEnvironmentOptionEnabled("ENABLE_PINCH_VIRTUAL_VIEWPORT")) {
      command_line->AppendSwitch(cc::switches::kEnablePinchVirtualViewport);
    }
    command_line->AppendSwitch(switches::kEnableOverlayScrollbar);

    // Remove this when we implement a selection API (see bug #1324292)
    command_line->AppendSwitch(switches::kDisableTouchEditing);
  }

  const char* form_factor_string = NULL;
  switch (form_factor) {
    case FORM_FACTOR_DESKTOP:
      form_factor_string = switches::kFormFactorDesktop;
      break;
    case FORM_FACTOR_TABLET:
      form_factor_string = switches::kFormFactorTablet;
      break;
    case FORM_FACTOR_PHONE:
      form_factor_string = switches::kFormFactorPhone;
      break;
    default:
      NOTREACHED();
  }
  command_line->AppendSwitchASCII(switches::kFormFactor, form_factor_string);

  const char* renderer_cmd_prefix = GetEnvironmentOption("RENDERER_CMD_PREFIX");
  if (renderer_cmd_prefix) {
    command_line->AppendSwitchASCII(switches::kRendererCmdPrefix,
                                    renderer_cmd_prefix);
  }
  if (IsEnvironmentOptionEnabled("NO_SANDBOX")) {
    command_line->AppendSwitch(switches::kNoSandbox);
  } else {
    if (IsEnvironmentOptionEnabled("DISABLE_SETUID_SANDBOX")) {
      command_line->AppendSwitch(switches::kDisableSetuidSandbox);
    }
    if (IsEnvironmentOptionEnabled("DISABLE_SECCOMP_FILTER_SANDBOX")) {
      command_line->AppendSwitch(switches::kDisableSeccompFilterSandbox);
    }
  }
  if (IsEnvironmentOptionEnabled("SINGLE_PROCESS")) {
    LOG(WARNING) <<
        "Running in single process mode. Multiple BrowserContext's will not "
        "work correctly, see https://launchpad.net/bugs/1283291";
    command_line->AppendSwitch(switches::kSingleProcess);
  }
  if (IsEnvironmentOptionEnabled("ALLOW_SANDBOX_DEBUGGING")) {
    command_line->AppendSwitch(switches::kAllowSandboxDebugging);
  }
  if (IsEnvironmentOptionEnabled("EXPERIMENTAL_ENABLE_GTALK_PLUGIN")) {
    command_line->AppendSwitch(switches::kEnableGoogleTalkPlugin);
  }
}

}

BrowserProcessMainImpl::BrowserProcessMainImpl()
    : state_(STATE_NOT_STARTED),
      native_display_(0),
      native_display_is_valid_(false) {}

BrowserProcessMainImpl::~BrowserProcessMainImpl() {
  CHECK(state_ == STATE_NOT_STARTED || state_ == STATE_SHUTDOWN) <<
      "BrowserProcessMain::Shutdown() should be called before process exit";
}

void BrowserProcessMainImpl::Start(scoped_ptr<ContentMainDelegate> delegate,
                                   PlatformIntegration* platform) {
  CHECK_EQ(state_, STATE_NOT_STARTED) <<
      "Browser components cannot be started more than once";
  CHECK(delegate) << "No ContentMainDelegate provided";

  main_delegate_ = delegate.Pass();
  PlatformIntegration::instance = platform;

  state_ = STATE_STARTED;

  shared_gl_context_ = main_delegate_->GetSharedGLContext();
  native_display_is_valid_ = main_delegate_->GetNativeDisplay(&native_display_);

  if (!shared_gl_context_.get()) {
    DLOG(INFO) << "No shared GL context has been provided. "
               << "Compositing will not work";
  }

  SetupAndVerifySignalHandlers();

  base::GlobalDescriptors::GetInstance()->Set(
      kPrimaryIPCChannel,
      kPrimaryIPCChannel + base::GlobalDescriptors::kBaseDescriptor);

  exit_manager_.reset(new base::AtExitManager());

  base::FilePath subprocess_exe = GetSubprocessPath();
  InitializeCommandLine(subprocess_exe);

  // We need to override FILE_EXE in the browser process to the path of the
  // renderer, as various bits of Chrome use this to find other resources
  PathService::Override(base::FILE_EXE, subprocess_exe);
  PathService::Override(base::FILE_MODULE, subprocess_exe);

  int exit_code;
  CHECK(!main_delegate_->BasicStartupComplete(&exit_code));

  content::InitializeMojo();

  content::ContentClientInitializer::Set(
      main_delegate_.get(),
      base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kSingleProcess));

#if defined(USE_NSS)
  if (!main_delegate_->GetNSSDbPath().empty()) {
    // Used for testing
    PathService::Override(crypto::DIR_NSSDB, main_delegate_->GetNSSDbPath());
  }
  crypto::EarlySetupForNSSInit();
#endif

  ui::RegisterPathProvider();
  content::RegisterPathProvider();
  content::RegisterContentSchemes(true);

  CHECK(base::i18n::InitializeICU()) << "Failed to initialize ICU";

  main_delegate_->PreSandboxStartup();
  main_delegate_->SandboxInitialized(base::EmptyString());

  content::UtilityProcessHostImpl::RegisterUtilityMainThreadFactory(
      content::CreateInProcessUtilityThread);
  content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(
      content::CreateInProcessRendererThread);
  content::GpuProcessHost::RegisterGpuMainThreadFactory(
      content::CreateInProcessGpuThread);

  browser_main_runner_.reset(content::BrowserMainRunner::Create());
  CHECK(browser_main_runner_.get()) << "Failed to create BrowserMainRunner";

  content::MainFunctionParams main_params(
      *base::CommandLine::ForCurrentProcess());
  CHECK_EQ(browser_main_runner_->Initialize(main_params), -1) <<
      "Failed to initialize BrowserMainRunner";
  CHECK_EQ(browser_main_runner_->Run(), 0) <<
      "Failed to run BrowserMainRunner";
}

void BrowserProcessMainImpl::Shutdown() {
  if (state_ != STATE_STARTED) {
    CHECK_NE(state_, STATE_SHUTTING_DOWN);
    return;
  }
  state_ = STATE_SHUTTING_DOWN;

  BrowserContext::AssertNoContextsExist();

  MessageLoopForUI::current()->Stop();

  browser_main_runner_->Shutdown();
  browser_main_runner_.reset();

  exit_manager_.reset();

  shared_gl_context_ = NULL;
  native_display_is_valid_ = false;
  native_display_ = 0;

  delete PlatformIntegration::instance;
  PlatformIntegration::instance = NULL;

  main_delegate_.reset();

  state_ = STATE_SHUTDOWN;
}

BrowserProcessMain::BrowserProcessMain() {}

BrowserProcessMain::~BrowserProcessMain() {}

// static
BrowserProcessMain* BrowserProcessMain::GetInstance() {
  return GetBrowserProcessMainInstance();
}

} // namespace oxide
