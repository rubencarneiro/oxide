// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include "oxide_qt_render_widget_host_view.h"

#include <QByteArray>
#include <QChar>
#include <QCursor>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QImage>
#include <QInputEvent>
#include <QInputMethod>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QTextCharFormat>
#include <QTouchEvent>
#include <QWheelEvent>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/common/cursors/webcursor.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_widget_host.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/event.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/rect.h"

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"
#include "shared/browser/oxide_form_factor.h"

#include "oxide_qt_web_view.h"

QT_USE_NAMESPACE

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
  return static_cast<double>(qevent->timestamp() / 1000.0);
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
  bool mouse_down = qevent->type() == QEvent::MouseButtonPress;
  Qt::MouseButton event_button = qevent->button();

  int modifiers = 0;

  if (buttons & Qt::LeftButton &&
      (!mouse_down || event_button != Qt::LeftButton)) {
    modifiers |= blink::WebInputEvent::LeftButtonDown;
  }
  if (buttons & Qt::MidButton &&
      (!mouse_down || event_button != Qt::MidButton)) {
    modifiers |= blink::WebInputEvent::MiddleButtonDown;
  }
  if (buttons & Qt::RightButton &&
      (!mouse_down || event_button != Qt::RightButton)) {
    modifiers |= blink::WebInputEvent::RightButtonDown;
  }

  modifiers |= QInputEventStateToWebEventModifiers(qevent);

  return modifiers;
}

ui::EventType QTouchPointStateToEventType(Qt::TouchPointState state) {
  switch (state) {
    case Qt::TouchPointPressed:
      return ui::ET_TOUCH_PRESSED;
    case Qt::TouchPointMoved:
    case Qt::TouchPointStationary:
      return ui::ET_TOUCH_MOVED;
    case Qt::TouchPointReleased:
      return ui::ET_TOUCH_RELEASED;
    default:
      NOTREACHED();
      return ui::ET_UNKNOWN;
  }
}

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(
    QKeyEvent* qevent, bool is_char) {
  content::NativeWebKeyboardEvent event;

  QKeyEvent* os_event = new QKeyEvent(*qevent);
  os_event->setAccepted(false);
  event.os_event = reinterpret_cast<gfx::NativeEvent>(os_event);

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);
  event.modifiers = QInputEventStateToWebEventModifiers(qevent);

  if (qevent->isAutoRepeat()) {
    event.modifiers |= blink::WebInputEvent::IsAutoRepeat;
  }

  switch (qevent->type()) {
  case QEvent::KeyPress: {
    event.type = is_char ?
        blink::WebInputEvent::Char : blink::WebInputEvent::RawKeyDown;
    break;
  }
  case QEvent::KeyRelease:
    event.type = blink::WebInputEvent::KeyUp;
    break;
  default:
    NOTREACHED();
  }

  if (event.modifiers & blink::WebInputEvent::AltKey) {
    event.isSystemKey = true;
  }

  int windowsKeyCode = QKeyEventKeyCodeToWebEventKeyCode(qevent);
  event.windowsKeyCode =
      blink::WebKeyboardEvent::windowsKeyCodeWithoutLocation(windowsKeyCode);
  event.modifiers |=
      blink::WebKeyboardEvent::locationModifiersFromWindowsKeyCode(
        windowsKeyCode);
  event.nativeKeyCode = qevent->nativeVirtualKey();
  event.setKeyIdentifierFromWindowsKeyCode();

  const unsigned short* text = qevent->text().utf16();
  memcpy(event.unmodifiedText, text, qMin(sizeof(event.unmodifiedText), sizeof(*text)));

  COMPILE_ASSERT(sizeof(event.unmodifiedText) == sizeof(event.text),
                 text_member_sizes_dont_match);

  if (event.modifiers & blink::WebInputEvent::ControlKey) {
    event.text[0] = GetControlCharacter(
        event.windowsKeyCode,
        event.modifiers & blink::WebInputEvent::ShiftKey);
  } else {
    memcpy(event.text, event.unmodifiedText, sizeof(event.unmodifiedText));
  }

  return event;
}

blink::WebMouseEvent MakeWebMouseEvent(QMouseEvent* qevent, float scale) {
  blink::WebMouseEvent event;

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);
  event.modifiers = QMouseEventStateToWebEventModifiers(qevent);

  event.x = qRound(qevent->x() / scale);
  event.y = qRound(qevent->y() / scale);

  event.windowX = event.x;
  event.windowY = event.y;

  event.globalX = qRound(qevent->globalX() / scale);
  event.globalY = qRound(qevent->globalY() / scale);

  event.clickCount = 0;

  switch (qevent->type()) {
  case QEvent::MouseButtonPress:
    event.type = blink::WebInputEvent::MouseDown;
    event.clickCount = 1;
    break;
  case QEvent::MouseButtonRelease:
    event.type = blink::WebInputEvent::MouseUp;
    break;
  case QEvent::MouseMove:
    event.type = blink::WebInputEvent::MouseMove;
    break;
  case QEvent::MouseButtonDblClick:
    event.type = blink::WebInputEvent::MouseDown;
    event.clickCount = 2;
    break;
  default:
    NOTREACHED();
  }

  switch(qevent->button()) {
  case Qt::LeftButton:
    event.button = blink::WebMouseEvent::ButtonLeft;
    break;
  case Qt::MidButton:
    event.button = blink::WebMouseEvent::ButtonMiddle;
    break;
  case Qt::RightButton:
    event.button = blink::WebMouseEvent::ButtonRight;
    break;
  default:
    event.button = blink::WebMouseEvent::ButtonNone;
    DCHECK_EQ(event.type, blink::WebMouseEvent::MouseMove);
  }

  return event;
}

blink::WebMouseWheelEvent MakeWebMouseWheelEvent(QWheelEvent* qevent, float scale) {
  blink::WebMouseWheelEvent event;

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);

  // In Chromium a wheel event is a type of mouse event, but this is not the
  // case in Qt (QWheelEvent is not derived from QMouseEvent). We create a
  // temporary QMouseEvent here so that we can use the same code for calculating
  // modifiers as we use for other mouse events
  QMouseEvent dummy(QEvent::MouseMove,
                    QPointF(0, 0),
                    Qt::NoButton,
                    qevent->buttons(),
                    qevent->modifiers());
  event.modifiers = QMouseEventStateToWebEventModifiers(&dummy);

  event.type = blink::WebInputEvent::MouseWheel;
  event.button = blink::WebMouseEvent::ButtonNone;

  event.x = qRound(qevent->x() / scale);
  event.y = qRound(qevent->y() / scale);

  event.windowX = event.x;
  event.windowY = event.y;

  event.globalX = qRound(qevent->globalX() / scale);
  event.globalY = qRound(qevent->globalY() / scale);

  // See comment in third_party/WebKit/Source/web/gtk/WebInputEventFactory.cpp
  static const float scrollbarPixelsPerTick = 160.0f / 3.0f;

  // angelDelta unit is 0.125degrees
  // 1 tick = 15degrees = (120*0.125)degrees
  QPoint delta(qevent->angleDelta());
  event.wheelTicksX = delta.x() / 120.0f;
  event.wheelTicksY = delta.y() / 120.0f;
  event.deltaX = event.wheelTicksX * scrollbarPixelsPerTick;
  event.deltaY = event.wheelTicksY * scrollbarPixelsPerTick;

  return event;
}

Qt::InputMethodHints QImHintsFromInputType(ui::TextInputType type) {
  switch (type) {
    case ui::TEXT_INPUT_TYPE_TEXT:
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
      return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
      return Qt::ImhHiddenText | Qt::ImhSensitiveData |
          Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase |
          Qt::ImhNoPredictiveText;
    case ui::TEXT_INPUT_TYPE_SEARCH:
      return Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_EMAIL:
      return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
      return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
      return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
      return Qt::ImhUrlCharactersOnly;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
      return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
      return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TIME:
      return Qt::ImhTime;
    default:
      return Qt::ImhNone;
  }
}

inline QCursor webcursor_to_qt_cursor(blink::WebCursorInfo::Type type) {
  Qt::CursorShape cs = Qt::ArrowCursor;
  switch (type) {
  case blink::WebCursorInfo::TypeCross:
    cs = Qt::CrossCursor;
    break;

  case blink::WebCursorInfo::TypeHand:
    cs = Qt::PointingHandCursor;
    break;

  case blink::WebCursorInfo::TypeCell:
  case blink::WebCursorInfo::TypeIBeam:
    cs = Qt::IBeamCursor;
    break;

  case blink::WebCursorInfo::TypeWait:
    cs = Qt::WaitCursor;
    break;

  case blink::WebCursorInfo::TypeHelp:
    cs = Qt::WhatsThisCursor;
    break;

  case blink::WebCursorInfo::TypeEastResize:
  case blink::WebCursorInfo::TypeWestResize:
  case blink::WebCursorInfo::TypeEastWestResize:
    cs = Qt::SizeHorCursor;
    break;

  case blink::WebCursorInfo::TypeNorthResize:
  case blink::WebCursorInfo::TypeSouthResize:
  case blink::WebCursorInfo::TypeNorthSouthResize:
    cs = Qt::SizeVerCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastResize:
  case blink::WebCursorInfo::TypeSouthWestResize:
    cs = Qt::SizeBDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthWestResize:
  case blink::WebCursorInfo::TypeSouthEastResize:
    cs = Qt::SizeFDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastSouthWestResize:
  case blink::WebCursorInfo::TypeNorthWestSouthEastResize:
  case blink::WebCursorInfo::TypeMove:
    cs = Qt::SizeAllCursor;
    break;

  case blink::WebCursorInfo::TypeColumnResize:
    cs = Qt::SplitHCursor;
    break;

  case blink::WebCursorInfo::TypeRowResize:
    cs = Qt::SplitVCursor;
    break;

  case blink::WebCursorInfo::TypeMiddlePanning:
  case blink::WebCursorInfo::TypeEastPanning:
  case blink::WebCursorInfo::TypeNorthPanning:
  case blink::WebCursorInfo::TypeNorthEastPanning:
  case blink::WebCursorInfo::TypeNorthWestPanning:
  case blink::WebCursorInfo::TypeSouthPanning:
  case blink::WebCursorInfo::TypeSouthEastPanning:
  case blink::WebCursorInfo::TypeSouthWestPanning:
  case blink::WebCursorInfo::TypeWestPanning:
  case blink::WebCursorInfo::TypeGrab:
  case blink::WebCursorInfo::TypeGrabbing:
    cs = Qt::ClosedHandCursor;
    break;

  case blink::WebCursorInfo::TypeProgress:
    cs = Qt::BusyCursor;
    break;

  case blink::WebCursorInfo::TypeNoDrop:
  case blink::WebCursorInfo::TypeNotAllowed:
    cs = Qt::ForbiddenCursor;
    break;

  case blink::WebCursorInfo::TypeCopy:
  case blink::WebCursorInfo::TypeContextMenu:
  case blink::WebCursorInfo::TypeVerticalText:
  case blink::WebCursorInfo::TypeAlias:
  case blink::WebCursorInfo::TypeZoomIn:
  case blink::WebCursorInfo::TypeZoomOut:
  case blink::WebCursorInfo::TypeCustom:
  case blink::WebCursorInfo::TypePointer:
  case blink::WebCursorInfo::TypeNone:
  default:
    break;
  }
  return QCursor(cs);
}

}

// static
float RenderWidgetHostView::GetDeviceScaleFactorFromQScreen(QScreen* screen) {
  // For some reason, the Ubuntu QPA plugin doesn't override
  // QScreen::devicePixelRatio. However, applications using the Ubuntu
  // SDK use something called "grid units". The relationship between
  // grid units and device pixels is set by the "GRID_UNIT_PX" environment
  // variable. On a screen with a DPR of 1.0f, GRID_UNIT_PX is set to 8, and
  // 1 grid unit == 8 device pixels.
  // If we are using the Ubuntu backend, we use GRID_UNIT_PX to derive the
  // device pixel ratio, else we get it from QScreen::devicePixelRatio.
  // XXX: There are 2 scenarios where this is completely broken:
  //      1) Any apps not using the Ubuntu SDK but running with the Ubuntu
  //         QPA plugin. In this case, we derive a DPR from GRID_UNIT_PX if
  //         set, and the application probably uses QScreen::devicePixelRatio,
  //         which is always 1.0f
  //      2) Any apps using the Ubuntu SDK but not running with the Ubuntu
  //         QPA plugin. In this case, we get the DPR from
  //         QScreen::devicePixelRatio, and the application uses GRID_UNIX_PX
  //         if set
  //      I think it would be better if the Ubuntu QPA plugin did override
  //      QScreen::devicePixelRatio (it could still get that from GRID_UNIT_PX),
  //      and the Ubuntu SDK used this to convert between grid units and device
  //      pixels, then we could just use QScreen::devicePixelRatio here

  // Allow an override for testing
  {
    QByteArray force_dpr(qgetenv("OXIDE_FORCE_DPR"));
    bool ok;
    float scale = force_dpr.toFloat(&ok);
    if (ok) {
      return scale;
    }
  }

  QString platform = QGuiApplication::platformName();
  if (platform == QLatin1String("ubuntu") ||
      platform == QLatin1String("ubuntumirclient")) {
    QByteArray grid_unit_px(qgetenv("GRID_UNIT_PX"));
    bool ok;
    float scale = grid_unit_px.toFloat(&ok);
    if (ok) {
      return scale / 8;
    }
  }

  return float(screen->devicePixelRatio());
}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {
  if (!HasFocus()) {
    return;
  }
  delegate_->SetInputMethodEnabled(is_editable_node);
  if (QGuiApplication::inputMethod()->isVisible() != is_editable_node) {
    QGuiApplication::inputMethod()->setVisible(is_editable_node);
  }
}

void RenderWidgetHostView::Blur() {
  delegate_->Blur();
}

void RenderWidgetHostView::TextInputTypeChanged(ui::TextInputType type,
                                                ui::TextInputMode mode,
                                                bool can_compose_inline) {
  input_type_ = type;
  QGuiApplication::inputMethod()->update(Qt::ImQueryInput | Qt::ImHints);
  if (HasFocus() && (type != ui::TEXT_INPUT_TYPE_NONE) &&
      !QGuiApplication::inputMethod()->isVisible()) {
    delegate_->SetInputMethodEnabled(true);
    QGuiApplication::inputMethod()->show();
  }
}

void RenderWidgetHostView::ImeCancelComposition() {
  QGuiApplication::inputMethod()->reset();
}

void RenderWidgetHostView::GetScreenInfo(
    blink::WebScreenInfo* results) {
  QScreen* screen = delegate_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }
  GetWebScreenInfoFromQScreen(screen, results);
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow() {
  return GetViewBounds();
}

void RenderWidgetHostView::Focus() {
  delegate_->Focus();
}

bool RenderWidgetHostView::HasFocus() const {
  return delegate_->HasFocus();
}

void RenderWidgetHostView::Show() {
  delegate_->Show();
  WasShown();
}

void RenderWidgetHostView::Hide() {
  delegate_->Hide();
  WasHidden();
}

bool RenderWidgetHostView::IsShowing() {
  return delegate_->IsShowing();
}

void RenderWidgetHostView::SwapSoftwareFrame() {
  delegate_->SetCompositorFrameType(COMPOSITOR_FRAME_TYPE_SOFTWARE);
  delegate_->ScheduleUpdate();
}

void RenderWidgetHostView::SwapAcceleratedFrame() {
  delegate_->SetCompositorFrameType(COMPOSITOR_FRAME_TYPE_ACCELERATED);
  delegate_->ScheduleUpdate();
}

void RenderWidgetHostView::OnUpdateCursor(const content::WebCursor& cursor) {
  content::WebCursor::CursorInfo cursor_info;

  cursor.GetCursorInfo(&cursor_info);
  if (cursor.IsCustom()) {
    QImage::Format format = QImage::Format_Invalid;
    switch (cursor_info.custom_image.config()) {
    case SkBitmap::kRGB_565_Config: format = QImage::Format_RGB16;
    case SkBitmap::kARGB_4444_Config: format = QImage::Format_ARGB4444_Premultiplied;
    case SkBitmap::kARGB_8888_Config: format = QImage::Format_ARGB32_Premultiplied;
    default: ;
    }
    if (format == QImage::Format_Invalid) {
      return;
    }
    QImage cursor_image((uchar*)cursor_info.custom_image.getPixels(),
                        cursor_info.custom_image.width(),
                        cursor_info.custom_image.height(),
                        cursor_info.custom_image.rowBytes(),
                        format);

    QPixmap cursor_pixmap;
    if (cursor_pixmap.convertFromImage(cursor_image)) {
      delegate_->UpdateCursor(QCursor(cursor_pixmap));
    }
  } else {
    delegate_->UpdateCursor(webcursor_to_qt_cursor(cursor_info.type));
  }
}

RenderWidgetHostView::RenderWidgetHostView(
    content::RenderWidgetHost* render_widget_host,
    RenderWidgetHostViewDelegate* delegate) :
    oxide::RenderWidgetHostView(render_widget_host),
    delegate_(delegate),
    input_type_(ui::TEXT_INPUT_TYPE_NONE) {
  delegate->rwhv_ = this;
}

RenderWidgetHostView::~RenderWidgetHostView() {}

void RenderWidgetHostView::Init(oxide::WebView* view) {
  delegate_->Init(static_cast<WebView *>(view)->adapter());
}

// static
void RenderWidgetHostView::GetWebScreenInfoFromQScreen(
    QScreen* screen, blink::WebScreenInfo* result) {
  result->depth = screen->depth();
  result->depthPerComponent = 8; // XXX: Copied the GTK impl here
  result->isMonochrome = result->depth == 1;
  result->deviceScaleFactor = GetDeviceScaleFactorFromQScreen(screen);

  QRect rect = screen->geometry();
  result->rect = blink::WebRect(rect.x(),
                                rect.y(),
                                rect.width(),
                                rect.height());

  QRect availableRect = screen->availableGeometry();
  result->availableRect = blink::WebRect(availableRect.x(),
                                         availableRect.y(),
                                         availableRect.width(),
                                         availableRect.height());
}

float RenderWidgetHostView::GetDeviceScaleFactor() const {
  return GetDeviceScaleFactorFromQScreen(delegate_->GetScreen());
}

void RenderWidgetHostView::HandleFocusEvent(QFocusEvent* event) {
  if (event->gotFocus()) {
    OnFocus();
    if ((input_type_ != ui::TEXT_INPUT_TYPE_NONE) &&
        !QGuiApplication::inputMethod()->isVisible()) {
      // the focused node hasn’t changed and it is an input field
      delegate_->SetInputMethodEnabled(true);
      QGuiApplication::inputMethod()->show();
    }
  } else {
    OnBlur();
  }
}

void RenderWidgetHostView::HandleKeyEvent(QKeyEvent* event) {
  content::NativeWebKeyboardEvent e(MakeNativeWebKeyboardEvent(event, false));
  GetRenderWidgetHost()->ForwardKeyboardEvent(e);

  // If the event is a printable character, send a corresponding Char event
  if (event->type() == QEvent::KeyPress && QChar(e.text[0]).isPrint()) {
    GetRenderWidgetHost()->ForwardKeyboardEvent(
        MakeNativeWebKeyboardEvent(event, true));
  }
}

void RenderWidgetHostView::HandleMouseEvent(QMouseEvent* event) {
  if (!(event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::NoButton)) {
    event->ignore();
    return;
  }

  GetRenderWidgetHost()->ForwardMouseEvent(
      MakeWebMouseEvent(event, GetDeviceScaleFactor()));
}

void RenderWidgetHostView::HandleWheelEvent(QWheelEvent* event) {
  GetRenderWidgetHost()->ForwardWheelEvent(
      MakeWebMouseWheelEvent(event, GetDeviceScaleFactor()));
}

// Qt input methods don’t generate key events, but a lot of web pages out there
// rely on keydown and keyup events to e.g. perform search-as-you-type or
// enable/disable a submit button based on the contents of a text input field,
// so we send a fake pair of keydown/keyup events.
// This mimicks what is done in GtkIMContextWrapper::HandlePreeditChanged(…)
// and GtkIMContextWrapper::HandleCommit(…)
// (see content/browser/renderer_host/gtk_im_context_wrapper.cc).
static void sendFakeCompositionKeyEvent(content::RenderWidgetHostImpl* rwh,
                                        blink::WebInputEvent::Type type) {
  content::NativeWebKeyboardEvent fake_event;
  fake_event.windowsKeyCode = ui::VKEY_PROCESSKEY;
  fake_event.skip_in_browser = true;
  fake_event.type = type;
  rwh->ForwardKeyboardEvent(fake_event);
}

void RenderWidgetHostView::HandleInputMethodEvent(QInputMethodEvent* event) {
  content::RenderWidgetHostImpl* rwh =
      content::RenderWidgetHostImpl::From(GetRenderWidgetHost());

  QString preedit = event->preeditString();
  if (preedit.isEmpty()) {
    int replacementStart = event->replacementStart();
    int replacementLength = event->replacementLength();
    gfx::Range replacementRange = gfx::Range::InvalidRange();
    if (replacementLength > 0) {
      replacementRange.set_start(replacementStart);
      replacementRange.set_end(replacementStart + replacementLength);
    }
    sendFakeCompositionKeyEvent(rwh, blink::WebInputEvent::RawKeyDown);
    rwh->ImeConfirmComposition(
        base::UTF8ToUTF16(event->commitString().toStdString()),
        replacementRange, false);
    sendFakeCompositionKeyEvent(rwh, blink::WebInputEvent::KeyUp);
  } else {
    std::vector<blink::WebCompositionUnderline> underlines;
    int cursorPosition = -1;
    gfx::Range selectionRange = gfx::Range::InvalidRange();
    Q_FOREACH (const QInputMethodEvent::Attribute& attribute, event->attributes()) {
      switch (attribute.type) {
      case QInputMethodEvent::Cursor:
        if (attribute.length > 0) {
          cursorPosition = attribute.start;
        }
        break;
      case QInputMethodEvent::Selection:
        selectionRange.set_start(
            qMin(attribute.start, (attribute.start + attribute.length)));
        selectionRange.set_end(
            qMax(attribute.start, (attribute.start + attribute.length)));
        break;
      case QInputMethodEvent::TextFormat: {
        QTextCharFormat textCharFormat =
            attribute.value.value<QTextFormat>().toCharFormat();
        blink::WebColor color = textCharFormat.underlineColor().rgba();
        int start = qMin(attribute.start, (attribute.start + attribute.length));
        int end = qMax(attribute.start, (attribute.start + attribute.length));
        blink::WebCompositionUnderline underline(start, end, color, false);
        underlines.push_back(underline);
        break;
      }
      default:
        break;
      }
    }
    if (!selectionRange.IsValid()) {
      int position = (cursorPosition >= 0) ? cursorPosition : preedit.length();
      selectionRange = gfx::Range(position);
    }
    sendFakeCompositionKeyEvent(rwh, blink::WebInputEvent::RawKeyDown);
    rwh->ImeSetComposition(base::UTF8ToUTF16(preedit.toStdString()), underlines,
                           selectionRange.start(), selectionRange.end());
    sendFakeCompositionKeyEvent(rwh, blink::WebInputEvent::KeyUp);
  }
}

void RenderWidgetHostView::HandleTouchEvent(QTouchEvent* event) {
  // The event’s timestamp is not guaranteed to have the same origin as the
  // internal timedelta used by chromium to calculate speed and displacement
  // for a fling gesture, so we can’t use it.
  base::TimeDelta timestamp(base::TimeTicks::Now() - base::TimeTicks());

  float scale = 1 / GetDeviceScaleFactor();

  for (int i = 0; i < event->touchPoints().size(); ++i) {
    const QTouchEvent::TouchPoint& touch_point = event->touchPoints().at(i);

    int touch_id;
    std::map<int, int>::iterator it = touch_id_map_.find(touch_point.id());
    if (it != touch_id_map_.end()) {
      touch_id = it->second;
    } else {
      touch_id = 0;
      for (std::map<int, int>::iterator it = touch_id_map_.begin();
           it != touch_id_map_.end(); ++it) {
        touch_id = std::max(touch_id, it->second + 1);
      }
      touch_id_map_[touch_point.id()] = touch_id;
    }

    ui::TouchEvent ui_event(
        QTouchPointStateToEventType(touch_point.state()),
        gfx::ScalePoint(gfx::PointF(
          touch_point.pos().x(), touch_point.pos().y()), scale),
        0,
        touch_id,
        timestamp,
        0.0f, 0.0f,
        0.0f,
        float(touch_point.pressure()));
    ui_event.set_root_location(
        gfx::ScalePoint(gfx::PointF(
          touch_point.screenPos().x(), touch_point.screenPos().y()), scale));

    oxide::RenderWidgetHostView::HandleTouchEvent(ui_event);

    if (touch_point.state() == Qt::TouchPointReleased) {
      touch_id_map_.erase(touch_point.id());
    }
  }
}

void RenderWidgetHostView::HandleGeometryChanged() {
  OnResize();
}

QVariant RenderWidgetHostView::InputMethodQuery(
    Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImHints:
      return QVariant(QImHintsFromInputType(input_type_));
    case Qt::ImCursorRectangle: {
      gfx::Rect rect = caret_rect();
      return QRect(rect.x(), rect.y(), rect.width(), rect.height());
    }
    case Qt::ImCursorPosition:
      return static_cast<int>(selection_cursor_position() & INT_MAX);
    case Qt::ImSurroundingText:
      return QString::fromStdString(base::UTF16ToUTF8(selection_text_));
    case Qt::ImCurrentSelection:
      return QString::fromStdString(base::UTF16ToUTF8(GetSelectedText()));
    case Qt::ImAnchorPosition:
      return static_cast<int>(selection_anchor_position() & INT_MAX);
    default:
      break;
  }

  return QVariant();
}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  QRect rect(delegate_->GetViewBoundsPix());
  return gfx::Size(rect.width(), rect.height());
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  delegate_->SetSize(QSize(size.width(), size.height()));
}

gfx::Rect RenderWidgetHostView::GetViewBounds() const {
  QScreen* screen = delegate_->GetScreen();
  if (!screen) {
    return gfx::Rect();
  }

  QRect rect(delegate_->GetViewBoundsPix());
  return gfx::ScaleToEnclosingRect(
      gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height()),
                1.0f / GetDeviceScaleFactor());
}

} // namespace qt
} // namespace oxide
