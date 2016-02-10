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

#include "oxide_browser_platform_integration.h"

namespace oxide {

namespace {
const int kDoubleClickSlop = 25;
const int kMaxClicks = 3;
}

bool MouseEventState::IsConsecutiveClick(const blink::WebMouseEvent& event) {
  DCHECK_EQ(event.clickCount, 1);
  DCHECK_EQ(event.type, blink::WebInputEvent::MouseDown);

  if (click_count_ == 0) {
    return false;
  }

  if (click_count_ >= kMaxClicks) {
    return false;
  }

  if (event.button != click_button_) {
    return false;
  }

  if (int((event.timeStampSeconds - last_click_event_time_) * 1000) >
      BrowserPlatformIntegration::GetInstance()->GetClickInterval()) {
    return false;
  }

  if (std::abs(event.x - click_position_.x()) > kDoubleClickSlop ||
      std::abs(event.y - click_position_.y()) > kDoubleClickSlop) {
    return false;
  }

  return true;
}

MouseEventState::MouseEventState()
    : mouse_entered_(false),
      click_button_(blink::WebMouseEvent::ButtonNone),
      click_count_(0),
      last_click_event_time_(0) {}

void MouseEventState::Reset() {
  click_count_ = 0;
  click_button_ = blink::WebMouseEvent::ButtonNone;
  click_position_ = gfx::Point();
  last_click_event_time_ = 0.f;
}

void MouseEventState::UpdateEvent(blink::WebMouseEvent* event) {
  if (event->type != blink::WebInputEvent::MouseLeave &&
      !mouse_entered_) {
    mouse_entered_ = true;
    global_position_.SetPoint(event->globalX, event->globalY);
  } else {
    mouse_entered_ = false;
  }

  if (event->type == blink::WebInputEvent::MouseEnter ||
      event->type == blink::WebInputEvent::MouseLeave) {
    event->type = blink::WebInputEvent::MouseMove;
  }

  event->movementX = event->globalX - global_position_.x();
  event->movementY = event->globalY - global_position_.y();

  global_position_.SetPoint(event->globalX, event->globalY);

  if (event->type == blink::WebInputEvent::MouseDown) {
    if (IsConsecutiveClick(*event)) {
      event->clickCount = ++click_count_;
    } else {
      click_button_ = event->button;
      click_count_ = 1;
      click_position_.SetPoint(event->x, event->y);
    }
    last_click_event_time_ = event->timeStampSeconds;
  }
}

} // namespace oxide
