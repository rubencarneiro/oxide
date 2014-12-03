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
#include "cc/output/compositor_frame_metadata.h"
#include "content/common/cursors/webcursor.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

#include "shared/browser/oxide_renderer_frame_evictor_client.h"
#include "shared/port/content/browser/render_widget_host_view_oxide.h"

namespace cc {
class DelegatedFrameProvider;
class DelegatedRendererLayer;
}

namespace content {
class RenderWidgetHostImpl;
}

namespace oxide {

class RenderWidgetHostViewDelegate;
class WebView;

class RenderWidgetHostView final :
    public content::RenderWidgetHostViewOxide,
    public RendererFrameEvictorClient,
    public cc::DelegatedFrameResourceCollectionClient,
    public base::SupportsWeakPtr<RenderWidgetHostView> {
 public:
  RenderWidgetHostView(content::RenderWidgetHost* render_widget_host);
  ~RenderWidgetHostView();

  content::RenderWidgetHostImpl* host() const { return host_; }

  void CompositorDidCommit();
  void SetDelegate(RenderWidgetHostViewDelegate* delegate);

  const base::string16& selection_text() const {
    return selection_text_;
  }

  const cc::CompositorFrameMetadata& compositor_frame_metadata() const {
    return compositor_frame_metadata_;
  }

  // content::RenderWidgetHostViewBase implementation
  void Blur() final;

  // content::RenderWidgetHostView implementation
  content::RenderWidgetHost* GetRenderWidgetHost() const final;
  void SetSize(const gfx::Size& size) final;
  void SetBounds(const gfx::Rect& rect) final;
  void Focus() final;

 private:
  // content::RenderWidgetHostViewOxide implementation
  void OnTextInputStateChanged(ui::TextInputType type,
                               bool show_ime_if_needed) final;
  void OnSelectionBoundsChanged(const gfx::Rect& anchor_rect,
                                const gfx::Rect& focus_rect,
                                bool is_anchor_first) final;

  // content::RenderWidgetHostViewBase implementation
  void SelectionChanged(const base::string16& text,
                        size_t offset,
                        const gfx::Range& range) final;
  gfx::Size GetPhysicalBackingSize() const final;
  float GetTopControlsLayoutHeight() const final;
  void FocusedNodeChanged(bool is_editable_node) final;
  void OnSwapCompositorFrame(uint32 output_surface_id,
                             scoped_ptr<cc::CompositorFrame> frame) final;
  void InitAsPopup(content::RenderWidgetHostView* parent_host_view,
                   const gfx::Rect& pos) final;
  void InitAsFullscreen(
      content::RenderWidgetHostView* reference_host_view) final;
  void WasShown() final;
  void WasHidden() final;
  void MovePluginWindows(
      const std::vector<content::WebPluginGeometry>& moves) final;
  void UpdateCursor(const content::WebCursor& cursor) final;
  void SetIsLoading(bool is_loading) final;
  void TextInputTypeChanged(ui::TextInputType type,
                            ui::TextInputMode mode,
                            bool can_compose_inline,
                            int flags) final;
  void ImeCancelComposition() final;
  void RenderProcessGone(base::TerminationStatus status, int error_code) final;
  void Destroy() final;
  void SetTooltipText(const base::string16& tooltip_text) final;
  void CopyFromCompositingSurface(
      const gfx::Rect& src_subrect,
      const gfx::Size& dst_size,
      content::ReadbackRequestCallback& callback,
      const SkColorType color_type) final;
  void CopyFromCompositingSurfaceToVideoFrame(
      const gfx::Rect& src_subrect,
      const scoped_refptr<media::VideoFrame>& target,
      const base::Callback<void(bool)>& callback) final;
  bool CanCopyToVideoFrame() const final;
  bool HasAcceleratedSurface(const gfx::Size& desired_size) final;
  void GetScreenInfo(blink::WebScreenInfo* results) final;
  gfx::Rect GetBoundsInRootWindow() final;
  gfx::GLSurfaceHandle GetCompositingSurface() final;
  void ShowDisambiguationPopup(const gfx::Rect& rect_pixels,
                               const SkBitmap& zoomed_bitmap) final;
  void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo& touch,
                              content::InputEventAckState ack_result) final;
  void ImeCompositionRangeChanged(
      const gfx::Range& range,
      const std::vector<gfx::Rect>& character_bounds) final;

  // content::RenderWidgetHostView implementation
  void InitAsChild(gfx::NativeView parent_view) final;
  gfx::Vector2dF GetLastScrollOffset() const final;
  gfx::NativeView GetNativeView() const final;
  gfx::NativeViewId GetNativeViewId() const final;
  gfx::NativeViewAccessible GetNativeViewAccessible() final;
  bool HasFocus() const final;
  bool IsSurfaceAvailableForCopy() const final;
  void Show() final;
  void Hide() final;
  bool IsShowing() final;
  gfx::Rect GetViewBounds() const final;
  bool LockMouse() final;
  void UnlockMouse() final;

  // cc::DelegatedFrameResourceCollectionClient implementation
  void UnusedResourcesAreAvailable() final;

  // RendererFrameEvictorClient implemenetation
  void EvictCurrentFrame() final;

  // ===================

  void UpdateCursorOnWebView();

  void DestroyDelegatedContent();
  void SendDelegatedFrameAck(uint32 surface_id);
  void SendReturnedDelegatedResources();
  void RunAckCallbacks();
  void AttachLayer();
  void DetachLayer();

  content::RenderWidgetHostImpl* host_;

  RenderWidgetHostViewDelegate* delegate_;

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

  cc::CompositorFrameMetadata compositor_frame_metadata_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(RenderWidgetHostView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
