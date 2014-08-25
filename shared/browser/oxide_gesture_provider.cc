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

#include "oxide_gesture_provider.h"

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/time/time.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/event.h"
#include "ui/events/gesture_detection/filtered_gesture_provider.h"
#include "ui/events/gesture_detection/gesture_provider.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/events/gestures/gesture_sequence.h"
#include "ui/gfx/screen.h"

#include "shared/base/oxide_event_utils.h"
#include "shared/base/oxide_id_allocator.h"

namespace oxide {

namespace {

const double kDefaultRadius = 25.0f;

ui::GestureDetector::Config GetGestureDetectorConfig(float scale) {
  ui::GestureDetector::Config config;
  config.longpress_timeout = base::TimeDelta::FromMilliseconds(500);
  config.showpress_timeout = base::TimeDelta::FromMilliseconds(180);
  config.double_tap_timeout = base::TimeDelta::FromMilliseconds(300);

  config.touch_slop = 8 * scale;
  config.double_tap_slop = 100 * scale;
  config.minimum_fling_velocity = 50.0f * scale;
  config.maximum_fling_velocity = 8000.0f * scale;

  return config;
}

ui::ScaleGestureDetector::Config GetScaleGestureDetectorConfig(float scale) {
  ui::ScaleGestureDetector::Config config;
  config.min_scaling_touch_major = kDefaultRadius * 2 * scale;
  config.min_scaling_span = 170 * scale;

  return config;
}

ui::GestureProvider::Config GetGestureProviderConfig() {
  ui::GestureProvider::Config config;
  config.display = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();

  const float scale = 1.0f / config.display.device_scale_factor();

  config.gesture_detector_config = GetGestureDetectorConfig(scale);
  config.scale_gesture_detector_config = GetScaleGestureDetectorConfig(scale);
  config.gesture_begin_end_types_enabled = false;
  config.min_gesture_bounds_length = kDefaultRadius * scale;

  return config;
}

}

class MotionEvent : public ui::MotionEvent {
 public:
  MotionEvent();
  virtual ~MotionEvent();

  bool IsTouchIdActive(int id) const;
  void OnTouchEvent(const ui::TouchEvent& event);
  void RemoveInactiveTouchPoints();

  // ui::MotionEvent implementation
  scoped_ptr<ui::MotionEvent> Clone() const FINAL;

 private:
  static const size_t kMaxTouchPoints = ui::GestureSequence::kMaxGesturePoints;

  struct TouchPoint {
    TouchPoint();

    int platform_id;
    int id;
    float x;
    float y;
    float raw_x;
    float raw_y;
    float touch_major;
    float pressure;
    bool active;
  };

  MotionEvent(size_t pointer_count,
              size_t active_touch_point_count,
              const TouchPoint (&touch_points)[kMaxTouchPoints],
              Action action,
              int action_index,
              const base::TimeTicks& last_event_time);

  size_t GetIndexFromPlatformId(int id) const;
  bool HasPlatformId(int id) const;

  void AddTouchPoint(const ui::TouchEvent& event);
  void UpdateTouchPoint(const ui::TouchEvent& event);
  void UpdateAction(const ui::TouchEvent& event);

  // ui::MotionEvent implementation
  int GetId() const FINAL;
  Action GetAction() const FINAL;
  int GetActionIndex() const FINAL;
  size_t GetPointerCount() const FINAL;
  int GetPointerId(size_t pointer_index) const FINAL;
  float GetX(size_t pointer_index) const FINAL;
  float GetY(size_t pointer_index) const FINAL;
  float GetRawX(size_t pointer_index) const FINAL;
  float GetRawY(size_t pointer_index) const FINAL;
  float GetTouchMajor(size_t pointer_index) const FINAL;
  float GetPressure(size_t pointer_index) const FINAL;
  base::TimeTicks GetEventTime() const FINAL;

  size_t GetHistorySize() const FINAL;
  base::TimeTicks GetHistoricalEventTime(
      size_t historical_index) const FINAL;
  float GetHistoricalTouchMajor(size_t pointer_index,
                                size_t historical_index) const FINAL;
  float GetHistoricalX(size_t pointer_index,
                       size_t historical_index) const FINAL;
  float GetHistoricalY(size_t pointer_index,
                       size_t historical_index) const FINAL;
  ToolType GetToolType(size_t pointer_index) const FINAL;
  int GetButtonState() const FINAL;

  scoped_ptr<ui::MotionEvent> Cancel() const FINAL;

  static TouchPoint CreateTouchPointFromEvent(const ui::TouchEvent& event);

  size_t pointer_count_;
  size_t active_touch_point_count_;
  TouchPoint touch_points_[kMaxTouchPoints];

  IdAllocator id_allocator_;

  Action action_;
  int action_index_;

  base::TimeTicks last_event_time_;
};

MotionEvent::TouchPoint::TouchPoint()
    : platform_id(0), id(0), x(0), y(0), raw_x(0), raw_y(0),
      touch_major(0), pressure(0), active(true) {}

MotionEvent::MotionEvent(
    size_t pointer_count,
    size_t active_touch_point_count,
    const TouchPoint (&touch_points)[kMaxTouchPoints],
    Action action,
    int action_index,
    const base::TimeTicks& last_event_time)
    : pointer_count_(pointer_count),
      active_touch_point_count_(active_touch_point_count),
      id_allocator_(kMaxTouchPoints - 1),
      action_(action),
      action_index_(action_index),
      last_event_time_(last_event_time) {
  for (size_t i = 0; i < pointer_count_; ++i) {
    touch_points_[i] = touch_points[i];
    id_allocator_.MarkAsUsed(touch_points[i].id);
  }
}

size_t MotionEvent::GetIndexFromPlatformId(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return i;
    }
  }

  NOTREACHED();
  return 0;
}

bool MotionEvent::HasPlatformId(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return true;
    }
  }

  return false;
}

void MotionEvent::AddTouchPoint(const ui::TouchEvent& event) {
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

void MotionEvent::UpdateTouchPoint(
    const ui::TouchEvent& event) {
  size_t index = GetIndexFromPlatformId(event.touch_id());
  bool was_active = touch_points_[index].active;
  int id = touch_points_[index].id;

  touch_points_[index] = CreateTouchPointFromEvent(event);
  touch_points_[index].id = id;

  DCHECK(was_active || !touch_points_[index].active);

  if (!touch_points_[index].active && was_active) {
    DCHECK_GT(active_touch_point_count_, 0);
    active_touch_point_count_--;
  }
}

void MotionEvent::UpdateAction(const ui::TouchEvent& event) {
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

int MotionEvent::GetId() const {
  return GetPointerId(0);
}

ui::MotionEvent::Action MotionEvent::GetAction() const {
  return action_;
}

int MotionEvent::GetActionIndex() const {
  DCHECK(action_ == ACTION_POINTER_DOWN ||
         action_ == ACTION_POINTER_UP);
  DCHECK_GE(action_index_, 0);
  DCHECK_LT(action_index_, pointer_count_);
  return action_index_;
}

size_t MotionEvent::GetPointerCount() const {
  return pointer_count_;
}

int MotionEvent::GetPointerId(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].id;
}

float MotionEvent::GetX(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].x;
}

float MotionEvent::GetY(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].y;
}

float MotionEvent::GetRawX(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].raw_x;
}

float MotionEvent::GetRawY(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].raw_y;
}

float MotionEvent::GetTouchMajor(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].touch_major * 2;
}

float MotionEvent::GetPressure(size_t pointer_index) const {
  DCHECK_LE(pointer_index, pointer_count_);
  return touch_points_[pointer_index].pressure;
}

base::TimeTicks MotionEvent::GetEventTime() const {
  return last_event_time_;
}

size_t MotionEvent::GetHistorySize() const {
  return 0;
}

base::TimeTicks MotionEvent::GetHistoricalEventTime(
    size_t historical_index) const {
  NOTREACHED();
  return base::TimeTicks();
}

float MotionEvent::GetHistoricalTouchMajor(size_t pointer_index,
                                           size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

float MotionEvent::GetHistoricalX(size_t pointer_index,
                                  size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

float MotionEvent::GetHistoricalY(size_t pointer_index,
                                  size_t historical_index) const {
  NOTREACHED();
  return 0.0f;
}

ui::MotionEvent::ToolType MotionEvent::GetToolType(
    size_t pointer_index) const {
  NOTREACHED();
  return TOOL_TYPE_UNKNOWN;
}

int MotionEvent::GetButtonState() const {
  NOTREACHED();
  return 0;
}

scoped_ptr<ui::MotionEvent> MotionEvent::Cancel() const {
  return make_scoped_ptr(
      new MotionEvent(pointer_count_, active_touch_point_count_,
                      touch_points_, ACTION_CANCEL, -1,
                      last_event_time_)).PassAs<ui::MotionEvent>();
}

// static
MotionEvent::TouchPoint MotionEvent::CreateTouchPointFromEvent(
    const ui::TouchEvent& event) {
  TouchPoint result;

  result.platform_id = event.touch_id();
  result.x = event.x();
  result.y = event.y();
  result.raw_x = event.root_location_f().x();
  result.raw_y = event.root_location_f().y();
  result.pressure = event.force();

  result.touch_major = std::max(event.radius_x(), event.radius_y());
  if (result.touch_major == 0.0f) {
    result.touch_major = kDefaultRadius;
  }

  result.active = event.type() == ui::ET_TOUCH_PRESSED ||
                  event.type() == ui::ET_TOUCH_MOVED;

  return result;
}

MotionEvent::MotionEvent()
    : pointer_count_(0),
      active_touch_point_count_(0),
      id_allocator_(kMaxTouchPoints - 1),
      action_index_(-1) {}

MotionEvent::~MotionEvent() {}

bool MotionEvent::IsTouchIdActive(int id) const {
  for (size_t i = 0; i < pointer_count_; ++i) {
    if (touch_points_[i].platform_id == id) {
      return touch_points_[i].active;
    }
  }

  return false;
}

void MotionEvent::OnTouchEvent(const ui::TouchEvent& event) {
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
  last_event_time_ = event.time_stamp() + base::TimeTicks();
}

void MotionEvent::RemoveInactiveTouchPoints() {
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

scoped_ptr<ui::MotionEvent> MotionEvent::Clone() const {
  return make_scoped_ptr(
      new MotionEvent(pointer_count_, active_touch_point_count_,
                      touch_points_, action_, action_index_,
                      last_event_time_)).PassAs<ui::MotionEvent>();
}

class GestureProviderImpl : public GestureProvider,
                            public ui::GestureProviderClient {
 public:
  GestureProviderImpl(oxide::GestureProviderClient* client);
  virtual ~GestureProviderImpl();

 private:
  // GestureProvider implementation
  bool OnTouchEvent(const ui::TouchEvent& event) FINAL;
  void OnTouchEventAck(bool consumed) FINAL;

  scoped_ptr<ui::MotionEvent> GetTouchState() const FINAL;

  void SetDoubleTapSupportForPageEnabled(bool enabled) FINAL;

  // ui::GestureProviderClient implementation
  void OnGestureEvent(const ui::GestureEventData& gesture) FINAL;

  // Need the oxide identifier here, else this becomes
  // "ui::GestureProviderClient"
  oxide::GestureProviderClient* client_;
  MotionEvent touch_state_;
  ui::FilteredGestureProvider filtered_gesture_provider_;

  DISALLOW_COPY_AND_ASSIGN(GestureProviderImpl);
};

bool GestureProviderImpl::OnTouchEvent(const ui::TouchEvent& event) {
  switch (event.type()) {
    case ui::ET_TOUCH_PRESSED:
      if (touch_state_.IsTouchIdActive(event.touch_id())) {
        return false;
      }
      break;
    case ui::ET_TOUCH_MOVED:
    case ui::ET_TOUCH_RELEASED:
    case ui::ET_TOUCH_CANCELLED:
      if (!touch_state_.IsTouchIdActive(event.touch_id())) {
        return false;
      }
      break;
    default:
      NOTREACHED();
  }

  touch_state_.OnTouchEvent(event);
  bool result = filtered_gesture_provider_.OnTouchEvent(touch_state_);
  if (!result) {
    touch_state_.RemoveInactiveTouchPoints();
  }

  return result;
}

void GestureProviderImpl::OnTouchEventAck(bool consumed) {
  filtered_gesture_provider_.OnTouchEventAck(consumed);
  touch_state_.RemoveInactiveTouchPoints();
}

scoped_ptr<ui::MotionEvent> GestureProviderImpl::GetTouchState() const {
  return touch_state_.Clone();
}

void GestureProviderImpl::SetDoubleTapSupportForPageEnabled(bool enabled) {
  filtered_gesture_provider_.SetDoubleTapSupportForPageEnabled(enabled);
}

void GestureProviderImpl::OnGestureEvent(const ui::GestureEventData& gesture) {
  client_->OnGestureEvent(MakeWebGestureEvent(gesture));
}

GestureProviderImpl::GestureProviderImpl(oxide::GestureProviderClient* client)
    : client_(client),
      filtered_gesture_provider_(GetGestureProviderConfig(), this) {
  filtered_gesture_provider_.SetDoubleTapSupportForPlatformEnabled(true);
}

GestureProviderImpl::~GestureProviderImpl() {}

GestureProviderClient::~GestureProviderClient() {}

// static
scoped_ptr<GestureProvider> GestureProvider::Create(
    GestureProviderClient* client) {
  DCHECK(client) << "A GestureProviderClient must be provided";
  return make_scoped_ptr(
      new GestureProviderImpl(client)).PassAs<GestureProvider>();
}

GestureProvider::~GestureProvider() {}

} // namespace oxide
