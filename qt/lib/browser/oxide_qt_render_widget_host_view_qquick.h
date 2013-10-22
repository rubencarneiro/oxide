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

#ifndef _OXIDE_QT_LIB_BROWSER_RENDER_WIDGET_HOST_VIEW_QQUICK_H_
#define _OXIDE_QT_LIB_BROWSER_RENDER_WIDGET_HOST_VIEW_QQUICK_H_

#include <QQuickPaintedItem>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_render_widget_host_view.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
class QScreen;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

class BackingStore;

class RenderWidgetHostViewQQuick FINAL : public QQuickPaintedItem,
                                         public oxide::RenderWidgetHostView {
 public:
  RenderWidgetHostViewQQuick(content::RenderWidgetHost* render_widget_host,
                             QQuickItem* container);
  virtual ~RenderWidgetHostViewQQuick();

  static void GetScreenInfo(QScreen* screen, WebKit::WebScreenInfo* result);

  void Blur() FINAL;
  void Focus() FINAL;
  bool HasFocus() const FINAL;

  void Show() FINAL;
  void Hide() FINAL;
  bool IsShowing() FINAL;

  gfx::Rect GetViewBounds() const FINAL;

  content::BackingStore* AllocBackingStore(const gfx::Size& size) FINAL;

  void GetScreenInfo(WebKit::WebScreenInfo* results) FINAL;

  gfx::Rect GetBoundsInRootWindow() FINAL;

  void focusInEvent(QFocusEvent* event) FINAL;
  void focusOutEvent(QFocusEvent* event) FINAL;

  void keyPressEvent(QKeyEvent* event) FINAL;
  void keyReleaseEvent(QKeyEvent* event) FINAL;

  void mouseDoubleClickEvent(QMouseEvent* event) FINAL;
  void mouseMoveEvent(QMouseEvent* event) FINAL;
  void mousePressEvent(QMouseEvent* event) FINAL;
  void mouseReleaseEvent(QMouseEvent* event) FINAL;

  void wheelEvent(QWheelEvent* event) FINAL;

  void hoverMoveEvent(QHoverEvent* event) FINAL;

  void updatePolish() FINAL;
  void paint(QPainter* paint) FINAL;

 private:
  void ScheduleUpdate(const gfx::Rect& rect) FINAL;

  BackingStore* backing_store_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostViewQQuick);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_BROWSER_RENDER_WIDGET_HOST_VIEW_QQUICK_H_
