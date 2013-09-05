// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_render_widget_host_view.h"

#include "base/callback.h"
#include "base/logging.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "ui/gfx/rect.h"

namespace oxide {

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    content::RenderWidgetHostViewBase(),
    is_hidden_(false),
    host_(content::RenderWidgetHostImpl::From(host)) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  host_->SetView(this);
}

RenderWidgetHostView::~RenderWidgetHostView() {}

void RenderWidgetHostView::InitAsPopup(
    content::RenderWidgetHostView* parent_host_view,
    const gfx::Rect& pos) {
  NOTIMPLEMENTED() <<
      "InitAsPopup() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::InitAsFullscreen(
    content::RenderWidgetHostView* reference_host_view) {
  NOTIMPLEMENTED() <<
      "InitAsFullScreen() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::WasShown() {
  if (!is_hidden_) {
    return;
  }

  is_hidden_ = false;

  GetRenderWidgetHostImpl()->WasShown();
}

void RenderWidgetHostView::WasHidden() {
  if (is_hidden_) {
    return;
  }

  is_hidden_ = true;

  GetRenderWidgetHostImpl()->WasHidden();
}

void RenderWidgetHostView::MovePluginWindows(
    const gfx::Vector2d& scroll_offset,
    const std::vector<content::WebPluginGeometry>& moves) {}

void RenderWidgetHostView::Blur() {}

void RenderWidgetHostView::UpdateCursor(const WebCursor& cursor) {}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {}

void RenderWidgetHostView::TextInputTypeChanged(ui::TextInputType type,
                                                bool can_compose_inline,
                                                ui::TextInputMode mode) {}

void RenderWidgetHostView::ImeCancelComposition() {}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const ui::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::DidUpdateBackingStore(
    const gfx::Rect& scroll_rect,
    const gfx::Vector2d& scroll_delta,
    const std::vector<gfx::Rect>& copy_rects,
    const ui::LatencyInfo& latency_info) {
  if (is_hidden_) {
    return;
  }

  ScheduleUpdate(scroll_rect);

  for (size_t i = 0; i < copy_rects.size(); ++i) {
    gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
    if (rect.IsEmpty()) {
      continue;
    }

    ScheduleUpdate(rect);
  }
}

void RenderWidgetHostView::RenderProcessGone(base::TerminationStatus status,
                                             int error_code) {
  Destroy();
}

void RenderWidgetHostView::Destroy() {
  host_ = NULL;
  delete this;
}

void RenderWidgetHostView::SetTooltipText(const string16& tooltip_text) {}

void RenderWidgetHostView::SelectionBoundsChanged(
    const ViewHostMsg_SelectionBounds_Params& params) {}

void RenderWidgetHostView::ScrollOffsetChanged() {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback) {
  GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
}

void RenderWidgetHostView::CopyFromCompositingSurfaceToVideoFrame(
    const gfx::Rect& src_subrect,
    const scoped_refptr<media::VideoFrame>& target,
    const base::Callback<void(bool)>& callback) {
  NOTIMPLEMENTED();
  callback.Run(false);
}

bool RenderWidgetHostView::CanCopyToVideoFrame() const {
  return false;
}

void RenderWidgetHostView::OnAcceleratedCompositingStateChange() {

}

void RenderWidgetHostView::AcceleratedSurfaceBuffersSwapped(
    const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
    int gpu_host_id) {}

void RenderWidgetHostView::AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) {}

void RenderWidgetHostView::AcceleratedSurfaceSuspend() {}

void RenderWidgetHostView::AcceleratedSurfaceRelease() {}

bool RenderWidgetHostView::HasAcceleratedSurface(
    const gfx::Size& desired_size) {
  return false;
}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  // XXX: This default implementation assumes a scale factor of 1.0
  //      Implementations that change this for DPI-awareness need
  //      to also set WebScreenInfo::deviceScaleFactor in
  //      GetScreenInfo()
  return GetViewBounds().size();
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface() {
  NOTIMPLEMENTED();
  return gfx::GLSurfaceHandle();
}

void RenderWidgetHostView::SetHasHorizontalScrollbar(
    bool has_horizontal_scrollbar) {}

void RenderWidgetHostView::SetScrollOffsetPinning(bool is_pinned_to_left,
                                                  bool is_pinned_to_right) {}

void RenderWidgetHostView::OnAccessibilityEvents(
    const std::vector<AccessibilityHostMsg_EventParams>& params) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  if (requested_size_.width() != size.width() ||
      requested_size_.height() != size.height()) {
    requested_size_ = size;

    GetRenderWidgetHostImpl()->SendScreenRects();
    GetRenderWidgetHost()->WasResized();
  }
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
}

gfx::NativeView RenderWidgetHostView::GetNativeView() const {
  return NULL;
}

gfx::NativeViewId RenderWidgetHostView::GetNativeViewId() const {
  return 0;
}

gfx::NativeViewAccessible RenderWidgetHostView::GetNativeViewAccessible() {
  return NULL;
}

void RenderWidgetHostView::Focus() {}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::OnFocus() {
  GetRenderWidgetHostImpl()->GotFocus();
  GetRenderWidgetHost()->SetActive(true);
}

void RenderWidgetHostView::OnBlur() {
  GetRenderWidgetHost()->SetActive(false);
  GetRenderWidgetHost()->Blur();
}

} // namespace oxide
