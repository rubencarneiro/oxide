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

#ifndef _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_IMPL_H_
#define _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_IMPL_H_

#include <memory>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QtGlobal>

#include "base/callback.h"
#include "base/macros.h"

#include "qt/core/browser/input/input_method_context_owner_client.h"
#include "qt/core/browser/oxide_qt_motion_event_factory.h"
#include "qt/core/glue/contents_view.h"
#include "shared/browser/oxide_web_contents_view_client.h"

QT_BEGIN_NAMESPACE
class QScreen;
class QWindow;
QT_END_NAMESPACE

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

class CompositorFrameHandle;
class ContentsViewClient;
class InputMethodContext;
class LegacyTouchEditingClientProxy;

class ContentsViewImpl : public QObject,
                         public ContentsView,
                         public InputMethodContextOwnerClient,
                         public oxide::WebContentsViewClient {
  Q_OBJECT

 public:
  ContentsViewImpl(ContentsViewClient* client,
                   QObject* native_view);
  ~ContentsViewImpl() override;

  static ContentsViewImpl* FromWebContents(content::WebContents* contents);

  content::WebContents* GetWebContents() const;

  QObject* native_view() const { return native_view_; }

  display::Display GetDisplay() const override;
  QScreen* GetScreen() const;

  ContentsViewClient* client() const { return client_; }

 private:
  // XXX(chrisccoulson): This is going to be removed, please don't use it
  //  See https://launchpad.net/bugs/1665722
  float GetTopContentOffset() const;

  // ContentsView implementation
  QSharedPointer<CompositorFrameHandle> compositorFrameHandle() override;
  void didCommitCompositorFrame() override;
  void windowChanged() override;
  void wasResized() override;
  void visibilityChanged() override;
  void activeFocusChanged() override;
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
  void handleKeyEvent(QKeyEvent* event) override;
  void handleInputMethodEvent(QInputMethodEvent* event) override;
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

  // InputMethodContextOwnerClient implementation
  void SetInputMethodAccepted(bool accepted) override;

  // oxide::WebContentsViewClient implementation
  bool IsVisible() const override;
  bool HasFocus() const override;
  gfx::RectF GetBounds() const override;
  void SwapCompositorFrame() override;
  void EvictCurrentFrame() override;
  void UpdateCursor(const content::WebCursor& cursor) override;
  std::unique_ptr<oxide::WebPopupMenu> CreatePopupMenu(
      const std::vector<content::MenuItem> & items,
      int selected_item,
      bool allow_multiple_selection,
      const gfx::Rect& bounds,
      oxide::WebPopupMenuClient* client) override;
  std::unique_ptr<ui::TouchHandleDrawable>
  CreateTouchHandleDrawable() const override;
  oxide::InputMethodContext* GetInputMethodContext() const override;
  oxide::LegacyTouchEditingClient* GetLegacyTouchEditingClient() const override;
  void UnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) override;

 private Q_SLOTS:
  void OnScreenChanged();

 private:
  ContentsViewClient* client_;

  QPointer<QObject> native_view_;

  QPointer<QWindow> window_;

  MotionEventFactory motion_event_factory_;

  QSharedPointer<CompositorFrameHandle> compositor_frame_;

  std::unique_ptr<InputMethodContext> input_method_context_;

  std::unique_ptr<LegacyTouchEditingClientProxy> legacy_touch_editing_client_;

  DISALLOW_COPY_AND_ASSIGN(ContentsViewImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_IMPL_H_
