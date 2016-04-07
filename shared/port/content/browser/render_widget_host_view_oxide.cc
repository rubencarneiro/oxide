// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "render_widget_host_view_oxide.h"

#include "base/logging.h"
#include "content/common/view_messages.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

namespace content {

void RenderWidgetHostViewBase::GetDefaultScreenInfo(
    blink::WebScreenInfo* result) {
  RenderWidgetHostViewOxide::GetWebScreenInfoForDisplay(
      gfx::Screen::GetScreen()->GetPrimaryDisplay(),
      result);
}

void RenderWidgetHostViewOxide::SelectionBoundsChanged(
    const ViewHostMsg_SelectionBounds_Params& params) {
  OnSelectionBoundsChanged(params.anchor_rect,
                           params.focus_rect,
                           params.is_anchor_first);
}

RenderWidgetHostViewOxide::~RenderWidgetHostViewOxide() {}

// static
void RenderWidgetHostViewOxide::GetWebScreenInfoForDisplay(
    const gfx::Display& display,
    blink::WebScreenInfo* result) {
  result->deviceScaleFactor = display.device_scale_factor();
  result->depth = 24;
  result->depthPerComponent = 8;
  result->isMonochrome = false;
  result->rect = display.bounds();
  result->availableRect = display.work_area();

  // The rotation angle of gfx::Display is the clockwise screen rotation,
  // whereas the orientationAngle of blink::WebScreenInfo is the clockwise
  // content rotation
  result->orientationAngle = display.RotationAsDegree();
  if (result->orientationAngle == 90) {
    result->orientationAngle = 270;
  } else if (result->orientationAngle == 270) {
    result->orientationAngle = 90;
  }

  result->orientationType =
      RenderWidgetHostViewBase::GetOrientationTypeForMobile(display);
}

} // namespace content
