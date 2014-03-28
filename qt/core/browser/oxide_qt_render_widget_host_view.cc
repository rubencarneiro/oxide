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
#include <QFocusEvent>
#include <QGuiApplication>
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
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_widget_host.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/web/WebCompositionUnderline.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "third_party/WebKit/Source/platform/WindowsKeyboardCodes.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/rect.h"

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"
#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate_p.h"

#include "oxide_qt_backing_store.h"

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

namespace {

double QInputEventTimeToWebEventTime(QInputEvent* qevent) {
  return static_cast<double>(qevent->timestamp() / 1000.0);
}

int QKeyEventKeyCodeToWebEventKeyCode(QKeyEvent* qevent) {
  int qkeycode = qevent->key();

  if (qevent->modifiers() & Qt::KeypadModifier) {
    if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
      return (qkeycode - Qt::Key_0) + VK_NUMPAD0;
    }

    switch (qkeycode) {
    case Qt::Key_Asterisk:
      return VK_MULTIPLY;
    case Qt::Key_Plus:
      return VK_ADD;
    case Qt::Key_Minus:
      return VK_SUBTRACT;
    case Qt::Key_Period:
      return VK_DECIMAL;
    case Qt::Key_Slash:
      return VK_DIVIDE;
    case Qt::Key_PageUp:
      return VK_PRIOR;
    case Qt::Key_PageDown:
      return VK_NEXT;
    case Qt::Key_Home:
      return VK_HOME;
    case Qt::Key_End:
      return VK_END;
    case Qt::Key_Insert:
      return VK_INSERT;
    case Qt::Key_Delete:
      return VK_DELETE;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      return VK_RETURN;
    case Qt::Key_Up:
      return VK_UP;
    case Qt::Key_Down:
      return VK_DOWN;
    case Qt::Key_Left:
      return VK_LEFT;
    case Qt::Key_Right:
      return VK_RIGHT;
    default:
      return 0;
    }
  }

  if (qkeycode >= Qt::Key_A && qkeycode <= Qt::Key_Z) {
    return (qkeycode - Qt::Key_A) + VK_A;
  }
  if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
    return (qkeycode - Qt::Key_0) + VK_0;
  }

  switch (qkeycode) {
  case Qt::Key_Escape:
    return VK_ESCAPE;
  case Qt::Key_Tab:
  case Qt::Key_Backtab:
    return VK_TAB;
  case Qt::Key_Backspace:
    return VK_BACK;
  case Qt::Key_Return:
  case Qt::Key_Enter:
    return VK_RETURN;
  case Qt::Key_Insert:
    return VK_INSERT;
  case Qt::Key_Delete:
    return VK_DELETE;
  case Qt::Key_Pause:
    return VK_PAUSE;
  case Qt::Key_Print:
  case Qt::Key_SysReq:
    return VK_SNAPSHOT;
  case Qt::Key_Clear:
    return VK_CLEAR;
  case Qt::Key_Home:
    return VK_HOME;
  case Qt::Key_End:
    return VK_END;
  case Qt::Key_Left:
    return VK_LEFT;
  case Qt::Key_Right:
    return VK_RIGHT;
  case Qt::Key_Down:
    return VK_DOWN;
  case Qt::Key_PageUp:
    return VK_PRIOR;
  case Qt::Key_PageDown:
    return VK_NEXT;
  case Qt::Key_Shift:
    return VK_SHIFT;
  case Qt::Key_Control:
    return VK_CONTROL;
  case Qt::Key_Meta:
  case Qt::Key_Menu:
    return VK_MENU;
  case Qt::Key_Super_L:
    return VK_LWIN;
  case Qt::Key_Alt:
    return VK_LMENU;
  // case Qt::KeyAltGr:
  //   XXX
  case Qt::Key_CapsLock:
    return VK_CAPITAL;
  case Qt::Key_NumLock:
    return VK_NUMLOCK;
  case Qt::Key_ScrollLock:
    return VK_SCROLL;
  case Qt::Key_Super_R:
    return VK_RWIN;
  // case Qt::Key_HyperL:
  // case Qt::Key_HyperR:
  //   XXX
  case Qt::Key_Help:
    return VK_HELP;
  // case Qt::Key_DirectionL:
  // case Qt::Key_DirectionR:
  //   XXX
  case Qt::Key_Space:
    return VK_SPACE;
  case Qt::Key_Exclam:
    return VK_1;
  case Qt::Key_QuoteDbl:
  case Qt::Key_Apostrophe: // XXX Not sure about this
    return VK_OEM_7;
  case Qt::Key_NumberSign:
    return VK_3;
  case Qt::Key_Dollar:
    return VK_4;
  case Qt::Key_Percent:
    return VK_5;
  case Qt::Key_Ampersand:
    return VK_7;
  case Qt::Key_ParenLeft:
    return VK_9;
  case Qt::Key_ParenRight:
    return VK_0;
  case Qt::Key_Asterisk:
    return VK_8;
  case Qt::Key_Plus:
  case Qt::Key_Equal:
    return VK_OEM_PLUS;
  case Qt::Key_Comma:
  case Qt::Key_Less:
    return VK_OEM_COMMA;
  case Qt::Key_Minus:
  case Qt::Key_Underscore:
    return VK_OEM_MINUS;
  case Qt::Key_Period:
  case Qt::Key_Greater:
    return VK_OEM_PERIOD;
  case Qt::Key_Slash:
  case Qt::Key_Question:
    return VK_OEM_2;
  case Qt::Key_Colon:
  case Qt::Key_Semicolon:
    return VK_OEM_1;
  case Qt::Key_At:
    return VK_2;
  case Qt::Key_BracketLeft:
  case Qt::Key_BraceLeft:
    return VK_OEM_4;
  case Qt::Key_Backslash:
  case Qt::Key_Bar:
    return VK_OEM_5;
  case Qt::Key_BracketRight:
  case Qt::Key_BraceRight:
    return VK_OEM_6;
  case Qt::Key_AsciiCircum:
    return VK_6;
  case Qt::Key_QuoteLeft:
  case Qt::Key_AsciiTilde:
    return VK_OEM_3;
  case Qt::Key_Back:
    return VK_BROWSER_BACK;
  case Qt::Key_Forward:
    return VK_BROWSER_FORWARD;
  case Qt::Key_Stop:
    return VK_BROWSER_STOP;
  case Qt::Key_Refresh:
    return VK_BROWSER_REFRESH;
  case Qt::Key_VolumeDown:
    return VK_VOLUME_DOWN;
  case Qt::Key_VolumeMute:
    return VK_VOLUME_MUTE;
  case Qt::Key_VolumeUp:
    return VK_VOLUME_UP;
  case Qt::Key_MediaPlay:
  case Qt::Key_MediaPause:
  case Qt::Key_MediaTogglePlayPause:
    return VK_MEDIA_PLAY_PAUSE;
  case Qt::Key_MediaStop:
    return VK_MEDIA_STOP;
  case Qt::Key_MediaPrevious:
    return VK_MEDIA_PREV_TRACK;
  case Qt::Key_MediaNext:
    return VK_MEDIA_NEXT_TRACK;
  case Qt::Key_HomePage:
    return VK_BROWSER_HOME;
  case Qt::Key_Favorites:
    return VK_BROWSER_FAVORITES;
  case Qt::Key_Search:
    return VK_BROWSER_SEARCH;
  case Qt::Key_LaunchMail:
    return VK_MEDIA_LAUNCH_MAIL;
  case Qt::Key_LaunchMedia:
    return VK_MEDIA_LAUNCH_MEDIA_SELECT;
  case Qt::Key_Launch0:
    return VK_MEDIA_LAUNCH_APP1;
  case Qt::Key_Launch1:
    return VK_MEDIA_LAUNCH_APP2;
  default:
    break;
  }

  if (qkeycode >= Qt::Key_F1 && qkeycode <= Qt::Key_F24) {
    // We miss Qt::Key_F25 - Qt::Key_F35
    return (qkeycode - Qt::Key_F1) + VK_F1;
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
      return ui::ET_TOUCH_MOVED;
    case Qt::TouchPointStationary:
      return ui::ET_TOUCH_STATIONARY;
    case Qt::TouchPointReleased:
      return ui::ET_TOUCH_RELEASED;
    default:
      NOTREACHED();
      return ui::ET_UNKNOWN;
  }
}

content::NativeWebKeyboardEvent MakeNativeWebKeyboardEvent(
    QKeyEvent* qevent) {
  content::NativeWebKeyboardEvent event;

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);
  event.modifiers = QInputEventStateToWebEventModifiers(qevent);

  if (qevent->isAutoRepeat()) {
    event.modifiers |= blink::WebInputEvent::IsAutoRepeat;
  }

  switch (qevent->type()) {
  case QEvent::KeyPress:
    event.type = blink::WebInputEvent::KeyDown;
    break;
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
  memcpy(event.text, text, qMin(sizeof(event.text), sizeof(*text)));

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

}

void RenderWidgetHostView::Paint(const gfx::Rect& rect) {
  gfx::Rect scaled_rect(
      gfx::ScaleToEnclosingRect(rect, GetDeviceScaleFactor()));
  delegate_->SchedulePaintForRectPix(
      QRect(scaled_rect.x(),
            scaled_rect.y(),
            scaled_rect.width(),
            scaled_rect.height()));
}

void RenderWidgetHostView::BuffersSwapped() {
  delegate_->ScheduleUpdate();
}

RenderWidgetHostView::RenderWidgetHostView(
    content::RenderWidgetHost* render_widget_host,
    RenderWidgetHostViewDelegate* delegate) :
    oxide::RenderWidgetHostView(render_widget_host),
    backing_store_(NULL),
    delegate_(delegate),
    input_type_(ui::TEXT_INPUT_TYPE_NONE) {
  RenderWidgetHostViewDelegatePrivate::get(delegate)->rwhv = this;
}

RenderWidgetHostView::~RenderWidgetHostView() {}

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

void RenderWidgetHostView::Blur() {
  delegate_->Blur();
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

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  QRect rect(delegate_->GetViewBoundsPix());
  return gfx::Size(rect.width(), rect.height());
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  delegate_->SetSize(QSize(size.width(), size.height()));
  oxide::RenderWidgetHostView::SetSize(size);
}

content::BackingStore* RenderWidgetHostView::AllocBackingStore(
    const gfx::Size& size) {
  return new BackingStore(GetRenderWidgetHost(), size, GetDeviceScaleFactor());
}

float RenderWidgetHostView::GetDeviceScaleFactor() const {
  return GetDeviceScaleFactorFromQScreen(delegate_->GetScreen());
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

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {
  if (!HasFocus()) {
    return;
  }
  delegate_->SetInputMethodEnabled(is_editable_node);
  if (QGuiApplication::inputMethod()->isVisible() != is_editable_node) {
    QGuiApplication::inputMethod()->setVisible(is_editable_node);
  }
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

  event->accept();
}

void RenderWidgetHostView::HandleKeyEvent(QKeyEvent* event) {
  GetRenderWidgetHost()->ForwardKeyboardEvent(
      MakeNativeWebKeyboardEvent(event));
  event->accept();
}

void RenderWidgetHostView::HandleMouseEvent(QMouseEvent* event) {
  GetRenderWidgetHost()->ForwardMouseEvent(
      MakeWebMouseEvent(event, GetDeviceScaleFactor()));
  event->accept();
}

void RenderWidgetHostView::HandleWheelEvent(QWheelEvent* event) {
  GetRenderWidgetHost()->ForwardWheelEvent(
      MakeWebMouseWheelEvent(event, GetDeviceScaleFactor()));
  event->accept();
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
    rwh->ImeConfirmComposition(
        base::UTF8ToUTF16(event->commitString().toStdString()),
        replacementRange, false);
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
    rwh->ImeSetComposition(base::UTF8ToUTF16(preedit.toStdString()), underlines,
                           selectionRange.start(), selectionRange.end());
  }

  event->accept();
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

  event->accept();
}

void RenderWidgetHostView::DidUpdate(bool skipped) {
  AcknowledgeBuffersSwapped(skipped);
}

const QPixmap* RenderWidgetHostView::GetBackingStore() {
  content::RenderWidgetHostImpl* rwh =
      content::RenderWidgetHostImpl::From(GetRenderWidgetHost());
  BackingStore* backing_store =
      static_cast<BackingStore *>(rwh->GetBackingStore(!rwh->empty()));
  if (!backing_store) {
    return NULL;
  }

  return backing_store->pixmap();
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

} // namespace qt
} // namespace oxide
