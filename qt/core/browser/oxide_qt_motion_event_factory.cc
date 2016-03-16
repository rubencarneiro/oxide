// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "oxide_qt_motion_event_factory.h"

#include <algorithm>
#include <QPointF>
#include <QWindow>

#include "base/logging.h"
#include "base/time/time.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

const int kInvalidPointerIndex = -1;
const double kDefaultRadius = 24.0f;

}

class MotionEvent : public ui::MotionEventGeneric {
 public:
  MotionEvent() {}
};

MotionEventFactory::PointerProperties::PointerProperties()
    : platform_id(-1),
      location_bar_content_offset(0.f) {}

ui::MotionEvent::Action MotionEventFactory::DetermineAction(
    Qt::TouchPointState state) const {
  switch (state) {
    case Qt::TouchPointPressed:
      DCHECK_GT(touch_points_->size(), 0U);
      return touch_points_->size() == 1 ?
          ui::MotionEvent::ACTION_DOWN : ui::MotionEvent::ACTION_POINTER_DOWN;
    case Qt::TouchPointMoved:
      return ui::MotionEvent::ACTION_MOVE;
    case Qt::TouchPointStationary:
      NOTREACHED();
      return ui::MotionEvent::ACTION_NONE;
    case Qt::TouchPointReleased:
      DCHECK_GT(touch_points_->size(), 0U);
      return touch_points_->size() == 1 ?
          ui::MotionEvent::ACTION_UP : ui::MotionEvent::ACTION_POINTER_UP;
  }

  NOTREACHED();
  return ui::MotionEvent::ACTION_NONE;
}

int MotionEventFactory::GetPointerIndexForTouchPoint(
    const QTouchEvent::TouchPoint& touch_point) const {
  for (size_t i = 0; i < touch_points_->size(); ++i) {
    if (touch_points_[i].platform_id == touch_point.id()) {
      return static_cast<int>(i);
    }
  }

  return kInvalidPointerIndex;
}

bool MotionEventFactory::HasTouchPoint(
    const QTouchEvent::TouchPoint& touch_point) const {
  return std::find_if(touch_points_->begin(),
                      touch_points_->end(),
                      [&touch_point](const PointerProperties& p) {
    return p.platform_id == touch_point.id();
  }) != touch_points_->end();
}

int MotionEventFactory::UpdateTouchPoint(
    const QTouchEvent::TouchPoint& touch_point,
    QScreen* screen) {
  int index = GetPointerIndexForTouchPoint(touch_point);
  if (index == kInvalidPointerIndex) {
    return index;
  }

  PointerProperties& props = touch_points_[index];
  DCHECK_EQ(props.platform_id, touch_point.id());

  gfx::PointF pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(touch_point.pos()),
                                          screen);
  props.x = pos.x();
  props.y = pos.y() - props.location_bar_content_offset;

  // Not sure if screenPos is correct here
  gfx::PointF screen_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(touch_point.screenPos()),
                                          screen);
  props.raw_x = screen_pos.x();
  props.raw_y = screen_pos.y();

  props.pressure = touch_point.pressure();

  return index;
}

int MotionEventFactory::AddTouchPoint(
    const QTouchEvent::TouchPoint& touch_point,
    float location_bar_content_offset,
    QScreen* screen) {
  DCHECK(!HasTouchPoint(touch_point));

  int id = id_allocator_.AllocateId();
  if (id == kInvalidId) {
    LOG(ERROR) << "Attempt to track more touch points than we support";
    return kInvalidPointerIndex;
  }

  touch_points_->push_back(PointerProperties());

  PointerProperties& props = touch_points_->back();
  props.id = id;
  props.platform_id = touch_point.id();
  props.location_bar_content_offset = location_bar_content_offset;

  props.touch_major = props.touch_minor = kDefaultRadius * 2;

  return UpdateTouchPoint(touch_point, screen);
}

scoped_ptr<ui::MotionEventGeneric> MotionEventFactory::BuildMotionEventCommon(
    ui::MotionEvent::Action action,
    const base::TimeTicks& event_time) {
  scoped_ptr<ui::MotionEventGeneric> event(new MotionEvent());

  for (const auto& pointer : touch_points_.container()) {
    event->PushPointer(pointer);
  }

  event->set_action(action);
  event->set_event_time(event_time);

  return std::move(event);
}

scoped_ptr<ui::MotionEvent> MotionEventFactory::BuildMotionEvent(
    const QTouchEvent::TouchPoint& touch_point,
    float location_bar_content_offset,
    QScreen* screen,
    QEvent::Type type,
    const base::TimeTicks& event_time) {
  int pointer_index;
  if (touch_point.state() == Qt::TouchPointPressed) {
    pointer_index =
        AddTouchPoint(touch_point, location_bar_content_offset, screen);
  } else {
    pointer_index = UpdateTouchPoint(touch_point, screen);
  }

  if (pointer_index == kInvalidPointerIndex) {
    return nullptr;
  }

  scoped_ptr<ui::MotionEventGeneric> motion_event(
      BuildMotionEventCommon(DetermineAction(touch_point.state()),
                             event_time));
  motion_event->set_action_index(pointer_index);

  // TODO: Implement
  motion_event->set_flags(0);

  if (touch_point.state() == Qt::TouchPointReleased) {
    id_allocator_.FreeId(touch_points_[pointer_index].id);
    touch_points_->erase(touch_points_->begin() + pointer_index);
  }

  return std::move(motion_event);
}

MotionEventFactory::MotionEventFactory()
    : id_allocator_(kMaxTouchPoints) {}

MotionEventFactory::~MotionEventFactory() {}

scoped_ptr<ui::MotionEvent> MotionEventFactory::Cancel() {
  scoped_ptr<ui::MotionEventGeneric> motion_event(
      BuildMotionEventCommon(ui::MotionEvent::ACTION_CANCEL,
                             base::TimeTicks::Now()));

  for (const auto& pointer : touch_points_.container()) {
    id_allocator_.FreeId(pointer.id);
  }
  touch_points_->clear();

  return std::move(motion_event);
}

void MotionEventFactory::MakeMotionEvents(QTouchEvent* event,
                                          QScreen* screen,
                                          float location_bar_content_offset,
                                          ResultVector* out) {
  DCHECK(out);

  if (event->type() == QEvent::TouchCancel) {
    scoped_ptr<ui::MotionEvent> cancel_event = Cancel();
    out->push_back(std::move(cancel_event));
    return;
  }

  // The event’s timestamp is not guaranteed to have the same origin as the
  // internal timedelta used by chromium to calculate speed and displacement
  // for a fling gesture, so we can’t use it.
  base::TimeTicks timestamp = base::TimeTicks::Now();

  for (const auto& touch_point : event->touchPoints()) {
    if (touch_point.state() == Qt::TouchPointStationary) {
      continue;
    }

    scoped_ptr<ui::MotionEvent> motion_event(
        BuildMotionEvent(touch_point,
                         location_bar_content_offset,
                         screen,
                         event->type(),
                         timestamp));
    if (!motion_event) {
      continue;
    }

    out->push_back(std::move(motion_event));
  }
}

} // namespace qt
} // namespace oxide
