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

#include "qt/core/glue/oxide_qt_web_view_adapter.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

#if defined(ENABLE_COMPOSITING)
#include "oxide_qquick_accelerated_render_view_node.h"
#endif
#include "oxide_qquick_painted_render_view_node.h"

namespace oxide {
namespace qquick {

void RenderViewItem::geometryChanged(const QRectF& new_geometry,
                                     const QRectF& old_geometry) {
  QQuickItem::geometryChanged(new_geometry, old_geometry);
  HandleGeometryChanged();
}

RenderViewItem::RenderViewItem() :
    QQuickItem(),
    backing_store_(NULL)
#if defined(ENABLE_COMPOSITING)
    , texture_handle_(NULL),
    is_compositing_enabled_(false),
    is_compositing_enabled_state_changed_(false) {
#else
{
#endif
  setFlag(QQuickItem::ItemHasContents);

  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
}

void RenderViewItem::Init(oxide::qt::WebViewAdapter* view) {
  setParentItem(adapterToQObject<OxideQQuickWebView>(view));
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

void RenderViewItem::UpdateCursor(const QCursor& cursor) {
  setCursor(cursor);
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
  polish();
#else
  Q_ASSERT(0);
#endif
}

void RenderViewItem::focusInEvent(QFocusEvent* event) {
  Q_ASSERT(event->gotFocus());
  HandleFocusEvent(event);
}

void RenderViewItem::focusOutEvent(QFocusEvent* event) {
  Q_ASSERT(event->lostFocus());
  HandleFocusEvent(event);
}

void RenderViewItem::keyPressEvent(QKeyEvent* event) {
  HandleKeyEvent(event);
}

void RenderViewItem::keyReleaseEvent(QKeyEvent* event) {
  HandleKeyEvent(event);
}

void RenderViewItem::mouseDoubleClickEvent(QMouseEvent* event) {
  HandleMouseEvent(event);
}

void RenderViewItem::mouseMoveEvent(QMouseEvent* event) {
  HandleMouseEvent(event);
}

void RenderViewItem::mousePressEvent(QMouseEvent* event) {
  forceActiveFocus();
  HandleMouseEvent(event);
}

void RenderViewItem::mouseReleaseEvent(QMouseEvent* event) {
  HandleMouseEvent(event);
}

void RenderViewItem::wheelEvent(QWheelEvent* event) {
  HandleWheelEvent(event);
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

  HandleMouseEvent(&me);

  event->setAccepted(me.isAccepted());
}

void RenderViewItem::inputMethodEvent(QInputMethodEvent* event) {
  HandleInputMethodEvent(event);
}

void RenderViewItem::touchEvent(QTouchEvent* event) {
  if (event->type() == QEvent::TouchBegin) {
    forceActiveFocus();
  }
  HandleTouchEvent(event);
}

void RenderViewItem::updatePolish() {
  backing_store_ = NULL;
#if defined(ENABLE_COMPOSITING)
  texture_handle_ = NULL;

  if (is_compositing_enabled_) {
    texture_handle_ = GetCurrentTextureHandle();
  } else {
#else
  {
#endif
    backing_store_ = GetBackingStore();
  }
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

#if defined(ENABLE_COMPOSITING)
  if (is_compositing_enabled_) {
    AcceleratedRenderViewNode* node =
        static_cast<AcceleratedRenderViewNode *>(oldNode);
    if (!node) {
      node = new AcceleratedRenderViewNode(this);
    }

    node->setRect(QRectF(QPointF(0, 0), QSizeF(width(), height())));
    node->updateFrontTexture(texture_handle_);

    DidUpdate(false);
    return node;
  }
#endif

  PaintedRenderViewNode* node = static_cast<PaintedRenderViewNode *>(oldNode);
  if (!node) {
    node = new PaintedRenderViewNode();
  }

  QSize size;
  if (backing_store_) {
    size = QSize(backing_store_->width(), backing_store_->height());
  } else {
    size = QSizeF(width(), height()).toSize();
  }

  node->setSize(size);
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
