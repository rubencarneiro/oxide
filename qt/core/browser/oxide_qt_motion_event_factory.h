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

#ifndef _OXIDE_QT_CORE_BROWSER_MOTION_EVENT_FACTORY_H_
#define _OXIDE_QT_CORE_BROWSER_MOTION_EVENT_FACTORY_H_

#include <memory>
#include <QtGlobal>
#include <QEvent>
#include <QTouchEvent>

#include "base/callback.h"
#include "base/containers/stack_container.h"
#include "base/macros.h"
#include "ui/events/gesture_detection/motion_event_generic.h"

#include "shared/common/oxide_id_allocator.h"

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

namespace base {
class TimeTicks;
}

namespace oxide {
namespace qt {

// A helper class to produce ui::MotionEvents from QTouchEvents
class MotionEventFactory {
 public:
  MotionEventFactory();
  ~MotionEventFactory();

  std::unique_ptr<ui::MotionEvent> Cancel();

  using ResultVector = std::vector<std::unique_ptr<ui::MotionEvent>>;

  // Make a sequence of ui::MotionEvents from the specified QTouchEvent. This
  // also updates pointer state internally.
  // Specified QTouchEvents must be part of a valid touch event sequence
  void MakeMotionEvents(QTouchEvent* event,
                        QScreen* screen,
                        float location_bar_content_offset,
                        ResultVector* out);

 private:
  static const size_t kMaxTouchPoints = ui::MotionEvent::MAX_TOUCH_POINT_COUNT;

  struct PointerProperties : public ui::PointerProperties {
    PointerProperties();

    int platform_id;
    float location_bar_content_offset;
  };

  ui::MotionEvent::Action DetermineAction(Qt::TouchPointState state) const;
  int GetPointerIndexForTouchPoint(
      const QTouchEvent::TouchPoint& touch_point) const;
  bool HasTouchPoint(const QTouchEvent::TouchPoint& touch_point) const;
  int UpdateTouchPoint(const QTouchEvent::TouchPoint& touch_point,
                       QScreen* screen);
  int AddTouchPoint(const QTouchEvent::TouchPoint& touch_point,
                    float location_bar_content_offset,
                    QScreen* screen);
  std::unique_ptr<ui::MotionEventGeneric> BuildMotionEventCommon(
      ui::MotionEvent::Action action,
      const base::TimeTicks& event_time);
  std::unique_ptr<ui::MotionEvent> BuildMotionEvent(
      const QTouchEvent::TouchPoint& touch_point,
      float location_bar_content_offset,
      QScreen* screen,
      QEvent::Type type,
      const base::TimeTicks& event_time);

  oxide::IdAllocator id_allocator_;

  base::StackVector<PointerProperties, kMaxTouchPoints> touch_points_;

  DISALLOW_COPY_AND_ASSIGN(MotionEventFactory);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_MOTION_EVENT_FACTORY_H_
