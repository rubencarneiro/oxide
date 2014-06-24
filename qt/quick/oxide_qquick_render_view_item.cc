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
#include "oxide_qquick_accelerated_frame_node.h"
#endif
#include "oxide_qquick_software_frame_node.h"

namespace oxide {
namespace qquick {

class UpdatePaintNodeContext {
 public:
  UpdatePaintNodeContext(RenderViewItem* item)
      : type(oxide::qt::CompositorFrameHandle::TYPE_INVALID),
        item_(item) {}

  virtual ~UpdatePaintNodeContext() {
    item_->DidUpdatePaintNode(type);
  }

  oxide::qt::CompositorFrameHandle::Type type;

 private:
  RenderViewItem* item_;
};

void RenderViewItem::onWindowChanged(QQuickWindow* window) {
  if (window) {
    HandleGeometryChanged();
  }
}

void RenderViewItem::geometryChanged(const QRectF& new_geometry,
                                     const QRectF& old_geometry) {
  QQuickItem::geometryChanged(new_geometry, old_geometry);
  if (window()) {
    HandleGeometryChanged();
  }
}

void RenderViewItem::DidUpdatePaintNode(oxide::qt::CompositorFrameHandle::Type type) {
  last_composited_frame_type_ = type;
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    DidComposite();
  }
  compositor_frame_handle_.reset();
}

RenderViewItem::RenderViewItem() :
    frame_evicted_(false),
    received_new_compositor_frame_(false),
    last_composited_frame_type_(oxide::qt::CompositorFrameHandle::TYPE_INVALID) {
  setFlags(QQuickItem::ItemHasContents |
           QQuickItem::ItemClipsChildrenToShape);

  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);

  connect(this, SIGNAL(windowChanged(QQuickWindow*)),
          this, SLOT(onWindowChanged(QQuickWindow*)));
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

void RenderViewItem::ScheduleUpdate() {
  frame_evicted_ = false;
  received_new_compositor_frame_ = true;

  update();
  polish();
}

void RenderViewItem::EvictCurrentFrame() {
  frame_evicted_ = true;
  received_new_compositor_frame_ = false;

  Q_ASSERT(!compositor_frame_handle_);

  update();
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
  me.accept();

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
  compositor_frame_handle_ = GetCompositorFrameHandle();
}

QSGNode* RenderViewItem::updatePaintNode(
    QSGNode* oldNode,
    UpdatePaintNodeData* data) {
  Q_UNUSED(data);

  UpdatePaintNodeContext context(this);

  if (received_new_compositor_frame_) {
    Q_ASSERT(compositor_frame_handle_);
    Q_ASSERT(!frame_evicted_);
    context.type = compositor_frame_handle_->GetType();
  } else if (!frame_evicted_) {
    Q_ASSERT(!compositor_frame_handle_);
    context.type = last_composited_frame_type_;
  }

  if (context.type != last_composited_frame_type_) {
    delete oldNode;
    oldNode = NULL;
  }

  if (frame_evicted_) {
    printf("Frame evicted (%p)\n", static_cast<void *>(this));
    delete oldNode;
    return NULL;
  }

#if defined(ENABLE_COMPOSITING)
  if (context.type == oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED) {
    AcceleratedFrameNode* node = static_cast<AcceleratedFrameNode *>(oldNode);
    if (!node) {
      node = new AcceleratedFrameNode(this);
    }

    if (received_new_compositor_frame_ || !oldNode) {
      node->updateNode(compositor_frame_handle_);
    }

    return node;
  }
#else
  Q_ASSERT(context.type != oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED);
#endif

  if (context.type == oxide::qt::CompositorFrameHandle::TYPE_SOFTWARE) {
    SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
    if (!node) {
      node = new SoftwareFrameNode(this);
    }

    if (received_new_compositor_frame_ || !oldNode) {
      node->updateNode(compositor_frame_handle_);
    }

    return node;
  }

  Q_ASSERT(context.type == oxide::qt::CompositorFrameHandle::TYPE_INVALID);

  SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(oldNode);
  if (!node) {
    node = new SoftwareFrameNode(this);
  }

  QRectF rect(QPoint(0, 0), QSizeF(width(), height()));

  if (!oldNode || rect != node->rect()) {
    QImage blank(qRound(rect.width()),
                 qRound(rect.height()),
                 QImage::Format_ARGB32);
    blank.fill(Qt::white);
    node->setImage(blank);
  }

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
