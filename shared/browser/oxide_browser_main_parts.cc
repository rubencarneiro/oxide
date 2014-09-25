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
#include "net/base/net_module.h"
#include "third_party/khronos/EGL/egl.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/screen_type_delegate.h"
#include "ui/gl/gl_surface.h"

#include "shared/browser/compositor/oxide_compositor_utils.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_net_resource_provider.h"

#include "oxide_content_browser_client.h"
#include "oxide_default_screen_info.h"
#include "oxide_io_thread.h"
#include "oxide_message_pump.h"

namespace oxide {

namespace {

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
  egl_lib_.Reset(base::LoadNativeLibrary(base::FilePath("libEGL.so.1"), NULL));
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
  DCHECK_NE(orig_api_, EGL_NONE);

  _eglBindAPI eglBindAPI = reinterpret_cast<_eglBindAPI>(
      egl_lib_.GetFunctionPointer("eglBindAPI"));
  DCHECK(eglBindAPI);

  eglBindAPI(orig_api_);
}

class Screen : public gfx::Screen {
 public:
  Screen() {}

  bool IsDIPEnabled() FINAL {
    NOTIMPLEMENTED();
    return true;
  }

  gfx::Point GetCursorScreenPoint() FINAL {
    NOTIMPLEMENTED();
    return gfx::Point();
  }

  gfx::NativeWindow GetWindowUnderCursor() FINAL {
    NOTIMPLEMENTED();
    return NULL;
  }

  gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point) FINAL {
    NOTIMPLEMENTED();
    return NULL;
  }

  int GetNumDisplays() const FINAL {
    NOTIMPLEMENTED();
    return 1;
  }

  std::vector<gfx::Display> GetAllDisplays() const FINAL {
    NOTIMPLEMENTED();
    return std::vector<gfx::Display>();
  }

  gfx::Display GetDisplayNearestWindow(gfx::NativeView view) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayNearestPoint(const gfx::Point& point) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetDisplayMatching(const gfx::Rect& match_rect) const FINAL {
    NOTIMPLEMENTED();
    return gfx::Display();
  }

  gfx::Display GetPrimaryDisplay() const FINAL {
    blink::WebScreenInfo info(GetDefaultWebScreenInfo());

    gfx::Display display;
    display.set_bounds(info.rect);
    display.set_work_area(info.availableRect);
    display.set_device_scale_factor(info.deviceScaleFactor);

    return display;
  }

  void AddObserver(gfx::DisplayObserver* observer) FINAL {
    NOTIMPLEMENTED();
  }
  void RemoveObserver(gfx::DisplayObserver* observer) FINAL {
    NOTIMPLEMENTED();
  }
};

} // namespace

void BrowserMainParts::PreEarlyInitialization() {
  MessagePumpFactory* factory = GetMessagePumpFactory();
  CHECK(factory);
  base::MessageLoop::InitMessagePumpForUIFactory(factory);

  main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
}

int BrowserMainParts::PreCreateThreads() {
  // When using EGL, we need GLES for surfaceless contexts. Whilst the
  // default API is GLES and this will be the selected API on the GPU
  // thread, it is possible that the embedder has selected a different API
  // on the main thread. Temporarily switch to GLES whilst we initialize
  // the GL bits here
  ScopedBindGLESAPI gles_binder;

  // Work around a mesa race - see https://launchpad.net/bugs/1267893
  gfx::GLSurface::InitializeOneOff();

  primary_screen_.reset(new Screen());
  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                 primary_screen_.get());

  io_thread_.reset(new IOThread());

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
  CompositorUtils::GetInstance()->Destroy();
}

void BrowserMainParts::PostDestroyThreads() {
  gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE, NULL);
  io_thread_.reset();
}

BrowserMainParts::BrowserMainParts() {}

BrowserMainParts::~BrowserMainParts() {}

} // namespace oxide
