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

#include "touch_handle_drawable_host.h"

#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/vector2d_f.h"

#include "shared/browser/chrome_controller.h"

namespace oxide {

void TouchHandleDrawableHost::UpdatePosition() {
  gfx::Vector2dF offset(0, controller()->GetTopContentOffset());
  drawable_->SetOrigin(origin_ + offset);
}

void TouchHandleDrawableHost::SetEnabled(bool enabled) {
  drawable_->SetEnabled(enabled);
}

void TouchHandleDrawableHost::SetOrientation(
    ui::TouchHandleOrientation orientation,
    bool mirror_vertical,
    bool mirror_horizontal) {
  drawable_->SetOrientation(orientation, mirror_vertical, mirror_horizontal);
}

void TouchHandleDrawableHost::SetOrigin(const gfx::PointF& origin) {
  origin_ = origin;
  UpdatePosition();
}

void TouchHandleDrawableHost::SetAlpha(float alpha) {
  drawable_->SetAlpha(alpha);
}

gfx::RectF TouchHandleDrawableHost::GetVisibleBounds() const {
  gfx::Vector2dF offset(0, controller()->GetTopContentOffset());
  return drawable_->GetVisibleBounds() - offset;
}

float TouchHandleDrawableHost::GetDrawableHorizontalPaddingRatio() const {
  return drawable_->GetDrawableHorizontalPaddingRatio();
}

void TouchHandleDrawableHost::ContentOrTopControlsOffsetChanged() {
  UpdatePosition();
}

TouchHandleDrawableHost::TouchHandleDrawableHost(
    ChromeController* chrome_controller)
    : ChromeControllerObserver(chrome_controller) {}

TouchHandleDrawableHost::~TouchHandleDrawableHost() = default;

void TouchHandleDrawableHost::Init(
    std::unique_ptr<ui::TouchHandleDrawable> drawable) {
  drawable_ = std::move(drawable);
}

} // namespace oxide
