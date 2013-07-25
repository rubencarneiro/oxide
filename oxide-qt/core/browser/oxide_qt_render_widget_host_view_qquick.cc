// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qt_render_widget_host_view_qquick.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QRect>
#include <QScreen>

#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/public/web/WebScreenInfo.h"
#include "third_party/WebKit/Source/core/platform/WindowsKeyboardCodes.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/gfx/rect.h"

#include "oxide_qt_backing_store.h"

QT_USE_NAMESPACE

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
    modifiers |= WebKit::WebInputEvent::ShiftKey;
  }
  if (qmodifiers & Qt::ControlModifier) {
    modifiers |= WebKit::WebInputEvent::ControlKey;
  }
  if (qmodifiers & Qt::AltModifier) {
    modifiers |= WebKit::WebInputEvent::AltKey;
  }
  if (qmodifiers & Qt::MetaModifier) {
    modifiers |= WebKit::WebInputEvent::MetaKey;
  }
  if (qmodifiers & Qt::KeypadModifier) {
    modifiers |= WebKit::WebInputEvent::IsKeyPad;
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
    modifiers |= WebKit::WebInputEvent::LeftButtonDown;
  }
  if (buttons & Qt::MidButton &&
      (!mouse_down || event_button != Qt::MidButton)) {
    modifiers |= WebKit::WebInputEvent::MiddleButtonDown;
  }
  if (buttons & Qt::RightButton &&
      (!mouse_down || event_button != Qt::RightButton)) {
    modifiers |= WebKit::WebInputEvent::RightButtonDown;
  }

  modifiers |= QInputEventStateToWebEventModifiers(qevent);

  return modifiers;
}

content::NativeWebKeyboardEvent QKeyEventToWebEvent(
    QKeyEvent* qevent) {
  content::NativeWebKeyboardEvent event;

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);
  event.modifiers = QInputEventStateToWebEventModifiers(qevent);

  if (qevent->isAutoRepeat()) {
    event.modifiers |= WebKit::WebInputEvent::IsAutoRepeat;
  }

  switch (qevent->type()) {
  case QEvent::KeyPress:
    event.type = WebKit::WebInputEvent::KeyDown;
    break;
  case QEvent::KeyRelease:
    event.type = WebKit::WebInputEvent::KeyUp;
    break;
  default:
    NOTREACHED();
  }

  if (event.modifiers & WebKit::WebInputEvent::AltKey) {
    event.isSystemKey = true;
  }

  int windowsKeyCode = QKeyEventKeyCodeToWebEventKeyCode(qevent);
  event.windowsKeyCode =
      WebKit::WebKeyboardEvent::windowsKeyCodeWithoutLocation(windowsKeyCode);
  event.modifiers |=
      WebKit::WebKeyboardEvent::locationModifiersFromWindowsKeyCode(
        windowsKeyCode);
  event.nativeKeyCode = qevent->key();

  const unsigned short* text = qevent->text().utf16();
  memcpy(&event.text, text, qMin(sizeof(event.text), sizeof(text)));

  return event;
}

WebKit::WebMouseEvent QMouseEventToWebEvent(QMouseEvent* qevent) {
  WebKit::WebMouseEvent event;

  event.timeStampSeconds = QInputEventTimeToWebEventTime(qevent);
  event.modifiers = QMouseEventStateToWebEventModifiers(qevent);

  event.x = qevent->x();
  event.y = qevent->y();

  QPointF wp(qevent->windowPos());
  event.windowX = qRound(wp.x());
  event.windowY = qRound(wp.y());

  event.globalX = qevent->globalX();
  event.globalY = qevent->globalY();

  event.clickCount = 0;

  switch (qevent->type()) {
  case QEvent::MouseButtonPress:
    event.type = WebKit::WebInputEvent::MouseDown;
    event.clickCount = 1;
    break;
  case QEvent::MouseButtonRelease:
    event.type = WebKit::WebInputEvent::MouseUp;
    break;
  case QEvent::MouseMove:
    event.type = WebKit::WebInputEvent::MouseMove;
    break;
  case QEvent::MouseButtonDblClick:
    event.type = WebKit::WebInputEvent::Undefined;
    event.clickCount = 2;
  default:
    NOTREACHED();
  }

  switch(qevent->button()) {
  case Qt::LeftButton:
    event.button = WebKit::WebMouseEvent::ButtonLeft;
    break;
  case Qt::MidButton:
    event.button = WebKit::WebMouseEvent::ButtonMiddle;
    break;
  case Qt::RightButton:
    event.button = WebKit::WebMouseEvent::ButtonRight;
    break;
  default:
    event.button = WebKit::WebMouseEvent::ButtonNone;
    DCHECK_EQ(event.type, WebKit::WebMouseEvent::MouseMove);
  }

  return event;
}

WebKit::WebMouseWheelEvent QWheelEventToWebEvent(QWheelEvent* qevent,
                                                 QQuickItem* item) {
  WebKit::WebMouseWheelEvent event;

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

  event.type = WebKit::WebInputEvent::MouseWheel;
  event.button = WebKit::WebMouseEvent::ButtonNone;

  event.x = qevent->x();
  event.y = qevent->y();

  QPointF wp(item->mapToScene(qevent->posF()));
  event.windowX = qRound(wp.x());
  event.windowY = qRound(wp.y());

  event.globalX = qevent->globalX();
  event.globalY = qevent->globalY();

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

class OxideRenderViewItem : public QQuickPaintedItem {
 public:
  OxideRenderViewItem(oxide::qt::RenderWidgetHostViewQQuick* owner,
                      QQuickItem* parent) :
      QQuickPaintedItem(parent),
      owner_(owner),
      backing_store_(NULL) {}

  virtual ~OxideRenderViewItem() {}

 private:
  virtual void focusInEvent(QFocusEvent* event);
  virtual void focusOutEvent(QFocusEvent* event);

  virtual void keyPressEvent(QKeyEvent* event);
  virtual void keyReleaseEvent(QKeyEvent* event);

  virtual void mouseDoubleClickEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);

  virtual void wheelEvent(QWheelEvent* event);

  virtual void updatePolish();
  virtual void paint(QPainter* paint);

  oxide::qt::RenderWidgetHostViewQQuick* owner_;
  oxide::qt::BackingStore* backing_store_;
};

void OxideRenderViewItem::focusInEvent(QFocusEvent* event) {
  DCHECK(event->gotFocus());

  owner_->OnFocus();
  event->accept();
}

void OxideRenderViewItem::focusOutEvent(QFocusEvent* event) {
  DCHECK(event->lostFocus());

  owner_->OnBlur();
  event->accept();
}

void OxideRenderViewItem::keyPressEvent(QKeyEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardKeyboardEvent(
      QKeyEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::keyReleaseEvent(QKeyEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardKeyboardEvent(
      QKeyEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::mouseDoubleClickEvent(QMouseEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardMouseEvent(
      QMouseEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::mouseMoveEvent(QMouseEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardMouseEvent(
      QMouseEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::mousePressEvent(QMouseEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardMouseEvent(
      QMouseEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::mouseReleaseEvent(QMouseEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardMouseEvent(
      QMouseEventToWebEvent(event));
  event->accept();
}

void OxideRenderViewItem::wheelEvent(QWheelEvent* event) {
  owner_->GetRenderWidgetHost()->ForwardWheelEvent(
      QWheelEventToWebEvent(event, this));
  event->accept();
}

void OxideRenderViewItem::updatePolish() {
  bool force_create = !owner_->GetRenderWidgetHostImpl()->empty();
  backing_store_ = static_cast<oxide::qt::BackingStore *>(
      owner_->GetRenderWidgetHostImpl()->GetBackingStore(force_create));
}

void OxideRenderViewItem::paint(QPainter* painter) {
  QRectF rect(0, 0, width(), height());
  painter->drawPixmap(rect, *backing_store_->pixmap(), rect);
}

}

namespace oxide {
namespace qt {

void RenderWidgetHostViewQQuick::ScheduleUpdate(const gfx::Rect& rect) {
  view_item_->update(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
  view_item_->polish();
}

RenderWidgetHostViewQQuick::RenderWidgetHostViewQQuick(
    content::RenderWidgetHost* render_widget_host,
    QQuickItem* container) :
    oxide::RenderWidgetHostView(render_widget_host),
    view_item_(new OxideRenderViewItem(this, container)) {}

// static
void RenderWidgetHostViewQQuick::GetScreenInfo(
    QScreen* screen, WebKit::WebScreenInfo* result) {
  result->depth = screen->depth();
  result->depthPerComponent = 8; // XXX: Copied the GTK impl here
  result->isMonochrome = result->depth == 1;

  QRect rect = screen->geometry();
  result->rect = WebKit::WebRect(rect.x(),
                                 rect.y(),
                                 rect.width(),
                                 rect.height());

  QRect availableRect = screen->availableGeometry();
  result->availableRect = WebKit::WebRect(availableRect.x(),
                                          availableRect.y(),
                                          availableRect.width(),
                                          availableRect.height());
}

void RenderWidgetHostViewQQuick::Blur() {
  view_item_->setFocus(false);
}

void RenderWidgetHostViewQQuick::Focus() {
  view_item_->setFocus(true);
}

bool RenderWidgetHostViewQQuick::HasFocus() const {
  return view_item_->hasFocus();
}

void RenderWidgetHostViewQQuick::Show() {
  view_item_->setVisible(true);
}

void RenderWidgetHostViewQQuick::Hide() {
  view_item_->setVisible(false);
}

bool RenderWidgetHostViewQQuick::IsShowing() {
  return view_item_->isVisible();
}

gfx::Rect RenderWidgetHostViewQQuick::GetViewBounds() const {
  QPointF pos(view_item_->mapToScene(QPointF(0,0)));
  pos += QPointF(view_item_->window()->x(), view_item_->window()->y());

  return gfx::Rect(qRound(pos.x()),
                   qRound(pos.y()),
                   qRound(view_item_->width()),
                   qRound(view_item_->height()));
}

content::BackingStore* RenderWidgetHostViewQQuick::AllocBackingStore(
    const gfx::Size& size) {
  return new BackingStore(GetRenderWidgetHost(), size);
}

void RenderWidgetHostViewQQuick::GetScreenInfo(
    WebKit::WebScreenInfo* results) {
  GetScreenInfo(view_item_->window()->screen(), results);
}

gfx::Rect RenderWidgetHostViewQQuick::GetBoundsInRootWindow() {
  QRect rect = view_item_->window()->frameGeometry();
  return gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

} // namespace qt
} // namespace oxide
