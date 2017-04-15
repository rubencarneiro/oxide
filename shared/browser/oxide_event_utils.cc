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

#include "oxide_event_utils.h"

#include "base/logging.h"
#include "base/time/time.h"
#include "third_party/WebKit/public/platform/WebPointerProperties.h"
#include "third_party/WebKit/public/platform/WebTouchPoint.h"
#include "ui/events/gesture_detection/gesture_event_data.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/events/keycodes/keyboard_codes.h"

namespace oxide {

namespace {

blink::WebTouchPoint CreateWebTouchPoint(const ui::MotionEvent& event,
                                         size_t pointer_index) {
  blink::WebTouchPoint result;

  result.pointer_type = blink::WebPointerProperties::PointerType::kTouch;
  result.id = event.GetPointerId(pointer_index);

  switch (event.GetAction()) {
    case ui::MotionEvent::ACTION_DOWN:
      result.state = blink::WebTouchPoint::kStatePressed;
      break;
    case ui::MotionEvent::ACTION_UP:
      result.state = blink::WebTouchPoint::kStateReleased;
      break;
    case ui::MotionEvent::ACTION_MOVE:
      result.state = blink::WebTouchPoint::kStateMoved;
      break;
    case ui::MotionEvent::ACTION_CANCEL:
      result.state = blink::WebTouchPoint::kStateCancelled;
      break;
    case ui::MotionEvent::ACTION_POINTER_DOWN:
      result.state =
          pointer_index == static_cast<size_t>(event.GetActionIndex()) ?
            blink::WebTouchPoint::kStatePressed :
            blink::WebTouchPoint::kStateStationary;
      break;
    case ui::MotionEvent::ACTION_POINTER_UP:
      result.state =
          pointer_index == static_cast<size_t>(event.GetActionIndex()) ?
            blink::WebTouchPoint::kStateReleased :
            blink::WebTouchPoint::kStateStationary;
      break;
    default:
      NOTREACHED();
      result.state = blink::WebTouchPoint::kStateUndefined;
  }

  result.screen_position.x = event.GetRawX(pointer_index);
  result.screen_position.y = event.GetRawY(pointer_index);
  result.position.x = event.GetX(pointer_index);
  result.position.y = event.GetY(pointer_index);

  result.radius_x = result.radius_y = event.GetTouchMajor(pointer_index);
  result.force = event.GetPressure(pointer_index);

  return result;
}

}

blink::WebGestureEvent MakeWebGestureEvent(
    const ui::GestureEventData& gesture) {
  blink::WebGestureEvent result;
  result.x = gesture.x;
  result.y = gesture.y;
  result.global_x = gesture.raw_x;
  result.global_y = gesture.raw_y;

  result.SetTimeStampSeconds((gesture.time - base::TimeTicks()).InSecondsF());
  result.source_device = blink::kWebGestureDeviceTouchscreen;

  switch (gesture.type()) {
    case ui::ET_GESTURE_SHOW_PRESS:
      result.SetType(blink::WebInputEvent::kGestureShowPress);
      result.data.show_press.width = gesture.details.bounding_box_f().width();
      result.data.show_press.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_DOUBLE_TAP:
      result.SetType(blink::WebInputEvent::kGestureDoubleTap);
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tap_count = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_TAP:
      result.SetType(blink::WebInputEvent::kGestureTap);
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tap_count = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_TAP_UNCONFIRMED:
      result.SetType(blink::WebInputEvent::kGestureTapUnconfirmed);
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tap_count = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_LONG_PRESS:
      result.SetType(blink::WebInputEvent::kGestureLongPress);
      result.data.long_press.width = gesture.details.bounding_box_f().width();
      result.data.long_press.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_LONG_TAP:
      result.SetType(blink::WebInputEvent::kGestureLongTap);
      result.data.long_press.width = gesture.details.bounding_box_f().width();
      result.data.long_press.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_SCROLL_BEGIN:
      result.SetType(blink::WebInputEvent::kGestureScrollBegin);
      result.data.scroll_begin.delta_x_hint = gesture.details.scroll_x_hint();
      result.data.scroll_begin.delta_y_hint = gesture.details.scroll_y_hint();
      break;
    case ui::ET_GESTURE_SCROLL_UPDATE:
      result.SetType(blink::WebInputEvent::kGestureScrollUpdate);
      result.data.scroll_update.delta_x = gesture.details.scroll_x();
      result.data.scroll_update.delta_y = gesture.details.scroll_y();
      break;
    case ui::ET_GESTURE_SCROLL_END:
      result.SetType(blink::WebInputEvent::kGestureScrollEnd);
      break;
    case ui::ET_SCROLL_FLING_START:
      result.SetType(blink::WebInputEvent::kGestureFlingStart);
      result.data.fling_start.velocity_x = gesture.details.velocity_x();
      result.data.fling_start.velocity_y = gesture.details.velocity_y();
      break;
    case ui::ET_SCROLL_FLING_CANCEL:
      result.SetType(blink::WebInputEvent::kGestureFlingCancel);
      break;
    case ui::ET_GESTURE_PINCH_BEGIN:
      result.SetType(blink::WebInputEvent::kGesturePinchBegin);
      break;
    case ui::ET_GESTURE_PINCH_UPDATE:
      result.SetType(blink::WebInputEvent::kGesturePinchUpdate);
      result.data.pinch_update.scale = gesture.details.scale();
      break;
    case ui::ET_GESTURE_PINCH_END:
      result.SetType(blink::WebInputEvent::kGesturePinchEnd);
      break;
    case ui::ET_GESTURE_TAP_CANCEL:
      result.SetType(blink::WebInputEvent::kGestureTapCancel);
      break;
    case ui::ET_GESTURE_TAP_DOWN:
      result.SetType(blink::WebInputEvent::kGestureTapDown);
      result.data.tap_down.width = gesture.details.bounding_box_f().width();
      result.data.tap_down.height = gesture.details.bounding_box_f().height();
      break;
    default:
      NOTREACHED();
      break;
  }

  return result;
}

blink::WebTouchEvent MakeWebTouchEvent(const ui::MotionEvent& event,
                                       bool moved_beyond_slop_region) {
  blink::WebTouchEvent result;

  result.SetTimeStampSeconds(
      (event.GetEventTime() - base::TimeTicks()).InSecondsF());

  switch (event.GetAction()) {
    case ui::MotionEvent::ACTION_DOWN:
    case ui::MotionEvent::ACTION_POINTER_DOWN:
      result.SetType(blink::WebInputEvent::kTouchStart);
      break;
    case ui::MotionEvent::ACTION_UP:
    case ui::MotionEvent::ACTION_POINTER_UP:
      result.SetType(blink::WebInputEvent::kTouchEnd);
      break;
    case ui::MotionEvent::ACTION_MOVE:
      result.SetType(blink::WebInputEvent::kTouchMove);
      break;
    case ui::MotionEvent::ACTION_CANCEL:
      result.SetType(blink::WebInputEvent::kTouchCancel);
      break;
    default:
      NOTREACHED();
      result.SetType(blink::WebInputEvent::kUndefined);
  }

  result.dispatch_type =
      result.GetType() == blink::WebInputEvent::kTouchCancel ?
          blink::WebInputEvent::kEventNonBlocking :
          blink::WebInputEvent::kBlocking;

  result.moved_beyond_slop_region = moved_beyond_slop_region;

  result.touches_length = std::min(
      event.GetPointerCount(),
      static_cast<size_t>(blink::WebTouchEvent::kTouchesLengthCap));
  DCHECK_GT(result.touches_length, 0U);

  for (size_t i = 0; i < result.touches_length; ++i) {
    result.touches[i] = CreateWebTouchPoint(event, i);
  }

  return result;
}

int WindowsKeyCodeWithoutLocation(int code) {
  switch (code) {
    case ui::VKEY_LSHIFT:
    case ui::VKEY_RSHIFT:
      return ui::VKEY_SHIFT;
    case ui::VKEY_LCONTROL:
    case ui::VKEY_RCONTROL:
      return ui::VKEY_CONTROL;
    case ui::VKEY_LMENU:
    case ui::VKEY_RMENU:
      return ui::VKEY_MENU;
    default:
      return code;
  }
}

int LocationModifiersFromWindowsKeyCode(int code) {
  switch (code) {
    case ui::VKEY_LSHIFT:
    case ui::VKEY_LCONTROL:
    case ui::VKEY_LMENU:
      return blink::WebInputEvent::kIsLeft;
    case ui::VKEY_RSHIFT:
    case ui::VKEY_RCONTROL:
    case ui::VKEY_RMENU:
      return blink::WebInputEvent::kIsRight;
    default:
      return 0;
  }
}

} // namespace oxide
