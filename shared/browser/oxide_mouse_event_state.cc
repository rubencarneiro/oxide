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

#include "oxide_browser_platform_integration.h"

#include "oxide_mouse_event_state.h"


namespace oxide {

MouseEventState::MouseEventState()
    : click_count_(0)
    , click_transfer_(0)
    , last_button_(blink::WebMouseEvent::ButtonNone)
    , event_type_(blink::WebMouseEvent::Undefined)
    , last_click_event_(0)
{
}

void MouseEventState::UpdateFromSourceEvent(const blink::WebMouseEvent& event)
{
  if (event.type == blink::WebInputEvent::MouseDown) {
    if (last_button_ == event.button &&
        int((event.timeStampSeconds - last_click_event_) * 1000) <
              BrowserPlatformIntegration::GetInstance()->GetClickInterval() &&
        click_count_ < 3) {
        click_count_++;
    } else {
      click_count_ = 1;
    }
    click_transfer_ = click_count_;
    last_button_ = event.button;
    last_click_event_ = event.timeStampSeconds;
  } else {
    click_transfer_ = 0;
  }

  if (event.type == blink::WebInputEvent::MouseEnter ||
      event.type == blink::WebInputEvent::MouseLeave) {
    delta_.SetPoint(0, 0);
    global_position_.SetPoint(event.globalX, event.globalY);
    event_type_ = blink::WebInputEvent::MouseMove;
  } else {
    delta_.SetPoint(event.globalX - global_position_.x(),
                    event.globalY - global_position_.y());
    global_position_.SetPoint(event.globalX, event.globalY);
    event_type_ = event.type;
  }
}

void MouseEventState::CoerceForwardEvent(blink::WebMouseEvent& event)
{
  event.type = event_type_;
  event.clickCount = click_transfer_;

  event.movementX = delta_.x();
  event.movementY = delta_.y();
}

} // namespace oxide
