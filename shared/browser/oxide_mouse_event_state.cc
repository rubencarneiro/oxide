// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxide_mouse_event_state.h"

#include <cstdlib>

#include "base/logging.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "third_party/WebKit/public/platform/WebMouseEvent.h"

#include "oxide_browser_platform_integration.h"

namespace oxide {

namespace {
const int kDoubleClickSlop = 25.f;
const int kMaxClicks = 3;
}

bool MouseEventState::IsConsecutiveClick(const blink::WebMouseEvent& event) {
  DCHECK_EQ(event.click_count, 1);
  DCHECK_EQ(event.GetType(), blink::WebInputEvent::kMouseDown);

  if (click_count_ == 0) {
    return false;
  }

  if (click_count_ >= kMaxClicks) {
    return false;
  }

  if (event.button != click_button_) {
    return false;
  }

  if (int((event.TimeStampSeconds() - last_click_event_time_) * 1000) >
      BrowserPlatformIntegration::GetInstance()->GetClickInterval()) {
    return false;
  }

  if (std::abs(event.PositionInWidget().x - click_position_.x()) > kDoubleClickSlop ||
      std::abs(event.PositionInWidget().y - click_position_.y()) > kDoubleClickSlop) {
    return false;
  }

  return true;
}

MouseEventState::MouseEventState()
    : mouse_entered_(false),
      click_button_(blink::WebPointerProperties::Button::kNoButton),
      click_count_(0),
      last_click_event_time_(0) {}

void MouseEventState::Reset() {
  click_count_ = 0;
  click_button_ = blink::WebPointerProperties::Button::kNoButton;
  click_position_ = gfx::PointF();
  last_click_event_time_ = 0.f;
}

void MouseEventState::UpdateEvent(blink::WebMouseEvent* event) {
  if (event->GetType() != blink::WebInputEvent::kMouseLeave &&
      !mouse_entered_) {
    mouse_entered_ = true;
    global_position_ = event->PositionInScreen();
  } else {
    mouse_entered_ = false;
  }

  if (event->GetType() == blink::WebInputEvent::kMouseEnter ||
      event->GetType() == blink::WebInputEvent::kMouseLeave) {
    event->SetType(blink::WebInputEvent::kMouseMove);
  }

  event->movement_x = event->PositionInScreen().x - global_position_.x();
  event->movement_y = event->PositionInScreen().y - global_position_.y();

  global_position_ = event->PositionInScreen();

  if (event->GetType() == blink::WebInputEvent::kMouseDown) {
    if (IsConsecutiveClick(*event)) {
      event->click_count = ++click_count_;
    } else {
      click_button_ = event->button;
      click_count_ = 1;
      click_position_ = event->PositionInWidget();
    }
    last_click_event_time_ = event->TimeStampSeconds();
  }
}

} // namespace oxide
