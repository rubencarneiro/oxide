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

#include "oxide_qquick_render_view_item.h"

#include <QGuiApplication>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QQuickWindow>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QTouchEvent>

#include "qt/quick/api/oxideqquickwebview_p.h"

#if defined(ENABLE_COMPOSITING)
#include "oxide_qquick_accelerated_render_view_node.h"
#endif
#include "oxide_qquick_painted_render_view_node.h"

namespace oxide {
namespace qquick {

void RenderViewItem::SchedulePaintForRectPix(const QRect& rect) {
#if defined(ENABLE_COMPOSITING)
  if (is_compositing_enabled_) {
    is_compositing_enabled_state_changed_ = true;
    is_compositing_enabled_ = false;
  }
#endif

  if (rect.isNull() && !dirty_rect_.isNull()) {
    dirty_rect_ = QRectF(0, 0, width(), height()).toAlignedRect();
  } else {
    dirty_rect_ |= (QRectF(0, 0, width(), height()) & rect).toAlignedRect();
  }

  update();
  polish();
}

void RenderViewItem::ScheduleUpdate() {
#if defined(ENABLE_COMPOSITING)
  if (!is_compositing_enabled_) {
    is_compositing_enabled_state_changed_ = true;
    is_compositing_enabled_ = true;
  }

  update();
#else
  Q_ASSERT(0);
#endif
}

RenderViewItem::RenderViewItem(
    OxideQQuickWebView* webview) :
    QQuickItem(webview),
    backing_store_(NULL)
#if defined(ENABLE_COMPOSITING)
    , is_compositing_enabled_(false),
    is_compositing_enabled_state_changed_(false) {
#else
{
#endif
  setFlag(QQuickItem::ItemHasContents);

  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
}

void RenderViewItem::Blur() {
  setFocus(false);
}

void RenderViewItem::Focus() {
  setFocus(true);
}

bool RenderViewItem::HasFocus() {
  return hasFocus();
}

void RenderViewItem::Show() {
  setVisible(true);
}

void RenderViewItem::Hide() {
  setVisible(false);
}

bool RenderViewItem::IsShowing() {
  return isVisible();
}

QRect RenderViewItem::GetViewBoundsPix() {
  if (!window()) {
    return QRect();
  }

  QPointF pos(mapToScene(QPointF(0, 0)) + window()->position());

  return QRect(qRound(pos.x()), qRound(pos.y()),
               qRound(width()), qRound(height()));
}

void RenderViewItem::SetSize(const QSize& size) {
  setSize(QSizeF(size));
  polish();
}

QScreen* RenderViewItem::GetScreen() {
  if (!window()) {
    return NULL;
  }

  return window()->screen();
}

void RenderViewItem::SetInputMethodEnabled(bool enabled) {
  setFlag(QQuickItem::ItemAcceptsInputMethod, enabled);
  QGuiApplication::inputMethod()->update(Qt::ImEnabled);
}

void RenderViewItem::focusInEvent(QFocusEvent* event) {
  Q_ASSERT(event->gotFocus());
  ForwardFocusEvent(event);
}

void RenderViewItem::focusOutEvent(QFocusEvent* event) {
  Q_ASSERT(event->lostFocus());
  ForwardFocusEvent(event);
}

void RenderViewItem::keyPressEvent(QKeyEvent* event) {
  ForwardKeyEvent(event);
}

void RenderViewItem::keyReleaseEvent(QKeyEvent* event) {
  ForwardKeyEvent(event);
}

void RenderViewItem::mouseDoubleClickEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderViewItem::mouseMoveEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderViewItem::mousePressEvent(QMouseEvent* event) {
  forceActiveFocus();
  ForwardMouseEvent(event);
}

void RenderViewItem::mouseReleaseEvent(QMouseEvent* event) {
  ForwardMouseEvent(event);
}

void RenderViewItem::wheelEvent(QWheelEvent* event) {
  ForwardWheelEvent(event);
}

void RenderViewItem::hoverMoveEvent(QHoverEvent* event) {
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

void RenderViewItem::inputMethodEvent(QInputMethodEvent* event) {
  ForwardInputMethodEvent(event);
}

void RenderViewItem::touchEvent(QTouchEvent* event) {
  if (event->type() == QEvent::TouchBegin) {
    forceActiveFocus();
  }
  ForwardTouchEvent(event);
}

void RenderViewItem::updatePolish() {
  backing_store_ = GetBackingStore();
}

QSGNode* RenderViewItem::updatePaintNode(
    QSGNode* oldNode,
    UpdatePaintNodeData* data) {
  Q_UNUSED(data);

#if defined(ENABLE_COMPOSITING)
  if (is_compositing_enabled_state_changed_) {
    delete oldNode;
    oldNode = NULL;
    is_compositing_enabled_state_changed_ = false;
  }
#endif

  if (width() <= 0 || height() <= 0) {
    delete oldNode;
    DidUpdate(true);
    return NULL;
  }

  // FIXME: What if we had a resize between scheduling the update
  //        on the main thread and now?

#if defined(ENABLE_COMPOSITING)
  if (is_compositing_enabled_) {
    AcceleratedRenderViewNode* node =
        static_cast<AcceleratedRenderViewNode *>(oldNode);
    if (!node) {
      node = new AcceleratedRenderViewNode(this);
    }

    node->setRect(QRectF(QPointF(0, 0), QSizeF(width(), height())));
    node->updateFrontTexture(GetFrontbufferTextureInfo());

    DidUpdate(false);
    return node;
  }
#endif

  PaintedRenderViewNode* node = static_cast<PaintedRenderViewNode *>(oldNode);
  if (!node) {
    node = new PaintedRenderViewNode();
  }

  node->setSize(QSizeF(width(), height()).toSize());
  node->setBackingStore(backing_store_);
  node->markDirtyRect(dirty_rect_);

  node->update();

  dirty_rect_ = QRect();

  return node;
}

QVariant RenderViewItem::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImEnabled:
      return (flags() & QQuickItem::ItemAcceptsInputMethod) != 0;
    default:
      return InputMethodQuery(query);
  }
}

} // namespace qquick
} // namespace oxide
