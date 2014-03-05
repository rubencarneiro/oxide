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
#include <QRect>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"

QT_BEGIN_NAMESPACE
class QPixmap;
QT_END_NAMESPACE

class OxideQQuickWebView;

namespace oxide {
namespace qquick {

class RenderViewItem Q_DECL_FINAL :
    public QQuickItem,
    public oxide::qt::RenderWidgetHostViewDelegate {
  Q_OBJECT

 public:
  RenderViewItem(OxideQQuickWebView* webview);

  void Blur() Q_DECL_FINAL;
  void Focus() Q_DECL_FINAL;
  bool HasFocus() Q_DECL_FINAL;

  void Show() Q_DECL_FINAL;
  void Hide() Q_DECL_FINAL;
  bool IsShowing() Q_DECL_FINAL;

  QRect GetViewBoundsPix() Q_DECL_FINAL;

  void SetSize(const QSize& size) Q_DECL_FINAL;

  QScreen* GetScreen() Q_DECL_FINAL;

  void SetInputMethodEnabled(bool enabled) Q_DECL_FINAL;

  void SchedulePaintForRectPix(const QRect& rect) Q_DECL_FINAL;
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
  const QPixmap* backing_store_;
#if defined(ENABLE_COMPOSITING)
  oxide::qt::TextureInfo texture_info_;
#endif
  QRect dirty_rect_;

#if defined(ENABLE_COMPOSITING)
  bool is_compositing_enabled_;
  bool is_compositing_enabled_state_changed_;
#endif

  Q_DISABLE_COPY(RenderViewItem);
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_RENDER_VIEW_ITEM_H_
