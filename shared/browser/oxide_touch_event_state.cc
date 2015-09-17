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

#include "oxide_touch_event_state.h"

#include "base/logging.h"
#include "ui/events/base_event_utils.h"
#include "ui/events/event.h"

namespace oxide {

namespace {
const double kDefaultRadius = 24.0f;
}

TouchEventState::TouchPoint::TouchPoint()
    : platform_id(0), id(0), x(0), y(0), raw_x(0), raw_y(0),
      pressure(0), active(true) {}

bool TouchEventState::IsTouchIdActive(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return touch_points_[i].active;
    }
  }

  return false;
}

void TouchEventState::RemoveInactiveTouchPoints() {
  if (pointer_count_ == active_touch_point_count_) {
    return;
  }

  size_t i = 0;
  while (i < pointer_count_) {
    while (!touch_points_[i].active && i < pointer_count_) {
      id_allocator_.FreeId(touch_points_[i].id);
      pointer_count_--;
      if (pointer_count_ > i) {
        touch_points_[i] = touch_points_[pointer_count_];
      }
    }

    i++;
  }

  DCHECK_EQ(pointer_count_, active_touch_point_count_);
}

size_t TouchEventState::GetIndexFromPlatformId(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return i;
    }
  }

  NOTREACHED();
  return 0;
}

bool TouchEventState::HasPlatformId(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return true;
    }
  }

  return false;
}

void TouchEventState::AddTouchPoint(const ui::TouchEvent& event) {
  DCHECK(!HasPlatformId(event.touch_id()));
  DCHECK_EQ(event.type(), ui::ET_TOUCH_PRESSED);

  if (pointer_count_ == static_cast<size_t>(kMaxTouchPoints)) {
    return;
  }

  touch_points_[pointer_count_] = CreateTouchPointFromEvent(event);
  touch_points_[pointer_count_].id = id_allocator_.AllocateId();

  DCHECK_NE(touch_points_[pointer_count_].id, kInvalidId);
  DCHECK(touch_points_[pointer_count_].active);
  DCHECK_LE(active_touch_point_count_, pointer_count_);

  pointer_count_++;
  active_touch_point_count_++;
}

void TouchEventState::UpdateTouchPoint(const ui::TouchEvent& event) {
  size_t index = GetIndexFromPlatformId(event.touch_id());
  bool was_active = touch_points_[index].active;
  int id = touch_points_[index].id;

  touch_points_[index] = CreateTouchPointFromEvent(event);
  touch_points_[index].id = id;

  DCHECK(was_active || !touch_points_[index].active);

  if (!touch_points_[index].active && was_active) {
    DCHECK_GT(active_touch_point_count_, 0U);
    active_touch_point_count_--;
  }
}

void TouchEventState::UpdateAction(const ui::TouchEvent& event) {
  switch (event.type()) {
    case ui::ET_TOUCH_RELEASED:
      if (active_touch_point_count_ == 0) {
        action_ = ACTION_UP;
      } else {
        action_ = ACTION_POINTER_UP;
        action_index_ =
            static_cast<int>(GetIndexFromPlatformId(event.touch_id()));
      }
      break;
    case ui::ET_TOUCH_PRESSED:
      if (active_touch_point_count_ == 1) {
        action_ = ACTION_DOWN;
      } else {
        action_ = ACTION_POINTER_DOWN;
        action_index_ =
            static_cast<int>(GetIndexFromPlatformId(event.touch_id()));
      }
      break;
    case ui::ET_TOUCH_MOVED:
      action_ = ACTION_MOVE;
      break;
    case ui::ET_TOUCH_CANCELLED:
      action_ = ACTION_CANCEL;
      break;
    default:
      NOTREACHED();
  }
}

uint32_t TouchEventState::GetUniqueEventId() const {
  return unique_event_id_;
}

ui::MotionEvent::Action TouchEventState::GetAction() const {
  return action_;
}

int TouchEventState::GetActionIndex() const {
  DCHECK(action_ == ACTION_POINTER_DOWN ||
         action_ == ACTION_POINTER_UP);
  DCHECK_GE(action_index_, 0);
  DCHECK_LT(action_index_, static_cast<int>(pointer_count_));
  return action_index_;
}

size_t TouchEventState::GetPointerCount() const {
  return pointer_count_;
}

int TouchEventState::GetPointerId(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].id;
}

float TouchEventState::GetX(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].x;
}

float TouchEventState::GetY(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].y;
}

float TouchEventState::GetRawX(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].raw_x;
}

float TouchEventState::GetRawY(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].raw_y;
}

float TouchEventState::GetTouchMajor(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return kDefaultRadius * 2;
}

float TouchEventState::GetTouchMinor(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return kDefaultRadius * 2;
}

float TouchEventState::GetOrientation(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return 0;
}

float TouchEventState::GetPressure(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].pressure;
}

base::TimeTicks TouchEventState::GetEventTime() const {
  return last_event_time_;
}

size_t TouchEventState::GetHistorySize() const {
  return 0;
}

base::TimeTicks TouchEventState::GetHistoricalEventTime(
    size_t historical_index) const {
  NOTREACHED();
  return base::TimeTicks();
}

float TouchEventState::GetHistoricalTouchMajor(size_t pointer_index,
                                               size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

float TouchEventState::GetHistoricalX(size_t pointer_index,
                                      size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

float TouchEventState::GetHistoricalY(size_t pointer_index,
                                      size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

ui::MotionEvent::ToolType TouchEventState::GetToolType(
    size_t pointer_index) const {
  return TOOL_TYPE_UNKNOWN;
}

int TouchEventState::GetButtonState() const {
  return 0;
}

int TouchEventState::GetFlags() const {
  return flags_;
}

// static
TouchEventState::TouchPoint TouchEventState::CreateTouchPointFromEvent(
    const ui::TouchEvent& event) {
  TouchPoint result;

  result.platform_id = event.touch_id();
  result.x = event.x();
  result.y = event.y();
  result.raw_x = event.root_location_f().x();
  result.raw_y = event.root_location_f().y();
  result.pressure = event.pointer_details().force();

  DCHECK_EQ(event.pointer_details().radius_x(), 0);
  DCHECK_EQ(event.pointer_details().radius_y(), 0);
  DCHECK_EQ(event.rotation_angle(), 0);

  result.active = event.type() == ui::ET_TOUCH_PRESSED ||
                  event.type() == ui::ET_TOUCH_MOVED;

  return result;
}

TouchEventState::TouchEventState()
    : pointer_count_(0),
      active_touch_point_count_(0),
      id_allocator_(kMaxTouchPoints - 1),
      action_index_(-1),
      flags_(0) {}

TouchEventState::~TouchEventState() {}

bool TouchEventState::Update(const ui::TouchEvent& event) {
  switch (event.type()) {
    case ui::ET_TOUCH_PRESSED:
      if (IsTouchIdActive(event.touch_id())) {
        return false;
      }
      break;
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_RELEASED:
    case ui::ET_TOUCH_CANCELLED:
      if (!IsTouchIdActive(event.touch_id())) {
        return false;
      }
      break;
    default:
      NOTREACHED();
  }

  RemoveInactiveTouchPoints();

  switch (event.type()) {
    case ui::ET_TOUCH_PRESSED:
      AddTouchPoint(event);
      break;
    case ui::ET_TOUCH_RELEASED:
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_CANCELLED:
      UpdateTouchPoint(event);
      break;
    default:
      NOTREACHED();
  }

  UpdateAction(event);
  unique_event_id_ = event.unique_event_id();
  flags_ = event.flags();
  last_event_time_ = event.time_stamp() + base::TimeTicks();

  return true;
}

} // namespace oxide
