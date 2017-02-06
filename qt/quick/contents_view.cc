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

#include "contents_view.h"

#include <QGuiApplication>
#include <QHoverEvent>
#include <QPointF>
#include <QQuickWindow>
#include <QScreen>
#include <QTouchEvent>

#include "qt/core/glue/web_popup_menu.h"

#include "oxide_qquick_accelerated_frame_node.h"
#include "oxide_qquick_image_frame_node.h"
#include "oxide_qquick_software_frame_node.h"

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
  if (!view()) {
    return;
  }

  if (handling_unhandled_key_event_) {
    return;
  }

  view()->handleKeyEvent(event);
}

void ContentsView::handleMouseEvent(QMouseEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleMouseEvent(event);
}

void ContentsView::handleHoverEvent(QHoverEvent* event) {
  if (!view()) {
    return;
  }

  QPointF window_pos = item_->mapToScene(event->posF());
  view()->handleHoverEvent(event,
                            window_pos,
                            (window_pos + item_->window()->position()).toPoint());
}

void ContentsView::handleFocusEvent(QFocusEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleFocusEvent(event);
}

void ContentsView::didUpdatePaintNode() {
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    view()->didCommitCompositorFrame();
  }
}

QWindow* ContentsView::GetWindow() const {
  return item_->window();
}

bool ContentsView::IsVisible() const {
  return item_->isVisible();
}

bool ContentsView::HasFocus() const {
  return item_->hasActiveFocus() &&
      (item_->window() ? item_->window()->isActive() : false);
}

QRect ContentsView::GetBounds() const {
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

void ContentsView::SetInputMethodEnabled(bool enabled) {
  item_->setFlag(QQuickItem::ItemAcceptsInputMethod, enabled);
  QGuiApplication::inputMethod()->update(Qt::ImEnabled);
}

std::unique_ptr<qt::WebPopupMenu> ContentsView::CreateWebPopupMenu(
    const std::vector<qt::MenuItem>& items,
    bool allow_multiple_selection,
    const QRect& bounds,
    qt::WebPopupMenuClient* client) {
  return nullptr;
}

void ContentsView::HandleUnhandledKeyboardEvent(QKeyEvent* event) {
  QQuickWindow* w = item_->window();
  if (!w) {
    return;
  }

  Q_ASSERT(!handling_unhandled_key_event_);

  handling_unhandled_key_event_ = true;
  w->sendEvent(item_, event);
  handling_unhandled_key_event_ = false;
}

ContentsView::ContentsView(QQuickItem* item)
    : item_(item),
      received_new_compositor_frame_(false),
      frame_evicted_(false),
      last_composited_frame_type_(qt::CompositorFrameHandle::TYPE_INVALID),
      handling_unhandled_key_event_(false) {}

ContentsView::~ContentsView() {}

void ContentsView::init() {
  connect(item_, SIGNAL(windowChanged(QQuickWindow*)), SLOT(windowChanged()));
}

QVariant ContentsView::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImEnabled:
      return (item_->flags() & QQuickItem::ItemAcceptsInputMethod) != 0;
    default:
      return view() ? view()->inputMethodQuery(query) : QVariant();
  }
}

void ContentsView::handleItemChange(QQuickItem::ItemChange change) {
  if (!view()) {
    return;
  }

  if (change == QQuickItem::ItemVisibleHasChanged) {
    view()->visibilityChanged();
  } else if (change == QQuickItem::ItemActiveFocusHasChanged) {
    view()->activeFocusChanged();
  }
}

void ContentsView::windowChanged() {
  if (!view()) {
    return;
  }

  view()->windowChanged();
}

void ContentsView::handleKeyPressEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleKeyReleaseEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleInputMethodEvent(QInputMethodEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleInputMethodEvent(event);
}

void ContentsView::handleFocusInEvent(QFocusEvent* event) {
  handleFocusEvent(event);
}

void ContentsView::handleFocusOutEvent(QFocusEvent* event) {
  handleFocusEvent(event);
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

void ContentsView::handleTouchUngrabEvent() {
  if (!view()) {
    return;
  }

  view()->handleTouchUngrabEvent();
}

void ContentsView::handleWheelEvent(QWheelEvent* event) {
  if (!view()) {
    return;
  }

  QPointF window_pos = item_->mapToScene(event->posF());
  view()->handleWheelEvent(event, window_pos);
}

void ContentsView::handleTouchEvent(QTouchEvent* event) {
  if (!view()) {
    return;
  }

  if (event->type() == QEvent::TouchBegin) {
    item_->forceActiveFocus();
  }
  view()->handleTouchEvent(event);
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
  if (!view()) {
    return;
  }

  view()->handleDragEnterEvent(event);
}

void ContentsView::handleDragMoveEvent(QDragMoveEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleDragMoveEvent(event);
}

void ContentsView::handleDragLeaveEvent(QDragLeaveEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleDragLeaveEvent(event);
}

void ContentsView::handleDropEvent(QDropEvent* event) {
  if (!view()) {
    return;
  }

  view()->handleDropEvent(event);
}

void ContentsView::handleGeometryChanged() {
  if (!view()) {
    return;
  }

  if (!item_->window()) {
    return;
  }

  view()->wasResized();
}

QSGNode* ContentsView::updatePaintNode(QSGNode* old_node) {
  UpdatePaintNodeScope scope(this);

  qt::CompositorFrameHandle::Type type =
      qt::CompositorFrameHandle::TYPE_INVALID;
  QSharedPointer<qt::CompositorFrameHandle> handle;

  if (view()) {
    handle = view()->compositorFrameHandle();
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

  if (type == qt::CompositorFrameHandle::TYPE_ACCELERATED) {
    AcceleratedFrameNode* node = static_cast<AcceleratedFrameNode *>(old_node);
    if (!node) {
      node = new AcceleratedFrameNode(item_);
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == qt::CompositorFrameHandle::TYPE_IMAGE) {
    ImageFrameNode* node = static_cast<ImageFrameNode *>(old_node);
    if (!node) {
      node = new ImageFrameNode();
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  if (type == qt::CompositorFrameHandle::TYPE_SOFTWARE) {
    SoftwareFrameNode* node = static_cast<SoftwareFrameNode *>(old_node);
    if (!node) {
      node = new SoftwareFrameNode(item_);
    }

    if (received_new_compositor_frame_ || !old_node) {
      node->updateNode(handle);
    }

    return node;
  }

  Q_ASSERT(type == qt::CompositorFrameHandle::TYPE_INVALID);

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
