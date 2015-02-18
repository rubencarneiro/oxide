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
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "third_party/WebKit/public/web/WebTouchPoint.h"
#include "ui/events/gesture_detection/gesture_event_data.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/events/keycodes/keyboard_codes.h"

namespace oxide {

namespace {

blink::WebTouchPoint CreateWebTouchPoint(const ui::MotionEvent& event,
                                         size_t pointer_index) {
  blink::WebTouchPoint result;

  result.id = event.GetPointerId(pointer_index);

  bool is_action_index =
      pointer_index == static_cast<size_t>(event.GetActionIndex());
  switch (event.GetAction()) {
    case ui::MotionEvent::ACTION_DOWN:
      result.state = blink::WebTouchPoint::StatePressed;
      break;
    case ui::MotionEvent::ACTION_UP:
      result.state = blink::WebTouchPoint::StateReleased;
      break;
    case ui::MotionEvent::ACTION_MOVE:
      result.state = blink::WebTouchPoint::StateMoved;
      break;
    case ui::MotionEvent::ACTION_CANCEL:
      result.state = blink::WebTouchPoint::StateCancelled;
      break;
    case ui::MotionEvent::ACTION_POINTER_DOWN:
      result.state = is_action_index ?
          blink::WebTouchPoint::StatePressed :
          blink::WebTouchPoint::StateStationary;
      break;
    case ui::MotionEvent::ACTION_POINTER_UP:
      result.state = is_action_index ?
          blink::WebTouchPoint::StateReleased :
          blink::WebTouchPoint::StateStationary;
      break;
    default:
      NOTREACHED();
      result.state = blink::WebTouchPoint::StateUndefined;
  }

  result.screenPosition.x = event.GetRawX(pointer_index);
  result.screenPosition.y = event.GetRawY(pointer_index);
  result.position.x = event.GetX(pointer_index);
  result.position.y = event.GetY(pointer_index);

  result.radiusX = result.radiusY = event.GetTouchMajor(pointer_index);
  result.force = event.GetPressure(pointer_index);

  return result;
}

}

blink::WebGestureEvent MakeWebGestureEvent(
    const ui::GestureEventData& gesture) {
  blink::WebGestureEvent result;
  result.x = gesture.x;
  result.y = gesture.y;
  result.globalX = gesture.raw_x;
  result.globalY = gesture.raw_y;

  result.timeStampSeconds = (gesture.time - base::TimeTicks()).InSecondsF();
  result.sourceDevice = blink::WebGestureDeviceTouchscreen;

  switch (gesture.type()) {
    case ui::ET_GESTURE_SHOW_PRESS:
      result.type = blink::WebInputEvent::GestureShowPress;
      result.data.showPress.width = gesture.details.bounding_box_f().width();
      result.data.showPress.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_DOUBLE_TAP:
      result.type = blink::WebInputEvent::GestureDoubleTap;
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tapCount = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_TAP:
      result.type = blink::WebInputEvent::GestureTap;
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tapCount = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_TAP_UNCONFIRMED:
      result.type = blink::WebInputEvent::GestureTapUnconfirmed;
      DCHECK_EQ(1, gesture.details.tap_count());
      result.data.tap.tapCount = gesture.details.tap_count();
      result.data.tap.width = gesture.details.bounding_box_f().width();
      result.data.tap.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_LONG_PRESS:
      result.type = blink::WebInputEvent::GestureLongPress;
      result.data.longPress.width = gesture.details.bounding_box_f().width();
      result.data.longPress.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_LONG_TAP:
      result.type = blink::WebInputEvent::GestureLongTap;
      result.data.longPress.width = gesture.details.bounding_box_f().width();
      result.data.longPress.height = gesture.details.bounding_box_f().height();
      break;
    case ui::ET_GESTURE_SCROLL_BEGIN:
      result.type = blink::WebInputEvent::GestureScrollBegin;
      result.data.scrollBegin.deltaXHint = gesture.details.scroll_x_hint();
      result.data.scrollBegin.deltaYHint = gesture.details.scroll_y_hint();
      break;
    case ui::ET_GESTURE_SCROLL_UPDATE:
      result.type = blink::WebInputEvent::GestureScrollUpdate;
      result.data.scrollUpdate.deltaX = gesture.details.scroll_x();
      result.data.scrollUpdate.deltaY = gesture.details.scroll_y();
      break;
    case ui::ET_GESTURE_SCROLL_END:
      result.type = blink::WebInputEvent::GestureScrollEnd;
      break;
    case ui::ET_SCROLL_FLING_START:
      result.type = blink::WebInputEvent::GestureFlingStart;
      result.data.flingStart.velocityX = gesture.details.velocity_x();
      result.data.flingStart.velocityY = gesture.details.velocity_y();
      break;
    case ui::ET_SCROLL_FLING_CANCEL:
      result.type = blink::WebInputEvent::GestureFlingCancel;
      break;
    case ui::ET_GESTURE_PINCH_BEGIN:
      result.type = blink::WebInputEvent::GesturePinchBegin;
      break;
    case ui::ET_GESTURE_PINCH_UPDATE:
      result.type = blink::WebInputEvent::GesturePinchUpdate;
      result.data.pinchUpdate.scale = gesture.details.scale();
      break;
    case ui::ET_GESTURE_PINCH_END:
      result.type = blink::WebInputEvent::GesturePinchEnd;
      break;
    case ui::ET_GESTURE_TAP_CANCEL:
      result.type = blink::WebInputEvent::GestureTapCancel;
      break;
    case ui::ET_GESTURE_TAP_DOWN:
      result.type = blink::WebInputEvent::GestureTapDown;
      result.data.tapDown.width = gesture.details.bounding_box_f().width();
      result.data.tapDown.height = gesture.details.bounding_box_f().height();
      break;
    default:
      NOTREACHED();
      break;
  }

  return result;
}

blink::WebTouchEvent MakeWebTouchEvent(const ui::MotionEvent& event,
                                       bool may_cause_scrolling) {
  blink::WebTouchEvent result;

  result.timeStampSeconds =
      (event.GetEventTime() - base::TimeTicks()).InSecondsF();

  switch (event.GetAction()) {
    case ui::MotionEvent::ACTION_DOWN:
    case ui::MotionEvent::ACTION_POINTER_DOWN:
      result.type = blink::WebInputEvent::TouchStart;
      break;
    case ui::MotionEvent::ACTION_UP:
    case ui::MotionEvent::ACTION_POINTER_UP:
      result.type = blink::WebInputEvent::TouchEnd;
      break;
    case ui::MotionEvent::ACTION_MOVE:
      result.type = blink::WebInputEvent::TouchMove;
      break;
    case ui::MotionEvent::ACTION_CANCEL:
      result.type = blink::WebInputEvent::TouchCancel;
      break;
    default:
      NOTREACHED();
      result.type = blink::WebInputEvent::Undefined;
  }

  result.cancelable = result.type != blink::WebInputEvent::TouchCancel;
  result.causesScrollingIfUncanceled = may_cause_scrolling;

  result.touchesLength = std::min(
      event.GetPointerCount(),
      static_cast<size_t>(blink::WebTouchEvent::touchesLengthCap));
  DCHECK_GT(result.touchesLength, 0U);

  for (size_t i = 0; i < result.touchesLength; ++i) {
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
      return blink::WebInputEvent::IsLeft;
    case ui::VKEY_RSHIFT:
    case ui::VKEY_RCONTROL:
    case ui::VKEY_RMENU:
      return blink::WebInputEvent::IsRight;
    default:
      return 0;
  }
}

} // namespace oxide
