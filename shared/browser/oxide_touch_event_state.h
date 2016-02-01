// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_EVENT_STATE_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_EVENT_STATE_H_

#include "base/macros.h"
#include "base/time/time.h"
#include "ui/events/gesture_detection/motion_event.h"

#include "shared/common/oxide_id_allocator.h"

namespace ui {
class TouchEvent;
}

namespace oxide {

// An implementation of MotionEvent that tracks incoming ui::TouchEvents
class TouchEventState : public ui::MotionEvent {
 public:
  TouchEventState();
  ~TouchEventState() override;

  bool Update(const ui::TouchEvent& event);

 private:
  static const size_t kMaxTouchPoints = ui::MotionEvent::MAX_TOUCH_POINT_COUNT;

  struct TouchPoint {
    TouchPoint();

    int platform_id;
    int id;
    float x;
    float y;
    float raw_x;
    float raw_y;
    float pressure;
    bool active;
  };

  bool IsTouchIdActive(int id) const;
  void MarkAllTouchPointsInactive();
  void RemoveInactiveTouchPoints();

  size_t GetIndexFromPlatformId(int id) const;
  bool HasPlatformId(int id) const;

  void AddTouchPoint(const ui::TouchEvent& event);
  void UpdateTouchPoint(const ui::TouchEvent& event);
  void UpdateAction(const ui::TouchEvent& event);

  // ui::MotionEvent implementation
  uint32_t GetUniqueEventId() const override;
  Action GetAction() const override;
  int GetActionIndex() const override;
  size_t GetPointerCount() const override;
  int GetPointerId(size_t pointer_index) const override;
  float GetX(size_t pointer_index) const override;
  float GetY(size_t pointer_index) const override;
  float GetRawX(size_t pointer_index) const override;
  float GetRawY(size_t pointer_index) const override;
  float GetTouchMajor(size_t pointer_index) const override;
  float GetTouchMinor(size_t pointer_index) const override;
  float GetOrientation(size_t pointer_index) const override;
  float GetPressure(size_t pointer_index) const override;
  float GetTilt(size_t pointer_index) const override;
  base::TimeTicks GetEventTime() const override;
  size_t GetHistorySize() const override;
  base::TimeTicks GetHistoricalEventTime(
      size_t historical_index) const override;
  float GetHistoricalTouchMajor(size_t pointer_index,
                                size_t historical_index) const override;
  float GetHistoricalX(size_t pointer_index,
                       size_t historical_index) const override;
  float GetHistoricalY(size_t pointer_index,
                       size_t historical_index) const override;
  ToolType GetToolType(size_t pointer_index) const override;
  int GetButtonState() const override;
  int GetFlags() const override;

  static TouchPoint CreateTouchPointFromEvent(const ui::TouchEvent& event);

  uint32_t unique_event_id_;

  size_t pointer_count_;
  size_t active_touch_point_count_;
  TouchPoint touch_points_[kMaxTouchPoints];

  IdAllocator id_allocator_;

  Action action_;
  int action_index_;

  int flags_;

  base::TimeTicks last_event_time_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_EVENT_STATE_H_
