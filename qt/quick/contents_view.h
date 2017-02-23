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
#include <QtGlobal>

#include "qt/core/glue/contents_view.h"
#include "qt/core/glue/contents_view_client.h"

#include "qt/quick/api/oxideqquickglobal.h"

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
class QSGNode;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class OXIDE_QTQUICK_EXPORT ContentsView : public QObject,
                                          public qt::ContentsViewClient {
  Q_OBJECT

 public:
  ContentsView(QQuickItem* item);
  ~ContentsView() override;

  void init();

  QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

  void handleItemChange(QQuickItem::ItemChange change);
  void handleKeyPressEvent(QKeyEvent* event);
  void handleKeyReleaseEvent(QKeyEvent* event);
  void handleInputMethodEvent(QInputMethodEvent* event);
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

 protected:
  QQuickItem* item() const { return item_; }

 private Q_SLOTS:
  void windowChanged();

 private:
  friend class UpdatePaintNodeScope;

  void handleKeyEvent(QKeyEvent* event);
  void handleMouseEvent(QMouseEvent* event);
  void handleHoverEvent(QHoverEvent* event);

  void didUpdatePaintNode();

  // qt::ContentsViewClient implementation
  QWindow* GetWindow() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  QRect GetBounds() const override;
  void ScheduleUpdate() override;
  void EvictCurrentFrame() override;
  void UpdateCursor(const QCursor& cursor) override;
  void SetInputMethodAccepted(bool accepted) override;
  std::unique_ptr<qt::WebPopupMenu> CreateWebPopupMenu(
      const std::vector<qt::MenuItem>& items,
      bool allow_multiple_selection,
      const QRect& bounds,
      qt::WebPopupMenuClient* client) override;
  void HandleUnhandledKeyboardEvent(QKeyEvent* event) override;

  QQuickItem* item_;

  QPointer<QQmlComponent> touch_handle_;

  bool received_new_compositor_frame_;
  bool frame_evicted_;
  qt::CompositorFrameHandle::Type last_composited_frame_type_;

  bool handling_unhandled_key_event_;

  Q_DISABLE_COPY(ContentsView)
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_CONTENTS_VIEW_H_
