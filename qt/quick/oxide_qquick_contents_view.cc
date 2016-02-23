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

#include "oxide_qquick_contents_view.h"

#include <QHoverEvent>
#include <QPointF>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTouchEvent>

#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"

#include "oxide_qquick_accelerated_frame_node.h"
#include "oxide_qquick_image_frame_node.h"
#include "oxide_qquick_software_frame_node.h"
#include "oxide_qquick_touch_handle_drawable.h"
#include "oxide_qquick_web_context_menu.h"
#include "oxide_qquick_web_popup_menu.h"

namespace oxide {
namespace qquick {

class UpdatePaintNodeScope {
 public:
  UpdatePaintNodeScope(ContentsView* view)
      : view_(view) {}

  ~UpdatePaintNodeScope() {
    view_->didUpdatePaintNode();
  }

 private:
  ContentsView* view_;
};

void ContentsView::handleKeyEvent(QKeyEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleKeyEvent(event);
}

void ContentsView::handleMouseEvent(QMouseEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleMouseEvent(event);
}

void ContentsView::handleHoverEvent(QHoverEvent* event) {
  if (!proxy()) {
    return;
  }

  QPointF window_pos = item_->mapToScene(event->posF());
  proxy()->handleHoverEvent(event,
                            window_pos.toPoint(),
                            (window_pos + item_->window()->position()).toPoint());
}

void ContentsView::didUpdatePaintNode() {
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    proxy()->didCommitCompositorFrame();
  }
}

QScreen* ContentsView::GetScreen() const {
  if (!item_->window()) {
    return nullptr;
  }

  return item_->window()->screen();
}

bool ContentsView::IsVisible() const {
  return item_->isVisible();
}

bool ContentsView::HasFocus() const {
  return item_->hasActiveFocus() &&
      (item_->window() ? item_->window()->isActive() : false);
}

QRect ContentsView::GetBoundsPix() const {
  if (!item_->window()) {
    return QRect();
  }

  QPointF pos(item_->mapToScene(QPointF(0, 0)) + item_->window()->position());

  return QRect(qRound(pos.x()), qRound(pos.y()),
               qRound(item_->width()), qRound(item_->height()));
}

void ContentsView::ScheduleUpdate() {
  frame_evicted_ = false;
  received_new_compositor_frame_ = true;

  item_->update();
}

void ContentsView::EvictCurrentFrame() {
  frame_evicted_ = true;
  received_new_compositor_frame_ = false;

  item_->update();
}

void ContentsView::UpdateCursor(const QCursor& cursor) {
  item_->setCursor(cursor);
}

oxide::qt::WebContextMenuProxy* ContentsView::CreateWebContextMenu(
    oxide::qt::WebContextMenuProxyClient* client) {
  return new WebContextMenu(item_, context_menu_, client);
}

oxide::qt::WebPopupMenuProxy* ContentsView::CreateWebPopupMenu(
    oxide::qt::WebPopupMenuProxyClient* client) {
  return new WebPopupMenu(item_, popup_menu_, client);
}

oxide::qt::TouchHandleDrawableProxy*
ContentsView::CreateTouchHandleDrawable() {
  return new TouchHandleDrawable(item_, touch_selection_controller_);
}

void ContentsView::TouchSelectionChanged(bool active, const QRectF& bounds) {
  if (touch_selection_controller_) {
    touch_selection_controller_->onTouchSelectionChanged(active, bounds);
  }
}

ContentsView::ContentsView(QQuickItem* item)
    : item_(item),
      received_new_compositor_frame_(false),
      frame_evicted_(false),
      last_composited_frame_type_(
          oxide::qt::CompositorFrameHandle::TYPE_INVALID) {}

ContentsView::~ContentsView() {}

void ContentsView::handleKeyPressEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleKeyReleaseEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleMousePressEvent(QMouseEvent* event) {
  item_->forceActiveFocus();
  handleMouseEvent(event);
}

void ContentsView::handleMouseMoveEvent(QMouseEvent* event) {
  handleMouseEvent(event);
}

void ContentsView::handleMouseReleaseEvent(QMouseEvent* event) {
  handleMouseEvent(event);
}

void ContentsView::handleWheelEvent(QWheelEvent* event) {
  if (!proxy()) {
    return;
  }

  QPointF window_pos = item_->mapToScene(event->posF());
  proxy()->handleWheelEvent(event, window_pos.toPoint());
}

void ContentsView::handleTouchEvent(QTouchEvent* event) {
  if (!proxy()) {
    return;
  }

  if (event->type() == QEvent::TouchBegin) {
    item_->forceActiveFocus();
  }
  proxy()->handleTouchEvent(event);
}

void ContentsView::handleHoverEnterEvent(QHoverEvent* event) {
  handleHoverEvent(event);
}

void ContentsView::handleHoverMoveEvent(QHoverEvent* event) {
  handleHoverEvent(event);
}

void ContentsView::handleHoverLeaveEvent(QHoverEvent* event) {
  handleHoverEvent(event);
}

void ContentsView::handleDragEnterEvent(QDragEnterEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleDragEnterEvent(event);
}

void ContentsView::handleDragMoveEvent(QDragMoveEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleDragMoveEvent(event);
}

void ContentsView::handleDragLeaveEvent(QDragLeaveEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleDragLeaveEvent(event);
}

void ContentsView::handleDropEvent(QDropEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleDropEvent(event);
}

QSGNode* ContentsView::updatePaintNode(QSGNode* old_node) {
  UpdatePaintNodeScope scope(this);

  oxide::qt::CompositorFrameHandle::Type type =
      oxide::qt::CompositorFrameHandle::TYPE_INVALID;
  QSharedPointer<oxide::qt::CompositorFrameHandle> handle;

  if (proxy()) {
    handle = proxy()->compositorFrameHandle();
    type = handle->GetType();
  }

  Q_ASSERT(!received_new_compositor_frame_ ||
           (received_new_compositor_frame_ && !frame_evicted_));

  if (type != last_composited_frame_type_) {
    delete old_node;
    old_node = nullptr;
  }

  last_composited_frame_type_ = type;

  if (frame_evicted_) {
    delete old_node;
    return nullptr;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_ACCELERATED) {
    AcceleratedFrameNode* node = static_cast<AcceleratedFrameNode *>(old_node);
    if (!node) {
      node = new AcceleratedFrameNode(item_);
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_IMAGE) {
    ImageFrameNode* node = static_cast<ImageFrameNode *>(old_node);
    if (!node) {
      node = new ImageFrameNode();
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == oxide::qt::CompositorFrameHandle::TYPE_SOFTWARE) {
    SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(old_node);
    if (!node) {
      node = new SoftwareFrameNode(item_);
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  Q_ASSERT(type == oxide::qt::CompositorFrameHandle::TYPE_INVALID);

  SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(old_node);
  if (!node) {
    node = new SoftwareFrameNode(item_);
  }

  QRectF rect(QPointF(0, 0), QSizeF(item_->width(), item_->height()));

  if (!old_node || rect != node->rect()) {
    QImage blank(qRound(rect.width()),
                 qRound(rect.height()),
                 QImage::Format_ARGB32);
    blank.fill(Qt::white);
    node->setImage(blank);
  }

  return node;
}

} // namespace qquick
} // namespace oxide
