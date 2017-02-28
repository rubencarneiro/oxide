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

#include "touch_handle_drawable_host.h"

#include "ui/touch_selection/touch_handle_orientation.h"

#include "qt/core/browser/contents_view_impl.h"
#include "qt/core/browser/oxide_qt_dpi_utils.h"
#include "qt/core/browser/oxide_qt_type_conversions.h"
#include "qt/core/glue/touch_handle_drawable.h"

namespace oxide {
namespace qt {

void TouchHandleDrawableHost::SetEnabled(bool enabled) {
  drawable_->SetEnabled(enabled);
}

void TouchHandleDrawableHost::SetOrientation(
    ui::TouchHandleOrientation orientation,
    bool mirror_vertical,
    bool mirror_horizontal) {
  drawable_->SetOrientation(
      static_cast<qt::TouchHandleDrawable::Orientation>(orientation),
      mirror_vertical, mirror_horizontal);
}

void TouchHandleDrawableHost::SetOrigin(const gfx::PointF& origin) {
  drawable_->SetOrigin(
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(origin, view_->GetScreen())));
}

void TouchHandleDrawableHost::SetAlpha(float alpha) {
  drawable_->SetAlpha(alpha);
}

gfx::RectF TouchHandleDrawableHost::GetVisibleBounds() const {
  return DpiUtils::ConvertQtPixelsToChromium(
      ToChromium(drawable_->GetVisibleBounds()), view_->GetScreen());
}

float TouchHandleDrawableHost::GetDrawableHorizontalPaddingRatio() const {
  return drawable_->GetDrawableHorizontalPaddingRatio();
}

TouchHandleDrawableHost::TouchHandleDrawableHost(const ContentsViewImpl* view)
    : view_(view) {}

TouchHandleDrawableHost::~TouchHandleDrawableHost() = default;

void TouchHandleDrawableHost::Init(
    std::unique_ptr<qt::TouchHandleDrawable> drawable) {
  drawable_ = std::move(drawable);
}

} // namespace qt
} // namespace oxide
