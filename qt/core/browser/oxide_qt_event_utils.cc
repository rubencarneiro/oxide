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

#include "oxide_qt_event_utils.h"

#include <QEvent>
#include <QHoverEvent>
#include <QInputEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <Qt>
#include <QWheelEvent>

#include "base/environment.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/geometry/point_f.h"

#include "shared/browser/oxide_event_utils.h"

#include "keyboard_code_conversion.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

// From content/browser/renderer_host/web_input_event_aura.cc
blink::WebUChar GetControlCharacter(int key_code, bool shift) {
  if (key_code >= ui::VKEY_A && key_code <= ui::VKEY_Z) {
    // ctrl-A ~ ctrl-Z map to \x01 ~ \x1A
    return key_code - ui::VKEY_A + 1;
  }
  if (shift) {
    // following graphics chars require shift key to input.
    switch (key_code) {
      // ctrl-@ maps to \x00 (Null byte)
      case ui::VKEY_2:
        return 0;
      // ctrl-^ maps to \x1E (Record separator, Information separator two)
      case ui::VKEY_6:
        return 0x1E;
      // ctrl-_ maps to \x1F (Unit separator, Information separator one)
      case ui::VKEY_OEM_MINUS:
        return 0x1F;
      // Returns 0 for all other keys to avoid inputting unexpected chars.
      default:
        break;
    }
  } else {
    switch (key_code) {
      // ctrl-[ maps to \x1B (Escape)
      case ui::VKEY_OEM_4:
        return 0x1B;
      // ctrl-\ maps to \x1C (File separator, Information separator four)
      case ui::VKEY_OEM_5:
        return 0x1C;
      // ctrl-] maps to \x1D (Group separator, Information separator three)
      case ui::VKEY_OEM_6:
        return 0x1D;
      // ctrl-Enter maps to \x0A (Line feed)
      case ui::VKEY_RETURN:
        return 0x0A;
      // Returns 0 for all other keys to avoid inputting unexpected chars.
      default:
        break;
    }
  }
  return 0;
}

double QInputEventTimeToWebEventTime(QInputEvent* qevent) {
  static bool g_testing_mode = []() {
    std::unique_ptr<base::Environment> env = base::Environment::Create();
    std::string result;
    return env->GetVar("OXIDE_TESTING_MODE", &result) &&
               result.size() > 0 &&
               result[0] != '0';
  }();

  if (g_testing_mode) {
    // We don't have timestamps in testing
    return base::TimeDelta(base::TimeTicks::Now() - base::TimeTicks())
        .InSecondsF();
  }

  return double(qevent->timestamp()) / 1000;
}

int QInputEventStateToWebEventModifiers(QInputEvent* qevent) {
  Qt::KeyboardModifiers qmodifiers = qevent->modifiers();

  int modifiers = 0;
  if (qmodifiers & Qt::ShiftModifier) {
    modifiers |= blink::WebInputEvent::ShiftKey;
  }
  if (qmodifiers & Qt::ControlModifier) {
    modifiers |= blink::WebInputEvent::ControlKey;
  }
  if (qmodifiers & Qt::AltModifier) {
    modifiers |= blink::WebInputEvent::AltKey;
  }
  if (qmodifiers & Qt::MetaModifier) {
    modifiers |= blink::WebInputEvent::MetaKey;
  }
  if (qmodifiers & Qt::KeypadModifier) {
    modifiers |= blink::WebInputEvent::IsKeyPad;
  }

  return modifiers;
}

int QMouseEventStateToWebEventModifiers(QMouseEvent* qevent) {
  Qt::MouseButtons buttons = qevent->buttons();

  int modifiers = 0;

  if (buttons & Qt::LeftButton) {
    modifiers |= blink::WebInputEvent::LeftButtonDown;
  }
  if (buttons & Qt::MidButton) {
    modifiers |= blink::WebInputEvent::MiddleButtonDown;
  }
  if (buttons & Qt::RightButton) {
    modifiers |= blink::WebInputEvent::RightButtonDown;
  }

  modifiers |= QInputEventStateToWebEventModifiers(qevent);

  return modifiers;
}

void ReleaseKeyEvent(void* event) {
  delete reinterpret_cast<QKeyEvent*>(event);
}

void* CopyKeyEvent(void* event) {
  return new QKeyEvent(*reinterpret_cast<QKeyEvent*>(event));
}

}

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(QKeyEvent* event,
                                                           bool is_char) {
  content::NativeWebKeyboardEvent result;

  QKeyEvent* os_event = new QKeyEvent(*event);
  os_event->setAccepted(false);
  result.SetExtraData(os_event, ReleaseKeyEvent, CopyKeyEvent);

  result.timeStampSeconds = QInputEventTimeToWebEventTime(event);
  result.modifiers = QInputEventStateToWebEventModifiers(event);

  if (event->isAutoRepeat()) {
    result.modifiers |= blink::WebInputEvent::IsAutoRepeat;
  }

  switch (event->type()) {
    case QEvent::KeyPress: {
      result.type = is_char ?
          blink::WebInputEvent::Char : blink::WebInputEvent::RawKeyDown;
      break;
    }
    case QEvent::KeyRelease:
      result.type = blink::WebInputEvent::KeyUp;
      break;
    default:
      NOTREACHED();
  }

  if (result.modifiers & blink::WebInputEvent::AltKey) {
    result.isSystemKey = true;
  }

  int windowsKeyCode = GetKeyboardCodeFromQKeyEvent(event);
  result.windowsKeyCode = oxide::WindowsKeyCodeWithoutLocation(windowsKeyCode);
  result.modifiers |=
      oxide::LocationModifiersFromWindowsKeyCode(windowsKeyCode);
  result.nativeKeyCode = event->nativeVirtualKey();

  const unsigned short* text = event->text().utf16();
  memcpy(result.unmodifiedText, text, qMin(sizeof(result.unmodifiedText), sizeof(*text)));

  static_assert(sizeof(result.unmodifiedText) == sizeof(result.text),
                "blink::WebKeyboardEvent::text and "
                "blink::WebKeyboardEvent::unmodifiedText sizes don't match");

  if (result.modifiers & blink::WebInputEvent::ControlKey) {
    result.text[0] = GetControlCharacter(
        result.windowsKeyCode,
        result.modifiers & blink::WebInputEvent::ShiftKey);
  } else {
    memcpy(result.text, result.unmodifiedText, sizeof(result.unmodifiedText));
  }

  return result;
}

blink::WebMouseEvent MakeWebMouseEvent(QMouseEvent* event,
                                       QScreen* screen,
                                       float location_bar_content_offset) {
  blink::WebMouseEvent result;

  result.timeStampSeconds = QInputEventTimeToWebEventTime(event);
  result.modifiers = QMouseEventStateToWebEventModifiers(event);

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.x = pos.x();
  result.y = std::floor(pos.y() - location_bar_content_offset);

  gfx::PointF window_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->windowPos()),
                                          screen);
  result.windowX = std::floor(window_pos.x());
  result.windowY = std::floor(window_pos.y());

  gfx::Point global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->globalPos()),
                                          screen);
  result.globalX = global_pos.x();
  result.globalY = global_pos.y();

  result.clickCount = 0;

  switch (event->type()) {
    case QEvent::MouseButtonPress:
      result.type = blink::WebInputEvent::MouseDown;
      result.clickCount = 1;
      break;
    case QEvent::MouseButtonRelease:
      result.type = blink::WebInputEvent::MouseUp;
      break;
    case QEvent::MouseMove:
      result.type = blink::WebInputEvent::MouseMove;
      break;
    default:
      NOTREACHED();
  }

  if (event->type() != QEvent::MouseMove) {
    switch(event->button()) {
      case Qt::LeftButton:
        result.button = blink::WebMouseEvent::ButtonLeft;
        break;
      case Qt::MidButton:
        result.button = blink::WebMouseEvent::ButtonMiddle;
        break;
      case Qt::RightButton:
        result.button = blink::WebMouseEvent::ButtonRight;
        break;
      default:
        NOTREACHED();
    }
  } else {
    if (event->buttons() & Qt::LeftButton) {
      result.button = blink::WebMouseEvent::ButtonLeft;
    }
    if (event->buttons() & Qt::MidButton) {
      result.button = blink::WebMouseEvent::ButtonMiddle;
    }
    if (event->buttons() & Qt::RightButton) {
      result.button = blink::WebMouseEvent::ButtonRight;
    }
  }

  return result;
}

blink::WebMouseWheelEvent MakeWebMouseWheelEvent(QWheelEvent* event,
                                                 const QPointF& window_pos,
                                                 QScreen* screen,
                                                 float location_bar_content_offset) {
  blink::WebMouseWheelEvent result;

  result.timeStampSeconds = QInputEventTimeToWebEventTime(event);

  // In Chromium a wheel event is a type of mouse event, but this is not the
  // case in Qt (QWheelEvent is not derived from QMouseEvent). We create a
  // temporary QMouseEvent here so that we can use the same code for calculating
  // modifiers as we use for other mouse events
  QMouseEvent dummy(QEvent::MouseMove,
                    QPointF(0, 0),
                    Qt::NoButton,
                    event->buttons(),
                    event->modifiers());
  result.modifiers = QMouseEventStateToWebEventModifiers(&dummy);

  result.type = blink::WebInputEvent::MouseWheel;
  result.button = blink::WebMouseEvent::ButtonNone;

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.x = pos.x();
  result.y = std::floor(pos.y() - location_bar_content_offset);

  gfx::PointF converted_window_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(window_pos), screen);
  result.windowX = std::floor(converted_window_pos.x());
  result.windowY = std::floor(converted_window_pos.y());

  gfx::Point global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->globalPos()),
                                          screen);
  result.globalX = global_pos.x();
  result.globalY = global_pos.y();

  QPoint delta = event->angleDelta();
  QPoint pixels = event->pixelDelta();

  static const int kPixelsPerTick = 53;

  if (!pixels.isNull()) {
    gfx::Point p = DpiUtils::ConvertQtPixelsToChromium(ToChromium(pixels),
                                                       screen);
    result.deltaX = p.x();
    result.deltaY = p.y();
    result.wheelTicksX = result.deltaX / kPixelsPerTick;
    result.wheelTicksY = result.deltaY / kPixelsPerTick;
  } else {
    // angelDelta unit is 0.125degrees
    // Assuming 1 tick == 15degrees, then 1 tick == (120*0.125)degrees
    result.wheelTicksX = delta.x() / 120.0f;
    result.wheelTicksY = delta.y() / 120.0f;
    result.deltaX = result.wheelTicksX * kPixelsPerTick;
    result.deltaY = result.wheelTicksY * kPixelsPerTick;
  }

  return result;
}

blink::WebMouseEvent MakeWebMouseEvent(
    QHoverEvent* event,
    const QPointF& window_pos,
    const QPoint& global_pos,
    QScreen* screen,
    float location_bar_content_offset) {
  blink::WebMouseEvent result;

  result.timeStampSeconds = QInputEventTimeToWebEventTime(event);
  result.modifiers = QInputEventStateToWebEventModifiers(event);

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.x = pos.x();
  result.y = std::floor(pos.y() - location_bar_content_offset);

  gfx::PointF converted_window_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(window_pos), screen);
  result.windowX = std::floor(converted_window_pos.x());
  result.windowY = std::floor(converted_window_pos.y());

  gfx::Point converted_global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(global_pos), screen);
  result.globalX = converted_global_pos.x();
  result.globalY = converted_global_pos.y();

  result.clickCount = 0;

  switch (event->type()) {
    case QEvent::HoverEnter:
      result.type = blink::WebInputEvent::MouseEnter;
      break;
    case QEvent::HoverLeave:
      result.type = blink::WebInputEvent::MouseLeave;
      break;
    case QEvent::HoverMove:
      result.type = blink::WebInputEvent::MouseMove;
      break;
    default:
      NOTREACHED();
  }

  result.button = blink::WebMouseEvent::ButtonNone;

  return result;
}

} // namespace qt
} // namespace oxide
