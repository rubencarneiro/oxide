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

#ifndef _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
#define _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_H_

#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_render_widget_host_view.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPixmap;
class QScreen;
class QWheelEvent;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

class BackingStore;
class RenderWidgetHostViewDelegate;

class RenderWidgetHostView FINAL : public oxide::RenderWidgetHostView {
 public:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host,
                       RenderWidgetHostViewDelegate* delegate);
  virtual ~RenderWidgetHostView();

  static void GetScreenInfo(QScreen* screen, blink::WebScreenInfo* result);

  void Blur() FINAL;
  void Focus() FINAL;
  bool HasFocus() const FINAL;

  void Show() FINAL;
  void Hide() FINAL;
  bool IsShowing() FINAL;

  gfx::Rect GetViewBounds() const FINAL;

  void SetSize(const gfx::Size& size) FINAL;

  content::BackingStore* AllocBackingStore(const gfx::Size& size) FINAL;

  void GetScreenInfo(blink::WebScreenInfo* results) FINAL;

  gfx::Rect GetBoundsInRootWindow() FINAL;

  void ForwardFocusEvent(QFocusEvent* event);
  void ForwardKeyEvent(QKeyEvent* event);
  void ForwardMouseEvent(QMouseEvent* event);
  void ForwardWheelEvent(QWheelEvent* event);

  const QPixmap* GetBackingStore();

 private:
  void ScheduleUpdate(const gfx::Rect& rect) FINAL;

  BackingStore* backing_store_;
  scoped_ptr<RenderWidgetHostViewDelegate> delegate_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
