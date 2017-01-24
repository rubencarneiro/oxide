// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_EVENT_UTILS_H_
#define _OXIDE_SHARED_BROWSER_EVENT_UTILS_H_

#include "third_party/WebKit/public/platform/WebGestureEvent.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "third_party/WebKit/public/platform/WebTouchEvent.h"

#include "shared/common/oxide_shared_export.h"

namespace ui {
class GestureEventData;
class MotionEvent;
}

namespace oxide {

blink::WebGestureEvent MakeWebGestureEvent(const ui::GestureEventData& gesture);

blink::WebTouchEvent MakeWebTouchEvent(const ui::MotionEvent& event,
                                       bool moved_beyond_slop_region);

OXIDE_SHARED_EXPORT int WindowsKeyCodeWithoutLocation(int code);
OXIDE_SHARED_EXPORT int LocationModifiersFromWindowsKeyCode(int code);

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_EVENT_UTILS_H_
