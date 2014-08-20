// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#include <queue>
#include <string>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "cc/layers/delegated_frame_resource_collection.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/common/cursors/webcursor.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

#include "shared/browser/oxide_renderer_frame_evictor_client.h"

namespace cc {
class DelegatedFrameProvider;
class DelegatedRendererLayer;
}

namespace content {
class RenderWidgetHostImpl;
}

namespace oxide {

class WebView;

class RenderWidgetHostView FINAL :
    public content::RenderWidgetHostViewBase,
    public RendererFrameEvictorClient,
    public cc::DelegatedFrameResourceCollectionClient,
    public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);
  ~RenderWidgetHostView();

  content::RenderWidgetHostImpl* host() const { return host_; }

  void CompositorDidCommit();
  void SetWebView(WebView* view);

  const base::string16& selection_text() const {
    return selection_text_;
  }

  // content::RenderWidgetHostViewBase implementation
  void Blur() FINAL;

  // content::RenderWidgetHostView implementation
  content::RenderWidgetHost* GetRenderWidgetHost() const FINAL;
  void SetSize(const gfx::Size& size) FINAL;
  void SetBounds(const gfx::Rect& rect) FINAL;
  void Focus() FINAL;

 private:
  // content::RenderWidgetHostViewBase implementation
  void SelectionChanged(const base::string16& text,
                        size_t offset,
                        const gfx::Range& range) FINAL;

  gfx::Size GetPhysicalBackingSize() const FINAL;

  void FocusedNodeChanged(bool is_editable_node) FINAL;

  void OnSwapCompositorFrame(uint32 output_surface_id,
                             scoped_ptr<cc::CompositorFrame> frame) FINAL;

  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) FINAL;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) FINAL;

  void WasShown() FINAL;
  void WasHidden() FINAL;

  void MovePluginWindows(
      const std::vector<content::WebPluginGeometry>& moves) FINAL;

  void UpdateCursor(const content::WebCursor& cursor) FINAL;
  void SetIsLoading(bool is_loading) FINAL;

  void TextInputStateChanged(
      const ViewHostMsg_TextInputState_Params& params) FINAL;

  void ImeCancelComposition() FINAL;

  void RenderProcessGone(base::TerminationStatus status, int error_code) FINAL;

  void Destroy() FINAL;

  void SetTooltipText(const base::string16& tooltip_text) FINAL;

  void SelectionBoundsChanged(
      const ViewHostMsg_SelectionBounds_Params& params) FINAL;

  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      const base::Callback<void(bool, const SkBitmap&)>& callback,
      const SkColorType color_type) FINAL;

  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(bool)>& callback) FINAL;
  bool CanCopyToVideoFrame() const FINAL;

  void AcceleratedSurfaceInitialized(int host_id, int route_id) FINAL;
  void AcceleratedSurfaceBuffersSwapped(
      const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) FINAL;
  void AcceleratedSurfaceSuspend() FINAL;
  void AcceleratedSurfaceRelease() FINAL;

  bool HasAcceleratedSurface(const gfx::Size& desired_size) FINAL;

  void GetScreenInfo(blink::WebScreenInfo* results) FINAL;
  gfx::Rect GetBoundsInRootWindow() FINAL;

  gfx::GLSurfaceHandle GetCompositingSurface() FINAL;

  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) FINAL;

  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) FINAL;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeViewId GetNativeViewId() const FINAL;
  gfx::NativeViewAccessible GetNativeViewAccessible() FINAL;

  bool HasFocus() const FINAL;

  bool IsSurfaceAvailableForCopy() const FINAL;

  void Show() FINAL;
  void Hide() FINAL;
  bool IsShowing() FINAL;

  gfx::Rect GetViewBounds() const FINAL;

  bool LockMouse() FINAL;
  void UnlockMouse() FINAL;

  // cc::DelegatedFrameResourceCollectionClient implementation
  void UnusedResourcesAreAvailable() FINAL;

  // RendererFrameEvictorClient implemenetation
  void EvictCurrentFrame() FINAL;

  // ===================

  void UpdateCursorOnWebView();

  void DestroyDelegatedContent();
  void SendDelegatedFrameAck(uint32 surface_id);
  void SendReturnedDelegatedResources();
  void RunAckCallbacks();
  void AttachLayer();
  void DetachLayer();

  content::RenderWidgetHostImpl* host_;

  WebView* web_view_;

  gfx::GLSurfaceHandle shared_surface_handle_;

  scoped_refptr<cc::DelegatedFrameResourceCollection> resource_collection_;
  scoped_refptr<cc::DelegatedFrameProvider> frame_provider_;
  scoped_refptr<cc::DelegatedRendererLayer> layer_;

  // The output surface ID for the last frame from the renderer
  uint32 last_output_surface_id_;

  std::queue<base::Closure> ack_callbacks_;

  gfx::Size last_frame_size_dip_;

  bool frame_is_evicted_;

  gfx::Rect caret_rect_;
  size_t selection_cursor_position_;
  size_t selection_anchor_position_;

  ui::TextInputType current_text_input_type_;
  bool show_ime_if_needed_;
  bool focused_node_is_editable_;

  bool is_loading_;
  content::WebCursor current_cursor_;

  bool is_showing_;
  gfx::Size last_size_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
