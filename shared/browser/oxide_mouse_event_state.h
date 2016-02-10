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

#ifndef _OXIDE_SHARED_BROWSER_MOUSE_EVENT_STATE_H_
#define _OXIDE_SHARED_BROWSER_MOUSE_EVENT_STATE_H_

#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/gfx/geometry/point.h"

namespace oxide {

class MouseEventState {
 public:
  MouseEventState();

  void Reset();

  void UpdateEvent(blink::WebMouseEvent* event);

 private:
  bool IsConsecutiveClick(const blink::WebMouseEvent& event);

  bool mouse_entered_;
  gfx::Point global_position_;

  blink::WebMouseEvent::Button click_button_;
  int click_count_;
  gfx::Point click_position_;
  double last_click_event_time_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_MOUSE_EVENT_STATE_H_
