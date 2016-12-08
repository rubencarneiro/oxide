// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "chrome_controller_impl.h"

#include "base/logging.h"
#include "third_party/WebKit/public/platform/WebBrowserControlsState.h"

#include "qt/core/glue/chrome_controller_client.h"
#include "shared/browser/chrome_controller.h"

#include "contents_view_impl.h"
#include "oxide_qt_dpi_utils.h"
#include "web_contents_id_tracker.h"

namespace oxide {
namespace qt {

namespace {

blink::WebBrowserControlsState ModeToTopControlsState(ChromeController::Mode mode) {
  switch (mode) {
    case ChromeController::Mode::Auto:
      return blink::WebBrowserControlsBoth;
    case ChromeController::Mode::Shown:
      return blink::WebBrowserControlsShown;
    case ChromeController::Mode::Hidden:
      return blink::WebBrowserControlsHidden;
  }

  NOTREACHED();
  return blink::WebBrowserControlsBoth;
}

}

struct ChromeControllerImpl::InitProps {
  Mode mode = Mode::Auto;
  bool animation_enabled = true;
};

void ChromeControllerImpl::ChromePositionUpdated() {
  client_->ChromePositionUpdated();
}

void ChromeControllerImpl::init(WebContentsID web_contents_id) {
  content::WebContents* contents =
      WebContentsIDTracker::GetInstance()->GetWebContentsFromID(web_contents_id);
  DCHECK(contents);

  controller_ = oxide::ChromeController::FromWebContents(contents);
  DCHECK(controller_);

  controller_->set_client(this);

  contents_view_ = ContentsViewImpl::FromWebContents(contents);
  DCHECK(contents_view_);

  std::unique_ptr<InitProps> init_props = std::move(init_props_);

  setTopControlsHeight(top_controls_height_);
  setMode(init_props->mode);
  setAnimationEnabled(init_props->animation_enabled);

  ChromePositionUpdated();
}

int ChromeControllerImpl::topControlsHeight() const {
  return top_controls_height_;
}

void ChromeControllerImpl::setTopControlsHeight(int height) {
  top_controls_height_ = height;

  if (!controller_) {
    return;
  }

  controller_->SetTopControlsHeight(
      DpiUtils::ConvertQtPixelsToChromium(height,
                                          contents_view_->GetScreen()));
}

ChromeController::Mode ChromeControllerImpl::mode() const {
  if (!controller_) {
    return init_props_->mode;
  }

  switch (controller_->constraints()) {
    case blink::WebBrowserControlsShown:
      return Mode::Shown;
    case blink::WebBrowserControlsHidden:
      return Mode::Hidden;
    case blink::WebBrowserControlsBoth:
      return Mode::Auto;
  }

  NOTREACHED();
  return Mode::Auto;
}

void ChromeControllerImpl::setMode(Mode mode) {
  if (!controller_) {
    init_props_->mode = mode;
  } else {
    controller_->SetConstraints(ModeToTopControlsState(mode));
  }
}

bool ChromeControllerImpl::animationEnabled() const {
  if (!controller_) {
    return init_props_->animation_enabled;
  }

  return controller_->animation_enabled();
}

void ChromeControllerImpl::setAnimationEnabled(bool enabled) {
  if (!controller_) {
    init_props_->animation_enabled = enabled;
  } else {
    controller_->set_animation_enabled(enabled);
  }
}

void ChromeControllerImpl::show(bool animated) {
  if (!controller_) {
    return;
  }

  controller_->Show(animated);
}

void ChromeControllerImpl::hide(bool animated) {
  if (!controller_) {
    return;
  }

  controller_->Hide(animated);
}

int ChromeControllerImpl::topControlsOffset() const {
  if (!controller_) {
    return 0;
  }

  return DpiUtils::ConvertChromiumPixelsToQt(
      controller_->GetTopControlsOffset(),
      contents_view_->GetScreen());
}

int ChromeControllerImpl::topContentOffset() const {
  return DpiUtils::ConvertChromiumPixelsToQt(controller_->GetTopContentOffset(),
                                             contents_view_->GetScreen());
}

void ChromeControllerImpl::OnDisplayPropertiesChanged(
    const display::Display& display) {
  if (!contents_view_ || display.id() != contents_view_->GetDisplay().id()) {
    return;
  }

  setTopControlsHeight(top_controls_height_);
}

ChromeControllerImpl::ChromeControllerImpl(qt::ChromeControllerClient* client,
                                           QObject* handle)
    : client_(client),
      controller_(nullptr),
      contents_view_(nullptr),
      init_props_(new InitProps()),
      top_controls_height_(0) {
  setHandle(handle);
}

ChromeControllerImpl::~ChromeControllerImpl() {
  if (controller_) {
    controller_->set_client(nullptr);
  }
}

} // namespace qt
} // namespace oxide
