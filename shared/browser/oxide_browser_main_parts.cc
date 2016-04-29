// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include <utility>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/scoped_native_library.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/power_save_blocker_oxide.h"
#include "content/browser/web_contents/web_contents_view_oxide.h"
#include "EGL/egl.h"
#include "gpu/config/gpu_driver_bug_workaround_type.h"
#include "gpu/config/gpu_info_collector.h"
#include "gpu/ipc/service/gpu_service_shim_oxide.h"
#include "media/audio/audio_manager.h"
#include "net/base/net_module.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/gfx_utils_oxide.h"
#include "ui/gfx/screen.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface.h"

#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/browser/media/oxide_video_capture_device_factory_linux.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_net_resource_provider.h"
#include "shared/gpu/oxide_gl_context_dependent.h"
#include "shared/port/gpu_config/gpu_info_collector_oxide_linux.h"
#include "shared/port/media/video_capture_device_factory_override.h"
#include "shared/port/ui_base/clipboard_oxide.h"

#include "oxide_browser_context.h"
#include "oxide_browser_platform_integration.h"
#include "oxide_browser_process_main.h"
#include "oxide_gpu_info_collector_linux.h"
#include "oxide_hybris_utils.h"
#include "oxide_io_thread.h"
#include "oxide_lifecycle_observer.h"
#include "oxide_message_pump.h"
#include "oxide_power_save_blocker.h"
#include "oxide_render_process_initializer.h"
#include "oxide_screen_client.h"
#include "oxide_web_contents_view.h"

namespace oxide {

namespace {

std::unique_ptr<media::VideoCaptureDeviceFactory> CreateVideoCaptureDeviceFactory(
    std::unique_ptr<media::VideoCaptureDeviceFactory> delegate) {
  return base::WrapUnique(
      new VideoCaptureDeviceFactoryLinux(std::move(delegate)));
}

ui::Clipboard* CreateClipboard() {
  return BrowserPlatformIntegration::GetInstance()->CreateClipboard();
}

std::unique_ptr<base::MessagePump> CreateUIMessagePump() {
  return BrowserPlatformIntegration::GetInstance()->CreateUIMessagePump();
}

bool CanUseSharedGLContext() {
  if (!HybrisUtils::IsUsingAndroidEGL()) {
    return true;
  }

  if (content::GpuDataManagerImpl::GetInstance()->IsDriverBugWorkaroundActive(
          gpu::USE_VIRTUALIZED_GL_CONTEXTS)) {
    // Virtualized contexts on Android generally work around share group bugs.
    // Don't use the application provided share group if virtualized contexts
    // are to be used for this driver
    return false;
  }

  return true;
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
    NOTREACHED();
    return gfx::Point();
  }

  gfx::NativeWindow GetWindowUnderCursor() final {
    NOTREACHED();
    return nullptr;
  }

  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) final {
    NOTREACHED();
    return nullptr;
  }

  int GetNumDisplays() const final {
    NOTREACHED();
    return 1;
  }

  std::vector<gfx::Display> GetAllDisplays() const final {
    NOTREACHED();
    return std::vector<gfx::Display>();
  }

  gfx::Display GetDisplayNearestWindow(gfx::NativeView view) const final {
    // XXX(chrisccoulson): This gets called when a drag starts. |view|
    //  is the NativeView for the corresponding RenderWidgetHostView. It would
    //  be nice to find a way to cleverly map this to the associated RWHV and
    //  get the correct display
    return gfx::Display();
  }

  gfx::Display GetDisplayNearestPoint(const gfx::Point& point) const final {
    NOTREACHED();
    return gfx::Display();
  }

  gfx::Display GetDisplayMatching(const gfx::Rect& match_rect) const final {
    NOTREACHED();
    return gfx::Display();
  }

  gfx::Display GetPrimaryDisplay() const final {
    return BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetPrimaryDisplay();
  }

  void AddObserver(gfx::DisplayObserver* observer) final {
    NOTREACHED();
  }
  void RemoveObserver(gfx::DisplayObserver* observer) final {
    NOTREACHED();
  }
};

} // namespace

void BrowserMainParts::PreEarlyInitialization() {
  content::SetWebContentsViewOxideFactory(WebContentsView::Create);
  content::SetPowerSaveBlockerOxideDelegateFactory(CreatePowerSaveBlocker);
  media::SetVideoCaptureDeviceFactoryOverrideFactory(
      CreateVideoCaptureDeviceFactory);
  ui::SetClipboardOxideFactory(CreateClipboard);

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

  primary_screen_.reset(new Screen());
  gfx::Screen::SetScreenInstance(primary_screen_.get());

  io_thread_.reset(new IOThread());

  return 0;
}

void BrowserMainParts::PreMainMessageLoopRun() {
  media::AudioManager::SetGlobalAppName(
      BrowserPlatformIntegration::GetInstance()->GetApplicationName());

  if (CanUseSharedGLContext()) {
    scoped_refptr<GLContextDependent> share_context =
        BrowserPlatformIntegration::GetInstance()->GetGLShareContext();
    if (share_context) {
      // There's nothing to prevent other code in Oxide from accessing the
      // shared gfx::GLContext and associated gfx::GLShareGroup after the GPU
      // thread has begun consuming it, and adjusting their reference counts.
      // As it's not safe to do that, we clone it here. Note, this doesn't mean
      // that you can assume it's safe to use the handle returned by it for
      // anything
      gl_share_context_ = GLContextDependent::CloneFrom(share_context.get());
      gpu::oxide_shim::SetGLShareGroup(gl_share_context_->share_group());
    }
  }

  // Collect all graphics info. Ideally we would do this with
  // GpuDataManager::RequestCompleteGpuInfoIfNeeded, which would also start
  // the GPU service. However, we need this to complete synchronously so that
  // GpuDataManagerImpl::CanUseGpuBrowserCompositor returns the correct
  // result when the first WebContents is created.
  // XXX: This needs to be fixed when we have an external GPU service
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

  CompositorUtils::GetInstance()->Initialize(gl_share_context_.get());
  net::NetModule::SetResourceProvider(NetResourceProvider);

  lifecycle_observer_.reset(new LifecycleObserver());
  render_process_initializer_.reset(new RenderProcessInitializer());
}

bool BrowserMainParts::MainMessageLoopRun(int* result_code) {
  MessagePump::Get()->Start();
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

  gfx::Screen::SetScreenInstance(nullptr);
  io_thread_.reset();

  gpu::oxide_shim::SetGLShareGroup(nullptr);
  gl_share_context_ = nullptr;

  gpu::SetGpuInfoCollectorOxideLinux(nullptr);
}

BrowserMainParts::BrowserMainParts() {}

BrowserMainParts::~BrowserMainParts() {}

} // namespace oxide
