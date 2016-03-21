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

#ifndef _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_
#define _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include <QPointer>
#include <QSharedPointer>
#include <QtGlobal>

#include "qt/core/browser/input/oxide_qt_input_method_context_client.h"
#include "qt/core/browser/oxide_qt_event_utils.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy.h"
#include "shared/browser/oxide_web_contents_view_client.h"

QT_BEGIN_NAMESPACE
class QObject;
class QScreen;
QT_END_NAMESPACE

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

class CompositorFrameHandle;
class ContentsViewProxyClient;
class InputMethodContext;

class ContentsView : public ContentsViewProxy,
                     public InputMethodContextClient,
                     public oxide::WebContentsViewClient {
 public:
  ContentsView(ContentsViewProxyClient* client,
               QObject* native_view);
  ~ContentsView() override;

  static ContentsView* FromWebContents(content::WebContents* contents);

  QObject* native_view() const { return native_view_; }

  QScreen* GetScreen() const;

  // TODO: Get rid
  ContentsViewProxyClient* client() const { return client_; }

  float GetLocationBarContentOffset() const;

 private:
  // ContentsViewProxy implementation
  QSharedPointer<CompositorFrameHandle> compositorFrameHandle() override;
  void didCommitCompositorFrame() override;
  void wasResized() override;
  void visibilityChanged() override;
  void screenUpdated() override;
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
  void handleKeyEvent(QKeyEvent* event) override;
  void handleInputMethodEvent(QInputMethodEvent* event) override;
  void handleFocusEvent(QFocusEvent* event) override;
  void handleMouseEvent(QMouseEvent* event) override;
  void handleTouchUngrabEvent() override;
  void handleWheelEvent(QWheelEvent* event,
                        const QPointF& window_pos) override;
  void handleTouchEvent(QTouchEvent* event) override;
  void handleHoverEvent(QHoverEvent* event,
                        const QPointF& window_pos,
                        const QPoint& global_pos) override;
  void handleDragEnterEvent(QDragEnterEvent* event) override;
  void handleDragMoveEvent(QDragMoveEvent* event) override;
  void handleDragLeaveEvent(QDragLeaveEvent* event) override;
  void handleDropEvent(QDropEvent* event) override;
  void hideTouchSelectionController() override;

  // InputMethodContextClient implementation
  void SetInputMethodEnabled(bool enabled);

  // oxide::WebContentsViewClient implementation
  blink::WebScreenInfo GetScreenInfo() const override;
  bool IsVisible() const override;
  bool HasFocus() const override;
  gfx::RectF GetBounds() const override;
  void SwapCompositorFrame() override;
  void EvictCurrentFrame() override;
  void UpdateCursor(const content::WebCursor& cursor) override;
  oxide::WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params) override;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) override;
  ui::TouchHandleDrawable* CreateTouchHandleDrawable() const override;
  void TouchSelectionChanged(bool active,
                             const gfx::RectF& bounds,
                             bool handle_drag_in_progress) const override;
  oxide::InputMethodContext* GetInputMethodContext() const override;
  void UnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) override;

  ContentsViewProxyClient* client_;

  QPointer<QObject> native_view_;

  UITouchEventFactory touch_event_factory_;

  QSharedPointer<CompositorFrameHandle> compositor_frame_;

  scoped_ptr<InputMethodContext> input_method_context_;

  DISALLOW_COPY_AND_ASSIGN(ContentsView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_
