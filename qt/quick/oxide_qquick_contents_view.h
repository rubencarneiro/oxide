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

#ifndef _OXIDE_QQUICK_CONTENTS_VIEW_H_
#define _OXIDE_QQUICK_CONTENTS_VIEW_H_

#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QScopedPointer>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_contents_view_proxy.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QHoverEvent;
class QKeyEvent;
class QMouseEvent;
class QQmlComponent;
class QQuickItem;
class QQuickWindow;
class QScreen;
class QSGNode;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQQuickTouchSelectionController;

namespace oxide {
namespace qquick {

class ContentsView : public QObject,
                     public oxide::qt::ContentsViewProxyClient {
  Q_OBJECT

 public:
  ContentsView(QQuickItem* item);
  ~ContentsView() override;

  QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

  void handleItemChange(QQuickItem::ItemChange change);
  void handleKeyPressEvent(QKeyEvent* event);
  void handleKeyReleaseEvent(QKeyEvent* event);
  void handleInputMethodEvent(QInputMethodEvent* event);
  void handleFocusInEvent(QFocusEvent* event);
  void handleFocusOutEvent(QFocusEvent* event);
  void handleMousePressEvent(QMouseEvent* event);
  void handleMouseMoveEvent(QMouseEvent* event);
  void handleMouseReleaseEvent(QMouseEvent* event);
  void handleTouchUngrabEvent();
  void handleWheelEvent(QWheelEvent* event);
  void handleTouchEvent(QTouchEvent* event);
  void handleHoverEnterEvent(QHoverEvent* event);
  void handleHoverMoveEvent(QHoverEvent* event);
  void handleHoverLeaveEvent(QHoverEvent* event);
  void handleDragEnterEvent(QDragEnterEvent* event);
  void handleDragMoveEvent(QDragMoveEvent* event);
  void handleDragLeaveEvent(QDragLeaveEvent* event);
  void handleDropEvent(QDropEvent* event);
  void handleGeometryChanged();

  QSGNode* updatePaintNode(QSGNode* old_node);

  // XXX(chrisccoulson): Remove these UI component accessors from here in
  //  future. Instead, we should have an auxilliary UI client class. There'll
  //  be a WebView impl of this for the custom UI component APIs, and also an
  //  Ubuntu UI Toolkit impl
  QQmlComponent* contextMenu() const { return context_menu_; }
  void setContextMenu(QQmlComponent* context_menu) {
    context_menu_ = context_menu;
  }

  QQmlComponent* popupMenu() const { return popup_menu_; }
  void setPopupMenu(QQmlComponent* popup_menu) {
    popup_menu_ = popup_menu;
  }

  OxideQQuickTouchSelectionController* touchSelectionController() const {
    return touch_selection_controller_.data();
  }

  void hideTouchSelectionController() const;

 private Q_SLOTS:
  void windowChanged(QQuickWindow* window);
  void screenChanged(QScreen* screen);
  void screenGeometryChanged(const QRect& geometry);
  void screenOrientationChanged(Qt::ScreenOrientation orientation);

 private:
  friend class UpdatePaintNodeScope;

  void screenChangedHelper(QScreen* screen);

  void handleKeyEvent(QKeyEvent* event);
  void handleMouseEvent(QMouseEvent* event);
  void handleHoverEvent(QHoverEvent* event);
  void handleFocusEvent(QFocusEvent* event);

  void didUpdatePaintNode();

  // oxide::qt::ContentsViewProxyClient implementation
  QScreen* GetScreen() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  QRect GetBounds() const override;
  void ScheduleUpdate() override;
  void EvictCurrentFrame() override;
  void UpdateCursor(const QCursor& cursor) override;
  void SetInputMethodEnabled(bool enabled) override;
  oxide::qt::WebContextMenuProxy* CreateWebContextMenu(
      oxide::qt::WebContextMenuProxyClient* client) override;
  std::unique_ptr<oxide::qt::WebPopupMenuProxy> CreateWebPopupMenu(
      const QList<oxide::qt::MenuItem>& items,
      bool allow_multiple_selection,
      oxide::qt::WebPopupMenuProxyClient* client) override;
  oxide::qt::TouchHandleDrawableProxy* CreateTouchHandleDrawable() override;
  void TouchSelectionChanged(bool active,
                             const QRectF& bounds,
                             bool handle_drag_in_progress) override;
  void HandleUnhandledKeyboardEvent(QKeyEvent* event) override;

  QPointer<QQuickItem> item_;
  QScopedPointer<OxideQQuickTouchSelectionController> touch_selection_controller_;

  QPointer<QQuickWindow> window_;
  QPointer<QScreen> screen_;

  QPointer<QQmlComponent> context_menu_;
  QPointer<QQmlComponent> popup_menu_;
  QPointer<QQmlComponent> touch_handle_;

  bool received_new_compositor_frame_;
  bool frame_evicted_;
  oxide::qt::CompositorFrameHandle::Type last_composited_frame_type_;

  bool handling_unhandled_key_event_;

  Q_DISABLE_COPY(ContentsView)
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_CONTENTS_VIEW_H_
