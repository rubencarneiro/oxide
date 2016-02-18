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

#include <QPointer>
#include <QtGlobal>

#include "qt/core/browser/oxide_qt_event_utils.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy.h"
#include "shared/browser/oxide_web_contents_view_client.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace content {
class WebContents;
}

namespace oxide {
namespace qt {

class ContentsViewProxyClient;

class ContentsView : public ContentsViewProxy,
                     public oxide::WebContentsViewClient {
 public:
  ContentsView(
      ContentsViewProxyClient* client,
      QObject* native_view,
      const base::Callback<float(void)>& location_bar_content_offset_getter);
  ~ContentsView() override;

  static ContentsView* FromWebContents(content::WebContents* contents);

  QObject* native_view() const { return native_view_; }

  // TODO: Get rid
  ContentsViewProxyClient* client() const { return client_; }

 private:
  float GetDeviceScaleFactor() const;
  float GetLocationBarContentOffsetDip();

  // ContentsViewProxy implementation
  void handleKeyEvent(QKeyEvent* event) override;
  void handleMouseEvent(QMouseEvent* event) override;
  void handleHoverEvent(QHoverEvent* event,
                        const QPoint& window_pos,
                        const QPoint& global_pos) override;
  void handleTouchEvent(QTouchEvent* event) override;
  void handleWheelEvent(QWheelEvent* event, const QPoint& window_pos) override;
  void handleDragEnterEvent(QDragEnterEvent* event) override;
  void handleDragMoveEvent(QDragMoveEvent* event) override;
  void handleDragLeaveEvent(QDragLeaveEvent* event) override;
  void handleDropEvent(QDropEvent* event) override;

  // oxide::WebContentsViewClient implementation
  blink::WebScreenInfo GetScreenInfo() const override;
  gfx::Rect GetBoundsPix() const override;
  oxide::WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params) override;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) override;

  ContentsViewProxyClient* client_;

  QPointer<QObject> native_view_;

  // TODO: This is because location bar control is currently part of webview.
  //  We should split location bar control in to its own component, and also
  //  make the transform in WebContentsView instead. Then we can remove this
  //  hack
  base::Callback<float(void)> location_bar_content_offset_getter_;

  UITouchEventFactory touch_event_factory_;

  DISALLOW_COPY_AND_ASSIGN(ContentsView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENTS_VIEW_H_
