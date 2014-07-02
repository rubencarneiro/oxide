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

#ifndef _OXIDE_QT_CORE_BASE_EVENT_UTILS_H_
#define _OXIDE_QT_CORE_BASE_EVENT_UTILS_H_

#include <map>

#include <QtGlobal>

#include "base/memory/scoped_vector.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/events/event.h"

QT_BEGIN_NAMESPACE
class QKeyEvent;
class QMouseEvent;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

typedef std::map<int, int> TouchIDMap;

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(QKeyEvent* event,
                                                           bool is_char);

void MakeUITouchEvents(QTouchEvent* event,
                       float device_scale,
                       TouchIDMap* map,
                       ScopedVector<ui::TouchEvent>* results);

blink::WebMouseEvent MakeWebMouseEvent(QMouseEvent* event, float device_scale);

blink::WebMouseWheelEvent MakeWebMouseWheelEvent(QWheelEvent* event,
                                                 float device_scale);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BASE_EVENT_UTILS_H_
