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

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/scoped_native_library.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "EGL/egl.h"
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
#include "shared/gl/oxide_gl_context_adopted.h"
#include "shared/port/content/browser/power_save_blocker_oxide.h"
#include "shared/port/content/browser/render_widget_host_view_oxide.h"
#include "shared/port/content/browser/web_contents_view_oxide.h"
#include "shared/port/content/common/gpu_thread_shim_oxide.h"
#include "shared/port/gfx/gfx_utils_oxide.h"
#include "shared/port/gl/gl_implementation_oxide.h"

#include "oxide_browser_context.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
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

  base::MessageLoop::InitMessagePumpForUIFactory(CreateUIMessagePump);
  main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
  base::MessageLoop::InitMessagePumpForUIFactory(nullptr);
}

int BrowserMainParts::PreCreateThreads() {
  // When using EGL, we need GLES for surfaceless contexts. Whilst the
  // default API is GLES and this will be the selected API on the GPU
  // thread, it is possible that the embedder has selected a different API
  // on the main thread. Temporarily switch to GLES whilst we initialize
  // the GL bits here
  ScopedBindGLESAPI gles_binder;

  GLContextAdopted* gl_share_context =
      BrowserPlatformIntegration::GetInstance()->GetGLShareContext();
  if (gl_share_context) {
    gfx::InitializePreferredGLImplementation(
        gl_share_context->GetImplementation());
  }

  // Do this here rather than on the GPU thread to work around a mesa race -
  // see https://launchpad.net/bugs/1267893.
  // Also, it allows us to check if the GL share context platform matches
  // the selected Chromium GL platform before spinning up the GPU thread
  gfx::GLSurface::InitializeOneOff();

  if (gl_share_context &&
      gl_share_context->GetImplementation() == gfx::GetGLImplementation()) {
    content::oxide_gpu_shim::SetGLShareGroup(gl_share_context->share_group());
  } else {
    DLOG(INFO) << "No valid shared GL context has been provided. "
               << "Compositing will not work";
  }

  primary_screen_.reset(new Screen());
  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                 primary_screen_.get());

  io_thread_.reset(new IOThread());

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

  return 0;
}

void BrowserMainParts::PreMainMessageLoopRun() {
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
}

BrowserMainParts::BrowserMainParts() {}

BrowserMainParts::~BrowserMainParts() {}

} // namespace oxide
