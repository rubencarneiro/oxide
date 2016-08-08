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

int QKeyEventKeyCodeToWebEventKeyCode(QKeyEvent* qevent) {
  int qkeycode = qevent->key();

  // 1) Keypad
  if (qevent->modifiers() & Qt::KeypadModifier) {
    if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
      return (qkeycode - Qt::Key_0) + ui::VKEY_NUMPAD0;
    }

    switch (qkeycode) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      return ui::VKEY_RETURN;
    case Qt::Key_PageUp:
      return ui::VKEY_PRIOR;
    case Qt::Key_PageDown:
      return ui::VKEY_NEXT;
    case Qt::Key_End:
      return ui::VKEY_END;
    case Qt::Key_Home:
      return ui::VKEY_HOME;
    case Qt::Key_Left:
      return ui::VKEY_LEFT;
    case Qt::Key_Up:
      return ui::VKEY_UP;
    case Qt::Key_Right:
      return ui::VKEY_RIGHT;
    case Qt::Key_Down:
      return ui::VKEY_DOWN;
    case Qt::Key_Asterisk:
      return ui::VKEY_MULTIPLY;
    case Qt::Key_Plus:
      return ui::VKEY_ADD;
    case Qt::Key_Minus:
      return ui::VKEY_SUBTRACT;
    case Qt::Key_Period:
      return ui::VKEY_DECIMAL;
    case Qt::Key_Slash:
      return ui::VKEY_DIVIDE;
    case Qt::Key_Insert:
      return ui::VKEY_INSERT;
    case Qt::Key_Delete:
      return ui::VKEY_DELETE;
    default:
      return 0;
    }
  }

  // 2) VKEY_A - VKEY_Z
  if (qkeycode >= Qt::Key_A && qkeycode <= Qt::Key_Z) {
    return (qkeycode - Qt::Key_A) + ui::VKEY_A;
  }

  // 3) VKEY_0 - VKEY_9
  if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
    return (qkeycode - Qt::Key_0) + ui::VKEY_0;
  }
  switch (qkeycode) {
  case Qt::Key_ParenRight:
    return ui::VKEY_0;
  case Qt::Key_Exclam:
    return ui::VKEY_1;
  case Qt::Key_At:
    return ui::VKEY_2;
  case Qt::Key_NumberSign:
    return ui::VKEY_3;
  case Qt::Key_Dollar:
    return ui::VKEY_4;
  case Qt::Key_Percent:
    return ui::VKEY_5;
  case Qt::Key_AsciiCircum:
    return ui::VKEY_6;
  case Qt::Key_Ampersand:
    return ui::VKEY_7;
  case Qt::Key_Asterisk:
    return ui::VKEY_8;
  case Qt::Key_ParenLeft:
    return ui::VKEY_9;
  default:
    break;
  }

  // 4) VKEY_F1 - VKEY_F24
  if (qkeycode >= Qt::Key_F1 && qkeycode <= Qt::Key_F24) {
    // We miss Qt::Key_F25 - Qt::Key_F35
    return (qkeycode - Qt::Key_F1) + ui::VKEY_F1;
  }

  switch (qkeycode) {
  case Qt::Key_Backspace:
    return ui::VKEY_BACK;
  case Qt::Key_Tab:
  case Qt::Key_Backtab:
    return ui::VKEY_TAB;
  // VKEY_BACKTAB - not used in Chromium X11
  case Qt::Key_Clear:
    return ui::VKEY_CLEAR;
  case Qt::Key_Return:
  case Qt::Key_Enter:
    return ui::VKEY_RETURN;
  case Qt::Key_Shift:
    return ui::VKEY_SHIFT;
  case Qt::Key_Control:
    return ui::VKEY_CONTROL;
  case Qt::Key_Meta:
    return ui::VKEY_MENU;
  case Qt::Key_Pause:
    return ui::VKEY_PAUSE;
  case Qt::Key_CapsLock:
    return ui::VKEY_CAPITAL;
  case Qt::Key_Kana_Lock:
    return ui::VKEY_KANA;
  case Qt::Key_Hangul:
    return ui::VKEY_HANGUL;
  // VKEY_JUNJA
  // VKEY_FINAL
  case Qt::Key_Hangul_Hanja:
    return ui::VKEY_HANJA;
  case Qt::Key_Kanji:
    return ui::VKEY_KANJI;
  case Qt::Key_Escape:
    return ui::VKEY_ESCAPE;
  case Qt::Key_Henkan:
    return ui::VKEY_CONVERT;
  case Qt::Key_Muhenkan:
    return ui::VKEY_NONCONVERT;
  // VKEY_ACCEPT
  // VKEY_MODECHANGE
  case Qt::Key_Space:
    return ui::VKEY_SPACE;
  case Qt::Key_PageUp:
    return ui::VKEY_PRIOR;
  case Qt::Key_PageDown:
    return ui::VKEY_NEXT;
  case Qt::Key_End:
    return ui::VKEY_END;
  case Qt::Key_Home:
    return ui::VKEY_HOME;
  case Qt::Key_Left:
    return ui::VKEY_LEFT;
  case Qt::Key_Up:
    return ui::VKEY_UP;
  case Qt::Key_Right:
    return ui::VKEY_RIGHT;
  case Qt::Key_Down:
    return ui::VKEY_DOWN;
  case Qt::Key_Select:
    return ui::VKEY_SELECT;
  case Qt::Key_Print:
    return ui::VKEY_PRINT;
  case Qt::Key_Execute:
    return ui::VKEY_EXECUTE;
  // VKEY_SNAPSHOT
  case Qt::Key_Insert:
    return ui::VKEY_INSERT;
  case Qt::Key_Delete:
    return ui::VKEY_DELETE;
  case Qt::Key_Help:
    return ui::VKEY_HELP;
  // VKEY_0 - VKEY_Z handled above
  case Qt::Key_Super_L:
    return ui::VKEY_LWIN;
  case Qt::Key_Super_R:
    return ui::VKEY_RWIN;
  case Qt::Key_Menu:
    return ui::VKEY_APPS;
  // VKEY_SLEEP
  // VKEY_NUMPAD0 - VKEY_NUMPAD9 handled in keypad section
  case Qt::Key_multiply:
    return ui::VKEY_MULTIPLY;
  // VKEY_ADD handled in keypad section
  // VKEY_SEPARATOR
  // VKEY_SUBTRACT handled in keypad section
  // VKEY_DECIMAL handled in keypad section
  // VKEY_DIVIDE handled in keypad section
  // VKEY_F1 - VKEY_F24 handled above
  case Qt::Key_NumLock:
    return ui::VKEY_NUMLOCK;
  case Qt::Key_ScrollLock:
    return ui::VKEY_SCROLL;
  // VKEY_LSHIFT
  // VKEY_RSHIFT
  // VKEY_LCONTROL
  // VKEY_RCONTROL
  case Qt::Key_Alt:
    return ui::VKEY_LMENU;
  // VKEY_RMENU
  case Qt::Key_Back:
    return ui::VKEY_BROWSER_BACK;
  case Qt::Key_Forward:
    return ui::VKEY_BROWSER_FORWARD;
  case Qt::Key_Refresh:
    return ui::VKEY_BROWSER_REFRESH;
  case Qt::Key_Stop:
    return ui::VKEY_BROWSER_STOP;
  case Qt::Key_Search:
    return ui::VKEY_BROWSER_SEARCH;
  case Qt::Key_Favorites:
    return ui::VKEY_BROWSER_FAVORITES;
  case Qt::Key_HomePage:
    return ui::VKEY_BROWSER_HOME;
  case Qt::Key_VolumeMute:
    return ui::VKEY_VOLUME_MUTE;
  case Qt::Key_VolumeDown:
    return ui::VKEY_VOLUME_DOWN;
  case Qt::Key_VolumeUp:
    return ui::VKEY_VOLUME_UP;
  case Qt::Key_MediaNext:
    return ui::VKEY_MEDIA_NEXT_TRACK;
  case Qt::Key_MediaPrevious:
    return ui::VKEY_MEDIA_PREV_TRACK;
  case Qt::Key_MediaStop:
    return ui::VKEY_MEDIA_STOP;
  case Qt::Key_MediaPlay:
  case Qt::Key_MediaPause:
  case Qt::Key_MediaTogglePlayPause:
    return ui::VKEY_MEDIA_PLAY_PAUSE;
  case Qt::Key_LaunchMail:
    return ui::VKEY_MEDIA_LAUNCH_MAIL;
  case Qt::Key_LaunchMedia:
    return ui::VKEY_MEDIA_LAUNCH_MEDIA_SELECT;
  case Qt::Key_Launch0:
    return ui::VKEY_MEDIA_LAUNCH_APP1;
  case Qt::Key_Launch1:
    return ui::VKEY_MEDIA_LAUNCH_APP2;
  case Qt::Key_Colon:
  case Qt::Key_Semicolon:
    return ui::VKEY_OEM_1;
  case Qt::Key_Plus:
  case Qt::Key_Equal:
    return ui::VKEY_OEM_PLUS;
  case Qt::Key_Comma:
  case Qt::Key_Less:
    return ui::VKEY_OEM_COMMA;
  case Qt::Key_Minus:
  case Qt::Key_Underscore:
    return ui::VKEY_OEM_MINUS;
  case Qt::Key_Period:
  case Qt::Key_Greater:
    return ui::VKEY_OEM_PERIOD;
  case Qt::Key_Slash:
  case Qt::Key_Question:
    return ui::VKEY_OEM_2;
  case Qt::Key_QuoteLeft:
  case Qt::Key_AsciiTilde:
    return ui::VKEY_OEM_3;
  case Qt::Key_BracketLeft:
  case Qt::Key_BraceLeft:
    return ui::VKEY_OEM_4;
  case Qt::Key_Backslash:
  case Qt::Key_Bar:
    return ui::VKEY_OEM_5;
  case Qt::Key_BracketRight:
  case Qt::Key_BraceRight:
    return ui::VKEY_OEM_6;
  case Qt::Key_QuoteDbl:
  case Qt::Key_Apostrophe:
    return ui::VKEY_OEM_7;
  // VKEY_OEM_8
  case Qt::Key_Ugrave:
  case Qt::Key_brokenbar:
    return ui::VKEY_OEM_102;
  // VKEY_OEM_103
  // VKEY_OEM_104
  // VKEY_PROCESSKEY
  // VKEY_PACKET
  // VKEY_DBE_SBCSCHAR
  case Qt::Key_Zenkaku_Hankaku:
    return ui::VKEY_DBE_DBCSCHAR;
  // VKEY_ATTN
  // VKEY_CRSEL
  // VKEY_EXSEL
  // VKEY_EREOF
  // VKEY_PLAY
  // VKEY_ZOOM
  // VKEY_NONAME
  // VKEY_PA1
  // VKEY_OEM_CLEAR
  case Qt::Key_WLAN:
    return ui::VKEY_WLAN;
  case Qt::Key_PowerOff:
    return ui::VKEY_POWER;
  case Qt::Key_MonBrightnessDown:
    return ui::VKEY_BRIGHTNESS_DOWN;
  case Qt::Key_MonBrightnessUp:
    return ui::VKEY_BRIGHTNESS_UP;
  case Qt::Key_KeyboardBrightnessDown:
    return ui::VKEY_KBD_BRIGHTNESS_DOWN;
  case Qt::Key_KeyboardBrightnessUp:
    return ui::VKEY_KBD_BRIGHTNESS_UP;
  case Qt::Key_AltGr:
    return ui::VKEY_ALTGR;
  default:
    break;
  }

  return 0;
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

  int windowsKeyCode = QKeyEventKeyCodeToWebEventKeyCode(event);
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
