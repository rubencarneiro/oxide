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

#include "oxide_qquick_render_widget_host_view_delegate.h"

#include <QQuickWindow>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSizeF>

#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

void RenderWidgetHostViewDelegate::ScheduleUpdate(const QRect& rect) {
  update(rect);
  polish();
}

RenderWidgetHostViewDelegate::RenderWidgetHostViewDelegate(
    OxideQQuickWebView* webview) :
    QQuickPaintedItem(webview),
    backing_store_(NULL) {
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
}

void RenderWidgetHostViewDelegate::Blur() {
  setFocus(false);
}

void RenderWidgetHostViewDelegate::Focus() {
  setFocus(true);
}

bool RenderWidgetHostViewDelegate::HasFocus() {
  return hasFocus();
}

void RenderWidgetHostViewDelegate::Show() {
  setVisible(true);
}

void RenderWidgetHostViewDelegate::Hide() {
  setVisible(false);
}

bool RenderWidgetHostViewDelegate::IsShowing() {
  return isVisible();
}

QRect RenderWidgetHostViewDelegate::GetViewBounds() {
  QPointF pos(mapToScene(QPointF(0, 0)));
  if (window()) {
    pos += window()->position();
  }

  return QRect(qRound(pos.x()), qRound(pos.y()),
               qRound(width()), qRound(height()));
}

QRect RenderWidgetHostViewDelegate::GetBoundsInRootWindow() {
  if (!window()) {
    return GetViewBounds();
  }

  return window()->frameGeometry();
}

void RenderWidgetHostViewDelegate::SetSize(const QSize& size) {
  setSize(QSizeF(size));
  polish();
}

QScreen* RenderWidgetHostViewDelegate::GetScreen() {
  if (!window()) {
    return NULL;
  }

  return window()->screen();
}

void RenderWidgetHostViewDelegate::focusInEvent(QFocusEvent* event) {
  Q_ASSERT(event->gotFocus());
  ForwardFocusEvent(event);
}

void RenderWidgetHostViewDelegate::focusOutEvent(QFocusEvent* event) {
  Q_ASSERT(event->lostFocus());
  ForwardFocusEvent(event);
}

void RenderWidgetHostViewDelegate::keyPressEvent(QKeyEvent* event) {
  ForwardKeyEvent(event);
}

void RenderWidgetHostViewDelegate::keyReleaseEvent(QKeyEvent* event) {
  ForwardKeyEvent(event);
}

void RenderWidgetHostViewDelegate::mouseDoubleClickEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderWidgetHostViewDelegate::mouseMoveEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderWidgetHostViewDelegate::mousePressEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
  setFocus(true);
}

void RenderWidgetHostViewDelegate::mouseReleaseEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderWidgetHostViewDelegate::wheelEvent(QWheelEvent* event) {
  ForwardWheelEvent(event);
}

void RenderWidgetHostViewDelegate::hoverMoveEvent(QHoverEvent* event) {
  // QtQuick gives us a hover event unless we have a grab (which
  // happens implicitly on button press). As Chromium doesn't
  // distinguish between the 2, just give it a mouse event
  QPointF window_pos = mapToScene(event->posF());
  QMouseEvent me(QEvent::MouseMove,
                 event->posF(),
                 window_pos,
                 window_pos + window()->position(),
                 Qt::NoButton,
                 Qt::NoButton,
                 event->modifiers());

  ForwardMouseEvent(&me);

  event->setAccepted(me.isAccepted());
}

void RenderWidgetHostViewDelegate::updatePolish() {
  backing_store_ = GetBackingStore();
}

void RenderWidgetHostViewDelegate::paint(QPainter* painter) {
  if (!backing_store_) {
    return;
  }

  QRectF rect(0, 0, width(), height());
  painter->drawPixmap(rect, *backing_store_, rect);
}

} // namespace qquick
} // namespace oxide
