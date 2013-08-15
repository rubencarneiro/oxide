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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
#define _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "ui/gfx/size.h"

namespace content {
class RenderWidgetHostImpl;
}

namespace oxide {

class RenderWidgetHostView : public content::RenderWidgetHostViewBase {
 public:
  virtual ~RenderWidgetHostView();

  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) FINAL;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) FINAL;

  void WasShown() FINAL;
  void WasHidden() FINAL;

  void MovePluginWindows(
      const gfx::Vector2d& scroll_offset,
      const std::vector<content::WebPluginGeometry>& moves) FINAL;

  virtual void Blur() OVERRIDE;

  void UpdateCursor(const WebCursor& cursor) FINAL;

  void SetIsLoading(bool is_loading) FINAL;

  void TextInputTypeChanged(ui::TextInputType type,
                            bool can_compose_inline,
                            ui::TextInputMode mode) FINAL;

  void ImeCancelComposition() FINAL;
  void ImeCompositionRangeChanged(
      const ui::Range& range,
      const std::vector<gfx::Rect>& character_bounds) FINAL;

  void DidUpdateBackingStore(
      const gfx::Rect& scroll_rect,
      const gfx::Vector2d& scroll_delta,
      const std::vector<gfx::Rect>& copy_rects,
      const ui::LatencyInfo& latency_info) FINAL;

  void RenderProcessGone(base::TerminationStatus status, int error_code) FINAL;

  void Destroy() FINAL;

  void SetTooltipText(const string16& tooltip_text) FINAL;

  void SelectionBoundsChanged(
      const ViewHostMsg_SelectionBounds_Params& params) FINAL;

  void ScrollOffsetChanged() FINAL;

  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      const base::Callback<void(bool, const SkBitmap&)>& callback) FINAL;

  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(bool)>& callback) FINAL;
  bool CanCopyToVideoFrame() const FINAL;

  void OnAcceleratedCompositingStateChange() FINAL;
  void AcceleratedSurfaceBuffersSwapped(
      const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfaceSuspend() FINAL;
  void AcceleratedSurfaceRelease() FINAL;

  bool HasAcceleratedSurface(const gfx::Size& desired_size) FINAL;

  virtual gfx::Size GetPhysicalBackingSize() const OVERRIDE;

  gfx::GLSurfaceHandle GetCompositingSurface() FINAL;

  void SetHasHorizontalScrollbar(bool has_horizontal_scrollbar) FINAL;
  void SetScrollOffsetPinning(bool is_pinned_to_left,
                              bool is_pinned_to_right) FINAL;

  void OnAccessibilityNotifications(
      const std::vector<AccessibilityHostMsg_NotificationParams>& params)
      FINAL;

  void InitAsChild(gfx::NativeView parent_view) FINAL;

  content::RenderWidgetHost* GetRenderWidgetHost() const FINAL;
  content::RenderWidgetHostImpl* GetRenderWidgetHostImpl() const {
    return host_;
  }

  void SetSize(const gfx::Size& size) FINAL;
  void SetBounds(const gfx::Rect& rect) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeViewId GetNativeViewId() const FINAL;
  gfx::NativeViewAccessible GetNativeViewAccessible() FINAL;

  void Focus() OVERRIDE;

  bool IsSurfaceAvailableForCopy() const FINAL;

  bool LockMouse() FINAL;
  void UnlockMouse() FINAL;

  void OnFocus();
  void OnBlur();

 protected:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);

 private:
  virtual void ScheduleUpdate(const gfx::Rect& rect) = 0;

  bool is_hidden_;
  content::RenderWidgetHostImpl* host_;
  gfx::Size requested_size_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
