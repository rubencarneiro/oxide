// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_GESTURE_PROVIDER_H_
#define _OXIDE_SHARED_BROWSER_GESTURE_PROVIDER_H_

#include <memory>

#include "ui/events/gesture_detection/filtered_gesture_provider.h"

namespace blink {
class WebGestureEvent;
}

namespace ui {
class MotionEvent;
}

namespace oxide {

class GestureProviderClient {
 public:
  virtual ~GestureProviderClient();

  virtual void OnGestureEvent(const blink::WebGestureEvent& event) = 0;
};

class GestureProvider {
 public:
  static std::unique_ptr<GestureProvider> Create(GestureProviderClient* client);
  virtual ~GestureProvider();

  virtual ui::FilteredGestureProvider::TouchHandlingResult
      OnTouchEvent(const ui::MotionEvent& event) = 0;
  virtual void OnTouchEventAck(uint32_t unique_event_id,
                               bool consumed) = 0;

  virtual void SetDoubleTapSupportForPageEnabled(bool enabled) = 0;

  virtual const ui::MotionEvent* GetCurrentDownEvent() const = 0;

  virtual void ResetDetection() = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GESTURE_PROVIDER_H_
