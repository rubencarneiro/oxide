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

#ifndef _OXIDE_QT_QUICK_RENDER_VIEW_ITEM_H_
#define _OXIDE_QT_QUICK_RENDER_VIEW_ITEM_H_

#include <QQuickItem>
#include <QCursor>
#include <QImage>
#include <QRect>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"

namespace oxide {

namespace qt {
class WebViewAdapter;
}

namespace qquick {

class RenderViewItem Q_DECL_FINAL :
    public QQuickItem,
    public oxide::qt::RenderWidgetHostViewDelegate {
  Q_OBJECT

 public:
  RenderViewItem();

  void Init(oxide::qt::WebViewAdapter* view) Q_DECL_FINAL;

  void Blur() Q_DECL_FINAL;
  void Focus() Q_DECL_FINAL;
  bool HasFocus() Q_DECL_FINAL;

  void Show() Q_DECL_FINAL;
  void Hide() Q_DECL_FINAL;
  bool IsShowing() Q_DECL_FINAL;

  void UpdateCursor(const QCursor& cursor) Q_DECL_FINAL;

  QRect GetViewBoundsPix() Q_DECL_FINAL;

  void SetSize(const QSize& size) Q_DECL_FINAL;

  QScreen* GetScreen() Q_DECL_FINAL;

  void SetInputMethodEnabled(bool enabled) Q_DECL_FINAL;

  void ScheduleUpdate() Q_DECL_FINAL;

  void focusInEvent(QFocusEvent* event) Q_DECL_FINAL;
  void focusOutEvent(QFocusEvent* event) Q_DECL_FINAL;

  void keyPressEvent(QKeyEvent* event) Q_DECL_FINAL;
  void keyReleaseEvent(QKeyEvent* event) Q_DECL_FINAL;

  void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mousePressEvent(QMouseEvent* event) Q_DECL_FINAL;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_FINAL;

  void wheelEvent(QWheelEvent* event) Q_DECL_FINAL;

  void hoverMoveEvent(QHoverEvent* event) Q_DECL_FINAL;

  void inputMethodEvent(QInputMethodEvent* event) Q_DECL_FINAL;

  void touchEvent(QTouchEvent * event) Q_DECL_FINAL;

  void updatePolish() Q_DECL_FINAL;
  QSGNode* updatePaintNode(QSGNode* oldNode,
                           UpdatePaintNodeData* data) Q_DECL_FINAL;

  QVariant inputMethodQuery(Qt::InputMethodQuery query) const Q_DECL_FINAL;

 private:
  friend class UpdatePaintNodeContext;

  void geometryChanged(const QRectF& new_geometry,
                       const QRectF& old_geometry) Q_DECL_FINAL;
  void DidUpdatePaintNode(oxide::qt::CompositorFrameType type);

  bool received_new_compositor_frame_;
  oxide::qt::CompositorFrameType last_composited_frame_type_;

  QImage software_frame_data_;
  oxide::qt::AcceleratedFrameTextureHandle accelerated_frame_data_;

  Q_DISABLE_COPY(RenderViewItem);
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_RENDER_VIEW_ITEM_H_
