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

#include <QPointF>

#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/vector2d_f.h"

#include "qt/core/glue/oxide_qt_touch_handle_drawable_proxy.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

TouchHandleDrawable::TouchHandleDrawable(const ContentsView* view)
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
    gfx::PointF o = origin;
    o += gfx::Vector2dF(0, view_->GetLocationBarContentOffset());
    gfx::PointF scaled_origin =
        DpiUtils::ConvertChromiumPixelsToQt(o, view_->GetScreen());
    proxy_->SetOrigin(ToQt(scaled_origin));
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

  gfx::RectF bounds =
      DpiUtils::ConvertQtPixelsToChromium(
        ToChromium(proxy_->GetVisibleBounds()),
        view_->GetScreen());
  bounds += gfx::Vector2dF(0, -view_->GetLocationBarContentOffset());

  return bounds;
}

float TouchHandleDrawable::GetDrawableHorizontalPaddingRatio() const {
  if (!proxy_) {
    return 0.0f;
  }

  return proxy_->GetDrawableHorizontalPaddingRatio();
}

} // namespace qt
} // namespace oxide
