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

#include "oxide_browser_main_parts.h"

#include <set>
#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/scoped_native_library.h"
#include "base/strings/string_number_conversions.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "EGL/egl.h"
#include "gpu/command_buffer/service/gpu_switches.h"
#include "gpu/config/gpu_control_list.h"
#include "gpu/config/gpu_control_list_jsons.h"
#include "gpu/config/gpu_driver_bug_list.h"
#include "gpu/config/gpu_info_collector.h"
#include "net/base/net_module.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/screen_type_delegate.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface.h"

#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_net_resource_provider.h"
#include "shared/gpu/oxide_gl_context_adopted.h"
#include "shared/port/content/browser/power_save_blocker_oxide.h"
#include "shared/port/content/browser/render_widget_host_view_oxide.h"
#include "shared/port/content/browser/web_contents_view_oxide.h"
#include "shared/port/content/common/gpu_thread_shim_oxide.h"
#include "shared/port/gfx/gfx_utils_oxide.h"
#include "shared/port/gpu_config/gpu_info_collector_oxide_linux.h"

#include "oxide_android_properties.h"
#include "oxide_browser_context.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
#include "oxide_gpu_info_collector_linux.h"
#include "oxide_io_thread.h"
#include "oxide_message_pump.h"
#include "oxide_power_save_blocker.h"
#include "oxide_web_contents_view.h"

namespace oxide {

namespace {

blink::WebScreenInfo DefaultScreenInfoGetter() {
  return BrowserPlatformIntegration::GetInstance()->GetDefaultScreenInfo();
}

scoped_ptr<base::MessagePump> CreateUIMessagePump() {
  return BrowserPlatformIntegration::GetInstance()->CreateUIMessagePump();
}

std::string IntSetToString(const std::set<int>& list) {
  std::string rt;
  for (std::set<int>::const_iterator it = list.begin();
       it != list.end(); ++it) {
    if (!rt.empty()) {
      rt += ",";
    }
    rt += base::IntToString(*it);
  }
  return rt;
}

void ApplyGpuDriverBugWorkarounds(base::CommandLine* command_line,
                                  const gpu::GPUInfo& gpu_info) {
  gpu::GpuControlList::OsType os_type_override = gpu::GpuControlList::kOsAny;
  std::string os_version_override;
  if (AndroidProperties::GetInstance()->Available()) {
    os_type_override = gpu::GpuControlList::kOsAndroid;
    os_version_override = AndroidProperties::GetInstance()->GetOSVersion();
  }

  scoped_ptr<gpu::GpuDriverBugList> list(gpu::GpuDriverBugList::Create());
  list->LoadList(gpu::kGpuDriverBugListJson,
                 gpu::GpuControlList::kCurrentOsOnly,
                 os_type_override);
  std::set<int> workarounds = list->MakeDecision(
      os_type_override, os_version_override, gpu_info);
  if (workarounds.empty()) {
    return;
  }

  command_line->AppendSwitchASCII(switches::kGpuDriverBugWorkarounds,
                                  IntSetToString(workarounds));
}

class ScopedBindGLESAPI {
 public:
  ScopedBindGLESAPI();
  virtual ~ScopedBindGLESAPI();

 private:
  typedef EGLBoolean (*_eglBindAPI)(EGLenum);
  typedef EGLenum (*_eglQueryAPI)(void);

  bool has_egl_;
  base::ScopedNativeLibrary egl_lib_;
  EGLenum orig_api_;
};

ScopedBindGLESAPI::ScopedBindGLESAPI()
    : has_egl_(false),
      orig_api_(EGL_NONE) {
  egl_lib_.Reset(
      base::LoadNativeLibrary(base::FilePath("libEGL.so.1"), nullptr));
  if (!egl_lib_.is_valid()) {
    return;
  }

  _eglBindAPI eglBindAPI = reinterpret_cast<_eglBindAPI>(
      egl_lib_.GetFunctionPointer("eglBindAPI"));
  _eglQueryAPI eglQueryAPI = reinterpret_cast<_eglQueryAPI>(
      egl_lib_.GetFunctionPointer("eglQueryAPI"));
  if (!eglBindAPI || !eglQueryAPI) {
    return;
  }

  orig_api_ = eglQueryAPI();
  if (orig_api_ == EGL_NONE) {
    return;
  }

  has_egl_ = true;

  eglBindAPI(EGL_OPENGL_ES_API);
}

ScopedBindGLESAPI::~ScopedBindGLESAPI() {
  if (!has_egl_) {
    return;
  }

  DCHECK(egl_lib_.is_valid());
  DCHECK_NE(orig_api_, static_cast<EGLenum>(EGL_NONE));

  _eglBindAPI eglBindAPI = reinterpret_cast<_eglBindAPI>(
      egl_lib_.GetFunctionPointer("eglBindAPI"));
  DCHECK(eglBindAPI);

  eglBindAPI(orig_api_);
}

class Screen : public gfx::Screen {
 public:
  Screen() {}

  gfx::Point GetCursorScreenPoint() final {
    NOTIMPLEMENTED();
    return gfx::Point();
  }

  gfx::NativeWindow GetWindowUnderCursor() final {
    NOTIMPLEMENTED();
    return nullptr;
  }

  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) final {
    NOTIMPLEMENTED();
    return nullptr;
  }

  int GetNumDisplays() const final {
    NOTIMPLEMENTED();
    return 1;
  }

  std::vector<gfx::Display> GetAllDisplays() const final {
    NOTIMPLEMENTED();
    return std::vector<gfx::Display>();
  }

  gfx::Display GetDisplayNearestWindow(gfx::NativeView view) const final {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayNearestPoint(const gfx::Point& point) const final {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayMatching(const gfx::Rect& match_rect) const final {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetPrimaryDisplay() const final {
    blink::WebScreenInfo info(
        BrowserPlatformIntegration::GetInstance()->GetDefaultScreenInfo());

    gfx::Display display;
    display.set_bounds(info.rect);
    display.set_work_area(info.availableRect);
    display.set_device_scale_factor(info.deviceScaleFactor);

    return display;
  }

  void AddObserver(gfx::DisplayObserver* observer) final {
    NOTIMPLEMENTED();
  }
  void RemoveObserver(gfx::DisplayObserver* observer) final {
    NOTIMPLEMENTED();
  }
};

} // namespace

void BrowserMainParts::PreEarlyInitialization() {
  content::SetDefaultScreenInfoGetterOxide(DefaultScreenInfoGetter);
  content::SetWebContentsViewOxideFactory(WebContentsView::Create);
  content::SetPowerSaveBlockerOxideDelegateFactory(CreatePowerSaveBlocker);

  gfx::InitializeOxideNativeDisplay(
      BrowserPlatformIntegration::GetInstance()->GetNativeDisplay());

  gpu_info_collector_.reset(CreateGpuInfoCollectorLinux());
  gpu::SetGpuInfoCollectorOxideLinux(gpu_info_collector_.get());

  base::MessageLoop::InitMessagePumpForUIFactory(CreateUIMessagePump);
  main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
  base::MessageLoop::InitMessagePumpForUIFactory(nullptr);
}

int BrowserMainParts::PreCreateThreads() {
  {
    // When using EGL, we need GLES for surfaceless contexts. Whilst the
    // default API is GLES and this will be the selected API on the GPU
    // thread, it is possible that the embedder has selected a different API
    // on the main thread. Temporarily switch to GLES whilst we initialize
    // the GL bits here
    ScopedBindGLESAPI gles_binder;

    // Do this here rather than on the GPU thread to work around a mesa race -
    // see https://launchpad.net/bugs/1267893.
    gfx::GLSurface::InitializeOneOff();
  }

  GLContextAdopted* share_context =
      BrowserPlatformIntegration::GetInstance()->GetGLShareContext();
  if (share_context) {
    content::oxide_gpu_shim::SetGLShareGroup(share_context->share_group());
  }

  primary_screen_.reset(new Screen());
  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                 primary_screen_.get());

  io_thread_.reset(new IOThread());

  return 0;
}

void BrowserMainParts::PreMainMessageLoopRun() {
  gpu::GPUInfo gpu_info;
  gpu::CollectInfoResult rv = gpu::CollectContextGraphicsInfo(&gpu_info);
  switch (rv) {
    case gpu::kCollectInfoFatalFailure:
      LOG(ERROR) << "gpu::CollectContextGraphicsInfo failed";
      break;
    case gpu::kCollectInfoNone:
      NOTREACHED();
      break;
    default:
      break;
  }

  content::GpuDataManagerImpl::GetInstance()->UpdateGpuInfo(gpu_info);

  base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  if (!command_line.HasSwitch(switches::kDisableGpuDriverBugWorkarounds)) {
    ApplyGpuDriverBugWorkarounds(&command_line, gpu_info);
  }

  CompositorUtils::GetInstance()->Initialize();
  net::NetModule::SetResourceProvider(NetResourceProvider);
}

bool BrowserMainParts::MainMessageLoopRun(int* result_code) {
  MessageLoopForUI::current()->Start();
  return true;
}

void BrowserMainParts::PostMainMessageLoopRun() {
  CompositorUtils::GetInstance()->Shutdown();
}

void BrowserMainParts::PostDestroyThreads() {
  if (BrowserProcessMain::GetInstance()->GetProcessModel() ==
      PROCESS_MODEL_SINGLE_PROCESS) {
    BrowserContext::AssertNoContextsExist();
  }

  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, nullptr);
  io_thread_.reset();
  content::oxide_gpu_shim::SetGLShareGroup(nullptr);
  gpu::SetGpuInfoCollectorOxideLinux(nullptr);
}

BrowserMainParts::BrowserMainParts() {}

BrowserMainParts::~BrowserMainParts() {}

} // namespace oxide
