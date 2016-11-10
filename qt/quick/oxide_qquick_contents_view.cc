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

#include <QGuiApplication>
#include <QHoverEvent>
#include <QPointF>
#include <QQuickWindow>
#include <QScreen>
#include <QTouchEvent>

#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"

#include "oxide_qquick_accelerated_frame_node.h"
#include "oxide_qquick_image_frame_node.h"
#include "oxide_qquick_software_frame_node.h"
#include "oxide_qquick_touch_handle_drawable.h"
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

  if (handling_unhandled_key_event_) {
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
                            window_pos,
                            (window_pos + item_->window()->position()).toPoint());
}

void ContentsView::handleFocusEvent(QFocusEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleFocusEvent(event);
}

void ContentsView::didUpdatePaintNode() {
  if (received_new_compositor_frame_) {
    received_new_compositor_frame_ = false;
    proxy()->didCommitCompositorFrame();
  }
}

QWindow* ContentsView::GetWindow() const {
  return item_->window();
}

bool ContentsView::IsVisible() const {
  return item_->isVisible();
}

bool ContentsView::HasFocus() const {
  QQuickWindow* window = item_->window();
  return window ? (window->isActive() &&
      (window->activeFocusItem() == item_)) : false;
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

std::unique_ptr<oxide::qt::WebPopupMenuProxy> ContentsView::CreateWebPopupMenu(
    const std::vector<oxide::qt::MenuItem>& items,
    bool allow_multiple_selection,
    oxide::qt::WebPopupMenuProxyClient* client) {
  return std::unique_ptr<oxide::qt::WebPopupMenuProxy>(
      new WebPopupMenu(item_,
                       popup_menu_,
                       items,
                       allow_multiple_selection,
                       client));
}

oxide::qt::TouchHandleDrawableProxy*
ContentsView::CreateTouchHandleDrawable() {
  return new TouchHandleDrawable(item_, touch_selection_controller_.data());
}

void ContentsView::TouchSelectionChanged(
    oxide::qt::TouchSelectionControllerActiveStatus status,
    const QRectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) {
  if (touch_selection_controller_) {
    touch_selection_controller_->onTouchSelectionChanged(
        static_cast<OxideQQuickTouchSelectionController::Status>(status),
        bounds,
        handle_drag_in_progress,
        insertion_handle_tapped);
  }
}

void ContentsView::ContextMenuIntercepted() const {
  if (touch_selection_controller_) {
    Q_EMIT touch_selection_controller_->contextMenuIntercepted();
  }
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
      touch_selection_controller_(
          new OxideQQuickTouchSelectionController(this)),
      received_new_compositor_frame_(false),
      frame_evicted_(false),
      last_composited_frame_type_(
          oxide::qt::CompositorFrameHandle::TYPE_INVALID),
      handling_unhandled_key_event_(false) {
  connect(item_, SIGNAL(windowChanged(QQuickWindow*)), SLOT(windowChanged()));
}

ContentsView::~ContentsView() {}

QVariant ContentsView::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImEnabled:
      return (item_->flags() & QQuickItem::ItemAcceptsInputMethod) != 0;
    default:
      return proxy() ? proxy()->inputMethodQuery(query) : QVariant();
  }
}

void ContentsView::handleItemChange(QQuickItem::ItemChange change) {
  if (!proxy()) {
    return;
  }

  if (change == QQuickItem::ItemVisibleHasChanged) {
    proxy()->visibilityChanged();
  }
}

void ContentsView::windowChanged() {
  if (!proxy()) {
    return;
  }

  proxy()->windowChanged();
}

void ContentsView::handleKeyPressEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleKeyReleaseEvent(QKeyEvent* event) {
  handleKeyEvent(event);
}

void ContentsView::handleInputMethodEvent(QInputMethodEvent* event) {
  if (!proxy()) {
    return;
  }

  proxy()->handleInputMethodEvent(event);
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
  if (!proxy()) {
    return;
  }

  proxy()->handleTouchUngrabEvent();
}

void ContentsView::handleWheelEvent(QWheelEvent* event) {
  if (!proxy()) {
    return;
  }

  QPointF window_pos = item_->mapToScene(event->posF());
  proxy()->handleWheelEvent(event, window_pos);
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

void ContentsView::handleGeometryChanged() {
  if (!proxy()) {
    return;
  }

  if (!item_->window()) {
    return;
  }

  proxy()->wasResized();
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

void ContentsView::hideTouchSelectionController() const {
  if (!proxy()) {
    return;
  }

  proxy()->hideTouchSelectionController();
}

} // namespace qquick
} // namespace oxide
