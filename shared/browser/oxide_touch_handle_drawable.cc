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

#include "oxide_touch_handle_drawable.h"

#include "base/logging.h"
#include "content/public/browser/render_view_host.h"

#include "oxide_render_widget_host_view.h"
#include "oxide_touch_handle_drawable_delegate.h"
#include "oxide_web_view.h"

namespace oxide {

TouchHandleDrawable::TouchHandleDrawable(RenderWidgetHostView* rwhv) {
  content::RenderWidgetHost* rwh = rwhv->GetRenderWidgetHost();
  content::RenderViewHost* rvh = content::RenderViewHost::From(rwh);
  WebView* webview = WebView::FromRenderViewHost(rvh);
  if (!webview) {
    return;
  }
  delegate_.reset(webview->CreateTouchHandleDrawableDelegate());
}

TouchHandleDrawable::~TouchHandleDrawable() {}

void TouchHandleDrawable::SetEnabled(bool enabled) {
  if (delegate_) {
    delegate_->SetEnabled(enabled);
  }
}

void TouchHandleDrawable::SetOrientation(ui::TouchHandleOrientation orientation,
                                         bool mirror_vertical,
                                         bool mirror_horizontal) {
  if (delegate_) {
    delegate_->SetOrientation(orientation, mirror_vertical, mirror_horizontal);
  }
}

void TouchHandleDrawable::SetOrigin(const gfx::PointF& origin) {
  if (delegate_) {
    delegate_->SetOrigin(origin);
  }
}

void TouchHandleDrawable::SetAlpha(float alpha) {
  if (delegate_) {
    delegate_->SetAlpha(alpha);
  }
}

gfx::RectF TouchHandleDrawable::GetVisibleBounds() const {
  if (delegate_) {
    return delegate_->GetVisibleBounds();
  }
  return gfx::RectF();
}

float TouchHandleDrawable::GetDrawableHorizontalPaddingRatio() const {
  if (delegate_) {
    return delegate_->GetDrawableHorizontalPaddingRatio();
  }
  return 0.0f;
}

} // namespace oxide
