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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_HANDLE_DRAWABLE_HOST_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_HANDLE_DRAWABLE_HOST_H_

#include "cc/output/compositor_frame_metadata.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/touch_selection/touch_handle.h"

#include "shared/browser/oxide_web_contents_view.h"

namespace oxide {

class TouchHandleDrawableHost : public ui::TouchHandleDrawable {
 public:
  TouchHandleDrawableHost(WebContentsView* view);
  ~TouchHandleDrawableHost() override;

  void Init(std::unique_ptr<ui::TouchHandleDrawable> drawable);

 private:
  void OnSwapCompositorFrame(const CompositorFrameData* data,
                             const cc::CompositorFrameMetadata& metadata);
  void UpdatePosition();

  // ui::TouchHandleDrawable implementation
  void SetEnabled(bool enabled) override;
  void SetOrientation(ui::TouchHandleOrientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal) override;
  void SetOrigin(const gfx::PointF& origin) override;
  void SetAlpha(float alpha) override;
  gfx::RectF GetVisibleBounds() const override;
  float GetDrawableHorizontalPaddingRatio() const override;

  std::unique_ptr<WebContentsView::SwapCompositorFrameSubscription>
      swap_compositor_frame_subscription_;

  float content_offset_ = 0.f;
  gfx::PointF origin_;

  std::unique_ptr<ui::TouchHandleDrawable> drawable_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_HANDLE_DRAWABLE_HOST_H_
