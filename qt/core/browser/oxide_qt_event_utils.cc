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
#include "third_party/WebKit/public/platform/WebPointerProperties.h"
#include "ui/events/keycodes/dom/keycode_converter.h"
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
    modifiers |= blink::WebInputEvent::kShiftKey;
  }
  if (qmodifiers & Qt::ControlModifier) {
    modifiers |= blink::WebInputEvent::kControlKey;
  }
  if (qmodifiers & Qt::AltModifier) {
    modifiers |= blink::WebInputEvent::kAltKey;
  }
  if (qmodifiers & Qt::MetaModifier) {
    modifiers |= blink::WebInputEvent::kMetaKey;
  }
  if (qmodifiers & Qt::KeypadModifier) {
    modifiers |= blink::WebInputEvent::kIsKeyPad;
  }

  return modifiers;
}

int QMouseEventStateToWebEventModifiers(QMouseEvent* qevent) {
  Qt::MouseButtons buttons = qevent->buttons();

  int modifiers = 0;

  if (buttons & Qt::LeftButton) {
    modifiers |= blink::WebInputEvent::kLeftButtonDown;
  }
  if (buttons & Qt::MidButton) {
    modifiers |= blink::WebInputEvent::kMiddleButtonDown;
  }
  if (buttons & Qt::RightButton) {
    modifiers |= blink::WebInputEvent::kRightButtonDown;
  }

  modifiers |= QInputEventStateToWebEventModifiers(qevent);

  return modifiers;
}

blink::WebInputEvent::Type QInputEventTypeToWebEventType(QInputEvent* event,
                                                         bool is_char = false) {
  switch (event->type()) {
    case QEvent::KeyPress:
      return is_char ?
          blink::WebInputEvent::kChar : blink::WebInputEvent::kRawKeyDown;
    case QEvent::KeyRelease:
      return blink::WebInputEvent::kKeyUp;
    default:
      NOTREACHED();
      return blink::WebInputEvent::kUndefined;
  }
}

class QKeyEventWrapper
    : public content::NativeWebKeyboardEvent::ExtraData {
 public:
  QKeyEventWrapper(QKeyEvent event)
      : event_(std::move(event)) {}

  QKeyEvent& event() { return event_; }

 private:
  ~QKeyEventWrapper() = default;

  QKeyEvent event_;
};

}

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(QKeyEvent* event,
                                                           bool is_char) {
  content::NativeWebKeyboardEvent result(
      QInputEventTypeToWebEventType(event, is_char),
      QInputEventStateToWebEventModifiers(event),
      QInputEventTimeToWebEventTime(event));

  scoped_refptr<QKeyEventWrapper> wrapper =
      new QKeyEventWrapper(QKeyEvent(*event));
  wrapper->event().setAccepted(false);
  result.extra_data = wrapper;

  if (event->isAutoRepeat()) {
    result.SetModifiers(
        result.GetModifiers() | blink::WebInputEvent::kIsAutoRepeat);
  }

  if (result.GetModifiers() & blink::WebInputEvent::kAltKey) {
    result.is_system_key = true;
  }

  int windows_key_code = GetKeyboardCodeFromQKeyEvent(event);
  result.windows_key_code = oxide::WindowsKeyCodeWithoutLocation(windows_key_code);
  result.SetModifiers(
      result.GetModifiers()
      | oxide::LocationModifiersFromWindowsKeyCode(windows_key_code));
  result.native_key_code = event->nativeVirtualKey();

  result.dom_code = static_cast<int>(
      ui::KeycodeConverter::NativeKeycodeToDomCode(event->nativeScanCode()));
  result.dom_key = GetDomKeyFromQKeyEvent(event);

  const unsigned short* text = event->text().utf16();
  memcpy(result.unmodified_text, text, qMin(sizeof(result.unmodified_text), sizeof(*text)));

  static_assert(sizeof(result.unmodified_text) == sizeof(result.text),
                "blink::WebKeyboardEvent::text and "
                "blink::WebKeyboardEvent::unmodified_text sizes don't match");

  if (result.GetModifiers() & blink::WebInputEvent::kControlKey) {
    result.text[0] = GetControlCharacter(
        result.windows_key_code,
        result.GetModifiers() & blink::WebInputEvent::kShiftKey);
  } else {
    memcpy(result.text, result.unmodified_text, sizeof(result.unmodified_text));
  }

  return result;
}

blink::WebMouseEvent MakeWebMouseEvent(QMouseEvent* event,
                                       QScreen* screen) {
  blink::WebMouseEvent result;

  result.SetTimeStampSeconds(QInputEventTimeToWebEventTime(event));
  result.SetModifiers(QMouseEventStateToWebEventModifiers(event));
  result.pointer_type = blink::WebPointerProperties::PointerType::kMouse;

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.SetPositionInWidget(pos.x(), pos.y());

  gfx::Point global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->globalPos()),
                                          screen);
  result.SetPositionInScreen(global_pos.x(), global_pos.y());

  result.click_count = 0;

  switch (event->type()) {
    case QEvent::MouseButtonPress:
      result.SetType(blink::WebInputEvent::kMouseDown);
      result.click_count = 1;
      break;
    case QEvent::MouseButtonRelease:
      result.SetType(blink::WebInputEvent::kMouseUp);
      break;
    case QEvent::MouseMove:
      result.SetType(blink::WebInputEvent::kMouseMove);
      break;
    default:
      NOTREACHED();
  }

  if (event->type() != QEvent::MouseMove) {
    switch(event->button()) {
      case Qt::LeftButton:
        result.button = blink::WebPointerProperties::Button::kLeft;
        break;
      case Qt::MidButton:
        result.button = blink::WebPointerProperties::Button::kMiddle;
        break;
      case Qt::RightButton:
        result.button = blink::WebPointerProperties::Button::kRight;
        break;
      default:
        NOTREACHED();
    }
  } else {
    if (event->buttons() & Qt::LeftButton) {
      result.button = blink::WebPointerProperties::Button::kLeft;
    }
    if (event->buttons() & Qt::MidButton) {
      result.button = blink::WebPointerProperties::Button::kMiddle;
    }
    if (event->buttons() & Qt::RightButton) {
      result.button = blink::WebPointerProperties::Button::kRight;
    }
  }

  return result;
}

blink::WebMouseWheelEvent MakeWebMouseWheelEvent(QWheelEvent* event,
                                                 QScreen* screen) {
  blink::WebMouseWheelEvent result;

  // The timestamp has be referenced to TimeTicks as ui::InputHandlerProxy uses
  // this on the renderer side to compute a roundtrip delay.
  result.SetTimeStampSeconds(
      (base::TimeTicks::Now() - base::TimeTicks()).InSecondsF());

  result.pointer_type = blink::WebPointerProperties::PointerType::kMouse;

  // In Chromium a wheel event is a type of mouse event, but this is not the
  // case in Qt (QWheelEvent is not derived from QMouseEvent). We create a
  // temporary QMouseEvent here so that we can use the same code for calculating
  // modifiers as we use for other mouse events
  QMouseEvent dummy(QEvent::MouseMove,
                    QPointF(0, 0),
                    Qt::NoButton,
                    event->buttons(),
                    event->modifiers());
  result.SetModifiers(QMouseEventStateToWebEventModifiers(&dummy));

  result.SetType(blink::WebInputEvent::kMouseWheel);
  result.button = blink::WebPointerProperties::Button::kNoButton;

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.SetPositionInWidget(pos.x(), pos.y());

  gfx::Point global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->globalPos()),
                                          screen);
  result.SetPositionInScreen(global_pos.x(), global_pos.y());

  QPoint delta = event->angleDelta();
  QPoint pixels = event->pixelDelta();

  static const int kPixelsPerTick = 53;

  if (!pixels.isNull()) {
    gfx::Point p = DpiUtils::ConvertQtPixelsToChromium(ToChromium(pixels),
                                                       screen);
    result.delta_x = p.x();
    result.delta_y = p.y();
    result.wheel_ticks_x = result.delta_x / kPixelsPerTick;
    result.wheel_ticks_y = result.delta_y / kPixelsPerTick;
  } else {
    // angelDelta unit is 0.125degrees
    // Assuming 1 tick == 15degrees, then 1 tick == (120*0.125)degrees
    result.wheel_ticks_x = delta.x() / 120.0f;
    result.wheel_ticks_y = delta.y() / 120.0f;
    result.delta_x = result.wheel_ticks_x * kPixelsPerTick;
    result.delta_y = result.wheel_ticks_y * kPixelsPerTick;
  }

  return result;
}

blink::WebMouseEvent MakeWebMouseEvent(
    QHoverEvent* event,
    const QPoint& global_pos,
    QScreen* screen) {
  blink::WebMouseEvent result;

  result.SetTimeStampSeconds(QInputEventTimeToWebEventTime(event));
  result.SetModifiers(QInputEventStateToWebEventModifiers(event));
  result.pointer_type = blink::WebPointerProperties::PointerType::kMouse;

  gfx::Point pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(event->pos()), screen);
  result.SetPositionInWidget(pos.x(), pos.y());

  gfx::Point converted_global_pos =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(global_pos), screen);
  result.SetPositionInScreen(converted_global_pos.x(),
                             converted_global_pos.y());

  result.click_count = 0;

  switch (event->type()) {
    case QEvent::HoverEnter:
      result.SetType(blink::WebInputEvent::kMouseEnter);
      break;
    case QEvent::HoverLeave:
      result.SetType(blink::WebInputEvent::kMouseLeave);
      break;
    case QEvent::HoverMove:
      result.SetType(blink::WebInputEvent::kMouseMove);
      break;
    default:
      NOTREACHED();
  }

  result.button = blink::WebPointerProperties::Button::kNoButton;

  return result;
}

QKeyEvent* NativeWebKeyboardEventToQKeyEvent(
    const content::NativeWebKeyboardEvent& event) {
  QKeyEventWrapper* wrapper =
      static_cast<QKeyEventWrapper*>(event.extra_data.get());
  if (!wrapper) {
    return nullptr;
  }

  return &wrapper->event();
}

} // namespace qt
} // namespace oxide
