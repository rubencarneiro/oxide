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

#ifndef _OXIDE_QT_CORE_BROWSER_EVENT_UTILS_H_
#define _OXIDE_QT_CORE_BROWSER_EVENT_UTILS_H_

#include <map>
#include <QtGlobal>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/event.h"

QT_BEGIN_NAMESPACE
class QHoverEvent;
class QKeyEvent;
class QMouseEvent;
class QPoint;
class QPointF;
class QScreen;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class UITouchEventFactory final {
 public:
  UITouchEventFactory();
  ~UITouchEventFactory();

  void MakeEvents(QTouchEvent* event,
                  QScreen* screen,
                  float location_bar_content_offset,
                  ScopedVector<ui::TouchEvent>* results);
  scoped_ptr<ui::TouchEvent> Cancel();

 private:
  std::map<int, double> touch_point_content_offsets_;

  DISALLOW_COPY_AND_ASSIGN(UITouchEventFactory);
};

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(QKeyEvent* event,
                                                           bool is_char);

blink::WebMouseEvent MakeWebMouseEvent(QMouseEvent* event,
                                       QScreen* screen,
                                       float location_bar_content_offset);

blink::WebMouseWheelEvent MakeWebMouseWheelEvent(
    QWheelEvent* event,
    const QPointF& window_pos,
    QScreen* screen,
    float location_bar_content_offset);

blink::WebMouseEvent MakeWebMouseEvent(
    QHoverEvent* event,
    const QPointF& window_pos,
    const QPoint& global_pos,
    QScreen* screen,
    float location_bar_content_offset);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_EVENT_UTILS_H_
