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

#include "oxide_qt_contents_view.h"

#include <QEvent>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTouchEvent>

#include "base/memory/scoped_vector.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/common/drop_data.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/point.h"

#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"
#include "shared/browser/oxide_web_contents_view.h"

#include "oxide_qt_drag_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_web_context_menu.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

float ContentsView::GetDeviceScaleFactor() const {
  QScreen* screen = client_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetDeviceScaleFactorFromQScreen(screen);
}

float ContentsView::GetLocationBarContentOffsetDip() {
  if (location_bar_content_offset_getter_.is_null()) {
    return 0.f;
  }

  return location_bar_content_offset_getter_.Run();
}

void ContentsView::handleKeyEvent(QKeyEvent* event) {
  content::NativeWebKeyboardEvent e(MakeNativeWebKeyboardEvent(event, false));
  view()->HandleKeyEvent(e);

  // If the event is a printable character, send a corresponding Char event
  if (event->type() == QEvent::KeyPress && e.text[0] != 0) {
    view()->HandleKeyEvent(MakeNativeWebKeyboardEvent(event, true));
  }

  event->accept();
}

void ContentsView::handleMouseEvent(QMouseEvent* event) {
  if (!(event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::NoButton)) {
    event->ignore();
    return;
  }

  view()->HandleMouseEvent(
      MakeWebMouseEvent(event,
                        GetDeviceScaleFactor(),
                        GetLocationBarContentOffsetDip()));
  event->accept();
}

void ContentsView::handleHoverEvent(QHoverEvent* event,
                                    const QPoint& window_pos,
                                    const QPoint& global_pos) {
  view()->HandleMouseEvent(
      MakeWebMouseEvent(event,
                        window_pos,
                        global_pos,
                        GetDeviceScaleFactor(),
                        GetLocationBarContentOffsetDip()));
  event->accept();
}

void ContentsView::handleTouchEvent(QTouchEvent* event) {
  ScopedVector<ui::TouchEvent> events;
  touch_event_factory_.MakeEvents(event,
                                  GetDeviceScaleFactor(),
                                  GetLocationBarContentOffsetDip(),
                                  &events);

  for (size_t i = 0; i < events.size(); ++i) {
    view()->HandleTouchEvent(*events[i]);
  }

  event->accept();
}

void ContentsView::handleWheelEvent(QWheelEvent* event,
                                    const QPoint& window_pos) {
  view()->HandleWheelEvent(
      MakeWebMouseWheelEvent(event,
                             window_pos,
                             GetDeviceScaleFactor(),
                             GetLocationBarContentOffsetDip()));
  event->accept();
}

void ContentsView::handleDragEnterEvent(QDragEnterEvent* event) {
  content::DropData drop_data;
  gfx::Point location;
  blink::WebDragOperationsMask allowed_ops = blink::WebDragOperationNone;
  int key_modifiers = 0;

  GetDragEnterEventParams(event,
                          GetDeviceScaleFactor(),
                          &drop_data,
                          &location,
                          &allowed_ops,
                          &key_modifiers);

  view()->HandleDragEnter(drop_data, location, allowed_ops, key_modifiers);

  event->accept();
}

void ContentsView::handleDragMoveEvent(QDragMoveEvent* event) {
  gfx::Point location;
  int key_modifiers = 0;

  GetDropEventParams(event, GetDeviceScaleFactor(), &location, &key_modifiers);

  blink::WebDragOperation op = view()->HandleDragMove(location, key_modifiers);

  Qt::DropAction action;
  if ((action = ToQtDropAction(op)) != Qt::IgnoreAction) {
    event->setDropAction(action);
    event->accept();
  } else {
    event->ignore();
  }
}

void ContentsView::handleDragLeaveEvent(QDragLeaveEvent* event) {
  view()->HandleDragLeave();
}

void ContentsView::handleDropEvent(QDropEvent* event) {
  gfx::Point location;
  int key_modifiers = 0;

  GetDropEventParams(event, GetDeviceScaleFactor(), &location, &key_modifiers);

  blink::WebDragOperation op = view()->HandleDrop(location, key_modifiers);

  Qt::DropAction action;
  if ((action = ToQtDropAction(op)) != Qt::IgnoreAction) {
    event->setDropAction(action);
    event->accept();
  } else {
    event->ignore();
  }
}

blink::WebScreenInfo ContentsView::GetScreenInfo() const {
  QScreen* screen = client_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetWebScreenInfoFromQScreen(screen);
}

gfx::Rect ContentsView::GetBoundsPix() const {
  QRect bounds = client_->GetBoundsPix();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

oxide::WebContextMenu* ContentsView::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  WebContextMenu* menu = new WebContextMenu(rfh, params);
  menu->SetProxy(client_->CreateWebContextMenu(menu));
  return menu;
}

oxide::WebPopupMenu* ContentsView::CreatePopupMenu(
    content::RenderFrameHost* rfh) {
  WebPopupMenu* menu = new WebPopupMenu(rfh);
  menu->SetProxy(client_->CreateWebPopupMenu(menu));
  return menu;
}

ContentsView::ContentsView(
    ContentsViewProxyClient* client,
    QObject* native_view,
    const base::Callback<float(void)>& location_bar_content_offset_getter)
    : client_(client),
      native_view_(native_view),
      location_bar_content_offset_getter_(location_bar_content_offset_getter) {
  DCHECK(!client_->proxy_);
  client_->proxy_ = this;
}

ContentsView::~ContentsView() {
  DCHECK_EQ(client_->proxy_, this);
  client_->proxy_ = nullptr;
}

// static
ContentsView* ContentsView::FromWebContents(content::WebContents* contents) {
  oxide::WebContentsView* view =
      oxide::WebContentsView::FromWebContents(contents);
  if (!view) {
    return nullptr;
  }

  return static_cast<ContentsView*>(view->client());
}

} // namespace qt
} // namespace oxide
