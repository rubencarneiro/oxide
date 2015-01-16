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
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/posix/global_descriptors.h"
#include "base/run_loop.h"
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
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#if defined(USE_NSS)
#include "crypto/nss_util.h"
#endif
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
#include "gin/public/isolate_holder.h"
#endif
#include "ipc/ipc_descriptors.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gl/gl_implementation.h"
#include "ui/native_theme/native_theme_switches.h"

#include "shared/app/oxide_content_main_delegate.h"
#include "shared/app/oxide_platform_delegate.h"
#include "shared/common/oxide_constants.h"
#include "shared/common/oxide_content_client.h"
#include "shared/port/gl/gl_implementation_oxide.h"

#include "oxide_browser_context.h"
#include "oxide_form_factor.h"
#include "oxide_message_pump.h"

namespace content {

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
      content_client->utility_ = delegate->CreateContentUtilityClient();
    }
  }
};

} // namespace content

namespace oxide {

class BrowserProcessMainImpl : public BrowserProcessMain {
 public:
  BrowserProcessMainImpl();
  virtual ~BrowserProcessMainImpl();

  void Start(scoped_ptr<PlatformDelegate> delegate,
#if defined(USE_NSS)
             const base::FilePath& nss_db_path,
#endif
             SupportedGLImplFlags supported_gl_flags,
             ProcessModel process_model) final;
  void Shutdown() final;

  bool IsRunning() const final {
    return state_ == STATE_STARTED || state_ == STATE_SHUTTING_DOWN;
  }

  void IncrementPendingUnloadsCount() final {
    DCHECK_EQ(state_, STATE_STARTED);
    pending_unloads_count_++;
  }
  void DecrementPendingUnloadsCount() final {
    DCHECK_GT(pending_unloads_count_, 0U);
    if (--pending_unloads_count_ == 0 && state_ == STATE_SHUTTING_DOWN) {
      shutdown_loop_quit_closure_.Run();
    }
  }

  ProcessModel GetProcessModel() const final {
    DCHECK_NE(state_, STATE_NOT_STARTED);
    return process_model_;
  }

 private:

  enum State {
    STATE_NOT_STARTED,
    STATE_STARTED,
    STATE_SHUTTING_DOWN,
    STATE_SHUTDOWN
  };
  State state_;

  ProcessModel process_model_;

  // XXX: Don't change the order of these
  scoped_ptr<PlatformDelegate> platform_delegate_;
  scoped_ptr<ContentMainDelegate> main_delegate_;
  scoped_ptr<base::AtExitManager> exit_manager_;
  scoped_ptr<content::BrowserMainRunner> browser_main_runner_;

  size_t pending_unloads_count_;
  base::Closure shutdown_loop_quit_closure_;
};

namespace {

bool IsEnvironmentOptionEnabled(base::StringPiece option) {
  std::string name("OXIDE_");
  name += option.data();

  base::StringPiece val(getenv(name.c_str()));

  return !val.empty() && val == "1";
}

base::StringPiece GetEnvironmentOption(base::StringPiece option) {
  std::string name("OXIDE_");
  name += option.data();

  return getenv(name.c_str());
}

void SetupAndVerifySignalHandlers() {
  // Ignoring SIGCHLD will break base::GetTerminationStatus. CHECK that the
  // application hasn't done this
  struct sigaction sigact;
  CHECK(sigaction(SIGCHLD, nullptr, &sigact) == 0);
  CHECK(sigact.sa_handler != SIG_IGN) << "SIGCHLD should not be ignored";
  CHECK((sigact.sa_flags & SA_NOCLDWAIT) == 0) <<
      "SA_NOCLDWAIT should not be set";

  // We don't want SIGPIPE to terminate the process so set the SIGPIPE action
  // to SIG_IGN if it is currently SIG_DFL, else leave it as the application
  // set it - if the application has set a handler that terminates the process,
  // then tough luck
  CHECK(sigaction(SIGPIPE, nullptr, &sigact) == 0);
  if (sigact.sa_handler == SIG_DFL) {
    sigact.sa_handler = SIG_IGN;
    CHECK(sigaction(SIGPIPE, &sigact, nullptr) == 0);
  }
}

base::FilePath GetSubprocessPath() {
  base::StringPiece subprocess_path = GetEnvironmentOption("SUBPROCESS_PATH");
  if (!subprocess_path.empty()) {
    // Make sure that we have a properly formed absolute path
    // there are some load issues if not.
    return base::MakeAbsoluteFilePath(base::FilePath(subprocess_path.data()));
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

void InitializeCommandLine(const base::FilePath& subprocess_path,
                           ProcessModel process_model) {
  CHECK(base::CommandLine::Init(0, nullptr)) <<
      "CommandLine already exists. Did you call BrowserProcessMain::Start "
      "in a child process?";

  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  // Pick the correct subprocess path
  command_line->AppendSwitchASCII(switches::kBrowserSubprocessPath,
                                  subprocess_path.value().c_str());

  // This is needed so that we can share GL resources with the embedder
  command_line->AppendSwitch(switches::kInProcessGPU);

  command_line->AppendSwitch(switches::kUIPrioritizeInGpuProcess);
  command_line->AppendSwitch(switches::kEnableSmoothScrolling);

  // Remove this when we implement a selection API (see bug #1324292)
  command_line->AppendSwitch(switches::kDisableTouchEditing);

  command_line->AppendSwitch(
      cc::switches::kEnableTopControlsPositionCalculation);

  base::StringPiece renderer_cmd_prefix =
      GetEnvironmentOption("RENDERER_CMD_PREFIX");
  if (!renderer_cmd_prefix.empty()) {
    command_line->AppendSwitchASCII(switches::kRendererCmdPrefix,
                                    renderer_cmd_prefix.data());
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

  if (IsEnvironmentOptionEnabled("IGNORE_GPU_BLACKLIST")) {
    command_line->AppendSwitch(switches::kIgnoreGpuBlacklist);
  }

  if (process_model == PROCESS_MODEL_SINGLE_PROCESS) {
    command_line->AppendSwitch(switches::kSingleProcess);
  } else if (process_model == PROCESS_MODEL_PROCESS_PER_VIEW) {
    command_line->AppendSwitch(switches::kProcessPerTab);
  } else if (process_model == PROCESS_MODEL_PROCESS_PER_SITE) {
    command_line->AppendSwitch(switches::kProcessPerSite);
  } else if (process_model == PROCESS_MODEL_SITE_PER_PROCESS) {
    command_line->AppendSwitch(switches::kSitePerProcess);
  } else {
    DCHECK(process_model == PROCESS_MODEL_PROCESS_PER_SITE_INSTANCE ||
           process_model == PROCESS_MODEL_MULTI_PROCESS);
  }

  if (IsEnvironmentOptionEnabled("ALLOW_SANDBOX_DEBUGGING")) {
    command_line->AppendSwitch(switches::kAllowSandboxDebugging);
  }
  if (IsEnvironmentOptionEnabled("EXPERIMENTAL_ENABLE_GTALK_PLUGIN")) {
    command_line->AppendSwitch(switches::kEnableGoogleTalkPlugin);
  }

  if (IsEnvironmentOptionEnabled("ENABLE_MEDIA_HUB_AUDIO")) {
    command_line->AppendSwitch(switches::kEnableMediaHubAudio);
  }
  base::StringPiece mediahub_fixed_session_domains = GetEnvironmentOption("MEDIA_HUB_FIXED_SESSION_DOMAINS");
  if (!mediahub_fixed_session_domains.empty()) {
    command_line->AppendSwitchASCII(switches::kMediaHubFixedSessionDomains,
                                    mediahub_fixed_session_domains.data());

    if (!IsEnvironmentOptionEnabled("ENABLE_MEDIA_HUB_AUDIO")) {
      command_line->AppendSwitch(switches::kEnableMediaHubAudio);
    }
  }
}

void AddFormFactorSpecificCommandLineArguments() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();

  FormFactor form_factor = GetFormFactorHint();

  if (form_factor == FORM_FACTOR_PHONE || form_factor == FORM_FACTOR_TABLET) {
    command_line->AppendSwitch(switches::kEnableViewport);
    command_line->AppendSwitch(switches::kEnableViewportMeta);
    command_line->AppendSwitch(switches::kMainFrameResizesAreOrientationChanges);
    command_line->AppendSwitch(switches::kEnablePinch);
    command_line->AppendSwitch(cc::switches::kEnablePinchVirtualViewport);
    command_line->AppendSwitch(switches::kEnableOverlayScrollbar);
    command_line->AppendSwitch(switches::kLimitMaxDecodedImageBytes);
  }

  const char* form_factor_string = nullptr;
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
}

bool IsUnsupportedProcessModel(ProcessModel process_model) {
  switch (process_model) {
    case PROCESS_MODEL_SINGLE_PROCESS:
    case PROCESS_MODEL_PROCESS_PER_VIEW:
    case PROCESS_MODEL_PROCESS_PER_SITE:
    case PROCESS_MODEL_SITE_PER_PROCESS:
      return true;
    default:
      return false;
  }
}

}

BrowserProcessMainImpl::BrowserProcessMainImpl()
    : state_(STATE_NOT_STARTED),
      process_model_(PROCESS_MODEL_MULTI_PROCESS),
      pending_unloads_count_(0) {}

BrowserProcessMainImpl::~BrowserProcessMainImpl() {
  CHECK(state_ == STATE_NOT_STARTED || state_ == STATE_SHUTDOWN) <<
      "BrowserProcessMain::Shutdown() should be called before process exit";
}

void BrowserProcessMainImpl::Start(scoped_ptr<PlatformDelegate> delegate,
#if defined(USE_NSS)
                                   const base::FilePath& nss_db_path,
#endif
                                   SupportedGLImplFlags supported_gl_impls,
                                   ProcessModel process_model) {
  CHECK_EQ(state_, STATE_NOT_STARTED) <<
      "Browser components cannot be started more than once";
  CHECK(delegate) << "No PlatformDelegate provided";

  platform_delegate_ = delegate.Pass();
  main_delegate_.reset(new ContentMainDelegate(platform_delegate_.get()));

  if (IsUnsupportedProcessModel(process_model)) {
    LOG(WARNING) <<
        "Using an unsupported process model. This may affect stability and "
        "security. Use at your own risk!";
  }
  process_model_ = process_model;

  state_ = STATE_STARTED;

  SetupAndVerifySignalHandlers();

  base::GlobalDescriptors::GetInstance()->Set(
      kPrimaryIPCChannel,
      kPrimaryIPCChannel + base::GlobalDescriptors::kBaseDescriptor);

  exit_manager_.reset(new base::AtExitManager());

  base::FilePath subprocess_exe = GetSubprocessPath();
  InitializeCommandLine(subprocess_exe, process_model_);

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

  AddFormFactorSpecificCommandLineArguments();

#if defined(USE_NSS)
  if (!nss_db_path.empty()) {
    // Used for testing
    PathService::OverrideAndCreateIfNeeded(crypto::DIR_NSSDB,
                                           nss_db_path,
                                           false, true);
  }
  crypto::EarlySetupForNSSInit();
#endif

  ui::RegisterPathProvider();
  content::RegisterPathProvider();
  content::RegisterContentSchemes(true);

  CHECK(base::i18n::InitializeICU()) << "Failed to initialize ICU";
#if defined(V8_USE_EXTERNAL_STARTUP_DATA)
  CHECK(gin::IsolateHolder::LoadV8Snapshot());
#endif  // V8_USE_EXTERNAL_STARTUP_DATA

  main_delegate_->PreSandboxStartup();
  main_delegate_->SandboxInitialized(base::EmptyString());

  content::UtilityProcessHostImpl::RegisterUtilityMainThreadFactory(
      content::CreateInProcessUtilityThread);
  content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(
      content::CreateInProcessRendererThread);
  content::GpuProcessHost::RegisterGpuMainThreadFactory(
      content::CreateInProcessGpuThread);

  std::vector<gfx::GLImplementation> allowed_gl_impls;
  if (supported_gl_impls & SUPPORTED_GL_IMPL_DESKTOP_GL) {
    allowed_gl_impls.push_back(gfx::kGLImplementationDesktopGL);
  }
  if (supported_gl_impls & SUPPORTED_GL_IMPL_EGL_GLES2) {
    allowed_gl_impls.push_back(gfx::kGLImplementationEGLGLES2);
  }
  allowed_gl_impls.push_back(gfx::kGLImplementationOSMesaGL);
  gfx::InitializeAllowedGLImplementations(allowed_gl_impls);

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

  if (pending_unloads_count_ > 0) {
    // Wait for any pending unload handlers to finish
    base::MessageLoop::ScopedNestableTaskAllower nestable_task_allower(
        base::MessageLoop::current());

    base::RunLoop run_loop;
    shutdown_loop_quit_closure_ = run_loop.QuitClosure();

    run_loop.Run();
  }

  if (process_model_ != PROCESS_MODEL_SINGLE_PROCESS) {
    // In single process mode, we do this check after destroying
    // threads, as we hold the single BrowserContext alive until then
    BrowserContext::AssertNoContextsExist();
  }

  MessageLoopForUI::current()->Stop();

  browser_main_runner_->Shutdown();
  browser_main_runner_.reset();

  exit_manager_.reset();

  main_delegate_.reset();
  platform_delegate_.reset();

  state_ = STATE_SHUTDOWN;
}

BrowserProcessMain::BrowserProcessMain() {}

BrowserProcessMain::~BrowserProcessMain() {}

// static
ProcessModel BrowserProcessMain::GetProcessModelOverrideFromEnv() {
  static bool g_initialized = false;
  static ProcessModel g_process_model = PROCESS_MODEL_UNDEFINED;

  if (g_initialized) {
    return g_process_model;
  }

  g_initialized = true;

  if (IsEnvironmentOptionEnabled("SINGLE_PROCESS")) {
    g_process_model = PROCESS_MODEL_SINGLE_PROCESS;
  } else {
    base::StringPiece env = GetEnvironmentOption("PROCESS_MODEL");
    if (!env.empty()) {
      if (env == "multi-process") {
        g_process_model = PROCESS_MODEL_MULTI_PROCESS;
      } else if (env == "single-process") {
        g_process_model = PROCESS_MODEL_SINGLE_PROCESS;
      } else if (env == "process-per-site-instance") {
        g_process_model = PROCESS_MODEL_PROCESS_PER_SITE_INSTANCE;
      } else if (env == "process-per-view") {
        g_process_model = PROCESS_MODEL_PROCESS_PER_VIEW;
      } else if (env == "process-per-site") {
        g_process_model = PROCESS_MODEL_PROCESS_PER_SITE;
      } else if (env == "site-per-process") {
        g_process_model = PROCESS_MODEL_SITE_PER_PROCESS;
      } else {
        LOG(WARNING) << "Invalid process mode: " << env.data();
      }
    }
  }

  return g_process_model;
}

// static
BrowserProcessMain* BrowserProcessMain::GetInstance() {
  static BrowserProcessMainImpl g_instance;
  return &g_instance;
}

} // namespace oxide
