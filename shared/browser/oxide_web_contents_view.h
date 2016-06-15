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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_

#include <memory>
#include <queue>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/browser/web_contents/web_contents_view_oxide.h" // nogncheck
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/drop_data.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/input/oxide_input_method_context_observer.h"
#include "shared/browser/oxide_drag_source_client.h"
#include "shared/browser/oxide_mouse_event_state.h"
#include "shared/browser/oxide_render_object_id.h"
#include "shared/browser/oxide_render_widget_host_view_container.h"
#include "shared/common/oxide_shared_export.h"

struct OxideHostMsg_ShowPopup_Params;

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
}

namespace cc {
class SolidColorLayer;
}

namespace content {
class NativeWebKeyboardEvent;
class RenderFrameHost;
class RenderWidgetHost;
class WebContents;
class WebContentsImpl;
}

namespace ui {
class MotionEvent;
class TouchSelectionController;
}

namespace oxide {

class Compositor;
class CompositorFrameHandle;
class DragSource;
class RenderWidgetHostView;
class WebContentsViewClient;
class WebPopupMenu;

class OXIDE_SHARED_EXPORT WebContentsView
    : public content::WebContentsViewOxide,
      public content::WebContentsObserver,
      public CompositorClient,
      public CompositorObserver,
      public DragSourceClient,
      public InputMethodContextObserver,
      public RenderWidgetHostViewContainer {
 public:
  ~WebContentsView();
  static content::WebContentsViewOxide* Create(
      content::WebContents* web_contents);

  static WebContentsView* FromWebContents(content::WebContents* contents);

  content::WebContents* GetWebContents() const;

  WebContentsViewClient* client() const { return client_; }

  void SetClient(WebContentsViewClient* client);

  void set_editing_capabilities_changed_callback(
      const base::Closure& callback) {
    editing_capabilities_changed_callback_ = callback;
  }

  bool IsVisible() const;
  bool HasFocus() const;

  gfx::Size GetSize() const;
  gfx::Size GetSizeInPixels() const;
  gfx::Rect GetBounds() const;

  blink::WebScreenInfo GetScreenInfo() const;

  void HandleKeyEvent(const content::NativeWebKeyboardEvent& event);
  void HandleMouseEvent(const blink::WebMouseEvent& event);
  void HandleMotionEvent(const ui::MotionEvent& event);
  void HandleWheelEvent(const blink::WebMouseWheelEvent& event);

  // XXX(chrisccoulson): Make a new class for these events - we don't use
  //  ui::DragTargetEvent because it's based on ui::OSExchangeData, which I
  //  don't think we want
  void HandleDragEnter(const content::DropData& drop_data,
                       const gfx::Point& location,
                       blink::WebDragOperationsMask allowed_ops,
                       int key_modifiers);
  blink::WebDragOperation HandleDragMove(const gfx::Point& location,
                                         int key_modifiers);
  void HandleDragLeave();
  blink::WebDragOperation HandleDrop(const gfx::Point& location,
                                     int key_modifiers);

  Compositor* GetCompositor() const;
  CompositorFrameHandle* GetCompositorFrameHandle() const;
  void DidCommitCompositorFrame();

  const cc::CompositorFrameMetadata& committed_frame_metadata() const {
    return committed_frame_metadata_;
  }

  void WasResized();
  void VisibilityChanged();
  void FocusChanged();
  void ScreenUpdated();

  void HideTouchSelectionController();

 private:
  WebContentsView(content::WebContents* web_contents);

  content::WebContentsImpl* web_contents_impl() const;

  // Return the TouchSelectionController for the currently displayed RWHV
  ui::TouchSelectionController* GetTouchSelectionController() const;

  // Return the currently displayed RWHV
  RenderWidgetHostView* GetRenderWidgetHostView() const;

  // Return the currentlty displayed RWH
  content::RenderWidgetHost* GetRenderWidgetHost() const;

  bool ShouldScrollFocusedEditableNodeIntoView();
  void MaybeScrollFocusedEditableNodeIntoView();

  gfx::RectF GetBoundsF() const;

  bool ViewSizeShouldBeScreenSize() const;

  void ResizeCompositorViewport();
  void UpdateContentsSize();

  void OnShowPopup(const OxideHostMsg_ShowPopup_Params& params);
  void OnHidePopup();

  // content::WebContentsView implementation
  gfx::NativeView GetNativeView() const override;
  gfx::NativeView GetContentNativeView() const override;
  gfx::NativeWindow GetTopLevelNativeWindow() const override;
  void GetContainerBounds(gfx::Rect* out) const override;
  void SizeContents(const gfx::Size& size) override;
  void Focus() override;
  void SetInitialFocus() override;
  void StoreFocus() override;
  void RestoreFocus() override;
  content::DropData* GetDropData() const override;
  gfx::Rect GetViewBounds() const override;
  void CreateView(const gfx::Size& initial_size,
                  gfx::NativeView context) override;
  content::RenderWidgetHostViewBase* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host,
      bool is_guest_view_hack) override;
  content::RenderWidgetHostViewBase* CreateViewForPopupWidget(
      content::RenderWidgetHost* render_widget_host) override;
  void SetPageTitle(const base::string16& title) override;
  void RenderViewCreated(content::RenderViewHost* host) override;
  void RenderViewSwappedIn(content::RenderViewHost* host) override;
  void SetOverscrollControllerEnabled(bool enabled) override;

  // content::RenderViewHostDelegateView implementation
  void ShowContextMenu(content::RenderFrameHost* render_frame_host,
                       const content::ContextMenuParams& params) override;
  void StartDragging(const content::DropData& drop_data,
                     blink::WebDragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& image_offset,
                     const content::DragEventSourceInfo& event_info) override;
  void UpdateDragCursor(blink::WebDragOperation operation) override;

  // content::WebContentsObserver implementation
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  void DidShowFullscreenWidget() override;
  void DidDestroyFullscreenWidget() override;
  void DidAttachInterstitialPage() override;
  void DidDetachInterstitialPage() override;
  bool OnMessageReceived(const IPC::Message& message,
                         content::RenderFrameHost* render_frame_host) override;

  // CompositorClient implementation
  void CompositorSwapFrame(CompositorFrameHandle* handle,
                           const SwapAckCallback& callback) override;

  // CompositorObserver implementation
  void CompositorDidCommit() override;
  void CompositorEvictResources() override;

  // DragSourceClient implementaion
  void EndDrag(blink::WebDragOperation operation) override;

  // InputMethodContextObserver implementation
  void InputPanelVisibilityChanged() override;

  // RenderWidgetHostViewContainer implementation
  void AttachLayer(scoped_refptr<cc::Layer> layer) override;
  void DetachLayer(scoped_refptr<cc::Layer> layer) override;
  void CursorChanged(RenderWidgetHostView* view) override;
  gfx::Size GetViewSizeInPixels() const override;
  bool IsFullscreen() const override;
  float GetLocationBarHeight() const override;
  ui::TouchHandleDrawable* CreateTouchHandleDrawable() const override;
  void TouchSelectionChanged(RenderWidgetHostView* view,
                             bool handle_drag_in_progress,
                             bool insertion_handle_tapped) const override;
  void EditingCapabilitiesChanged(RenderWidgetHostView* view) override;

  WebContentsViewClient* client_;
  base::Closure editing_capabilities_changed_callback_;

  std::unique_ptr<Compositor> compositor_;
  scoped_refptr<cc::SolidColorLayer> root_layer_;

  scoped_refptr<CompositorFrameHandle> current_compositor_frame_;
  std::vector<scoped_refptr<CompositorFrameHandle>> previous_compositor_frames_;
  std::queue<SwapAckCallback> compositor_ack_callbacks_;

  std::unique_ptr<content::DropData> current_drop_data_;
  blink::WebDragOperationsMask current_drag_allowed_ops_;
  blink::WebDragOperation current_drag_op_;
  RenderWidgetHostID current_drag_target_;

  std::unique_ptr<DragSource> drag_source_;

  base::WeakPtr<WebPopupMenu> active_popup_menu_;

  MouseEventState mouse_state_;

  cc::CompositorFrameMetadata committed_frame_metadata_;

  RenderWidgetHostID interstitial_rwh_id_;

  content::RenderFrameHost* render_frame_message_source_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
