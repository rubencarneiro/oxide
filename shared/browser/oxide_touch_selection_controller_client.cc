// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_touch_selection_controller_client.h"

#include "base/logging.h"
#include "content/browser/renderer_host/render_widget_host_delegate.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/render_view_host.h"
#include "ui/gfx/geometry/point_conversions.h"

#include "oxide_render_widget_host_view.h"
#include "oxide_touch_handle_drawable.h"
#include "oxide_web_view.h"

namespace oxide {

TouchSelectionControllerClient::TouchSelectionControllerClient(
    RenderWidgetHostView* rwhv) 
    : rwhv_(rwhv) {
  DCHECK(rwhv_);
}

TouchSelectionControllerClient::~TouchSelectionControllerClient() {}

void TouchSelectionControllerClient::OnScrollStarted() {
  // XXX: currently commented out because when doing a pinch-to-zoom gesture,
  // we don’t always get the corresponding GestureScrollEnd event, so
  // selection handles would remain hidden.
  //rwhv_->selection_controller()->SetTemporarilyHidden(true);
}

void TouchSelectionControllerClient::OnScrollCompleted() {
  // XXX: see above
  //rwhv_->selection_controller()->SetTemporarilyHidden(false);
}

bool TouchSelectionControllerClient::SupportsAnimation() const {
  return false;
}

void TouchSelectionControllerClient::SetNeedsAnimate() {
  NOTREACHED();
}

void TouchSelectionControllerClient::MoveCaret(const gfx::PointF& position) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(rwhv_->GetRenderWidgetHost());
  rwhi->MoveCaret(gfx::ToRoundedPoint(position));
}

void TouchSelectionControllerClient::MoveRangeSelectionExtent(
    const gfx::PointF& extent) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(rwhv_->GetRenderWidgetHost());
  content::RenderWidgetHostDelegate* host_delegate = rwhi->delegate();
  if (host_delegate) {
    host_delegate->MoveRangeSelectionExtent(gfx::ToRoundedPoint(extent));
  }
}

void TouchSelectionControllerClient::SelectBetweenCoordinates(
    const gfx::PointF& base, const gfx::PointF& extent) {
  content::RenderWidgetHostImpl* rwhi =
      content::RenderWidgetHostImpl::From(rwhv_->GetRenderWidgetHost());
  content::RenderWidgetHostDelegate* host_delegate = rwhi->delegate();
  if (host_delegate) {
    host_delegate->SelectRange(gfx::ToRoundedPoint(base),
                               gfx::ToRoundedPoint(extent));
  }
}

void TouchSelectionControllerClient::OnSelectionEvent(
    ui::SelectionEventType event) {
  content::RenderWidgetHost* rwh = rwhv_->GetRenderWidgetHost();
  content::RenderViewHost* rvh = content::RenderViewHost::From(rwh);
  if (!rvh) {
    return;
  }
  WebView* webview = WebView::FromRenderViewHost(rvh);
  if (webview) {
    webview->TouchSelectionChanged();
  }
}

scoped_ptr<ui::TouchHandleDrawable> TouchSelectionControllerClient::CreateDrawable() {
  return scoped_ptr<ui::TouchHandleDrawable>(new TouchHandleDrawable(rwhv_));
}

} // namespace oxide
