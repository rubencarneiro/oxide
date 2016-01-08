// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#include "oxide_qt_touch_handle_drawable.h"

#include "qt/core/glue/oxide_qt_touch_handle_drawable_proxy.h"

#include "oxide_qt_web_view.h"

#include <QPointF>

namespace oxide {
namespace qt {

TouchHandleDrawable::TouchHandleDrawable(const WebView* view)
    : view_(view) {}

TouchHandleDrawable::~TouchHandleDrawable() {}

void TouchHandleDrawable::SetProxy(TouchHandleDrawableProxy* proxy) {
  proxy_.reset(proxy);
}

void TouchHandleDrawable::SetEnabled(bool enabled) {
  if (proxy_) {
    proxy_->SetEnabled(enabled);
  }
}

void TouchHandleDrawable::SetOrientation(
    ui::TouchHandleOrientation orientation,
    bool mirror_vertical,
    bool mirror_horizontal) {
  if (proxy_) {
    TouchHandleDrawableProxy::Orientation o;
    switch (orientation) {
      case ui::TouchHandleOrientation::LEFT:
        o = TouchHandleDrawableProxy::Left;
        break;
      case ui::TouchHandleOrientation::CENTER:
        o = TouchHandleDrawableProxy::Center;
        break;
      case ui::TouchHandleOrientation::RIGHT:
        o = TouchHandleDrawableProxy::Right;
        break;
      case ui::TouchHandleOrientation::UNDEFINED:
        o = TouchHandleDrawableProxy::Undefined;
        break;
      default:
        Q_UNREACHABLE();
    }
    proxy_->SetOrientation(o, mirror_vertical, mirror_horizontal);
  }
}

void TouchHandleDrawable::SetOrigin(const gfx::PointF& origin) {
  if (proxy_) {
    const float dpr = view_->GetDeviceScaleFactor();
    proxy_->SetOrigin(QPointF(origin.x() * dpr, origin.y() * dpr));
  }
}

void TouchHandleDrawable::SetAlpha(float alpha) {
  if (proxy_) {
    proxy_->SetAlpha(alpha);
  }
}

gfx::RectF TouchHandleDrawable::GetVisibleBounds() const {
  if (!proxy_) {
    return gfx::RectF();
  }

  const float dpr = view_->GetDeviceScaleFactor();
  QRectF bounds = proxy_->GetVisibleBounds();
  return gfx::RectF(bounds.x() / dpr, bounds.y() / dpr,
                    bounds.width() / dpr, bounds.height() / dpr);
}

float TouchHandleDrawable::GetDrawableHorizontalPaddingRatio() const {
  if (!proxy_) {
    return 0.0f;
  }

  return proxy_->GetDrawableHorizontalPaddingRatio();
}

} // namespace qt
} // namespace oxide
