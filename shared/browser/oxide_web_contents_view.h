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
#include "base/callback_list.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/browser/renderer_host/text_input_manager.h" // nogncheck
#include "content/browser/web_contents/web_contents_view_oxide.h" // nogncheck
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/drop_data.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/browser_object_weak_ptrs.h"
#include "shared/browser/compositor/oxide_compositor_client.h"
#include "shared/browser/compositor/oxide_compositor_observer.h"
#include "shared/browser/input/input_method_context_client.h"
#include "shared/browser/legacy_touch_editing_controller.h"
#include "shared/browser/oxide_drag_source_client.h"
#include "shared/browser/oxide_mouse_event_state.h"
#include "shared/browser/oxide_render_widget_host_view_container.h"
#include "shared/browser/screen_observer.h"
#include "shared/common/oxide_shared_export.h"

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
}

namespace cc {
class CompositorFrameMetadata;
class SolidColorLayer;
}

namespace content {
class NativeWebKeyboardEvent;
class RenderFrameHost;
class RenderWidgetHost;
class TextInputManager;
class WebContents;
class WebContentsImpl;
}

namespace ui {
class MotionEvent;
class TouchSelectionController;
}

namespace oxide {

class ChromeController;
class Compositor;
class CompositorFrameData;
class CompositorFrameHandle;
class DragSource;
class LegacyTouchEditingClient;
class RenderWidgetHostView;
class WebContentsViewClient;
class WebContextMenuHost;
class WebPopupMenuHost;

class OXIDE_SHARED_EXPORT WebContentsView
    : public content::WebContentsViewOxide,
      public content::WebContentsObserver,
      public CompositorClient,
      public CompositorObserver,
      public DragSourceClient,
      public InputMethodContextClient,
      public LegacyTouchEditingController,
      public RenderWidgetHostViewContainer,
      public ScreenObserver {
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

  display::Display GetDisplay() const;

  void HandleKeyEvent(const content::NativeWebKeyboardEvent& event);
  void HandleMouseEvent(blink::WebMouseEvent event);
  void HandleMotionEvent(const ui::MotionEvent& event);
  void HandleWheelEvent(blink::WebMouseWheelEvent event);

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

  using SwapCompositorFrameSubscription =
      base::CallbackList<void(const CompositorFrameData*,
                              const cc::CompositorFrameMetadata&)>::Subscription;
  std::unique_ptr<SwapCompositorFrameSubscription>
  AddSwapCompositorFrameCallback(
      const base::Callback<void(const CompositorFrameData*,
                                const cc::CompositorFrameMetadata&)>& callback);

  void WasResized();
  void ScreenRectsChanged();
  void VisibilityChanged();
  void FocusChanged();
  void ScreenChanged();

  // XXX(chrisccoulson): This will probably be removed, please don't use it
  //  See https://launchpad.net/bugs/1665722
  ChromeController* chrome_controller() const {
    return chrome_controller_.get();
  }

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

  void DidCloseContextMenu();
  void DidHidePopupMenu();

  void HideTouchSelectionController();

  void CursorChangedInternal(RenderWidgetHostView* view);
  void TouchEditingStatusChangedInternal(RenderWidgetHostView* view);
  void TextInputStateChangedInternal(const content::TextInputState* state);
  void SelectionBoundsChangedInternal(
      const content::TextInputManager::SelectionRegion& region);
  void TextSelectionChangedInternal(
      const content::TextInputManager::TextSelection& selection);

  void SyncClientWithNewView(RenderWidgetHostView* view = nullptr);

  const content::TextInputManager::TextSelection* GetTextSelection() const;

  // content::WebContentsView implementation
  gfx::NativeView GetNativeView() const override;
  gfx::NativeView GetContentNativeView() const override;
  gfx::NativeWindow GetTopLevelNativeWindow() const override;
  void GetScreenInfo(content::ScreenInfo* screen_info) const override;
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
                     const content::DragEventSourceInfo& event_info,
                     content::RenderWidgetHostImpl* source_rwh) override;
  void UpdateDragCursor(blink::WebDragOperation operation) override;
  void GotFocus() override;
  void ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                     const gfx::Rect& bounds,
                     int item_height,
                     double item_font_size,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool right_aligned,
                     bool allow_multiple_selection) override;
  void HidePopupMenu() override;

  // content::WebContentsObserver implementation
  void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  void RenderViewHostChanged(content::RenderViewHost* old_host,
                             content::RenderViewHost* new_host) override;
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  void DidShowFullscreenWidget() override;
  void DidDestroyFullscreenWidget() override;
  void DidAttachInterstitialPage() override;
  void DidDetachInterstitialPage() override;

  // CompositorClient implementation
  void CompositorSwapFrame(CompositorFrameHandle* handle,
                           const SwapAckCallback& callback) override;

  // CompositorObserver implementation
  void CompositorDidCommit() override;
  void CompositorEvictResources() override;

  // DragSourceClient implementaion
  void EndDrag(blink::WebDragOperation operation) override;

  // InputMethodContextClient implementation
  void InputPanelVisibilityChanged() override;
  void SetComposingText(
      const base::string16& text,
      const std::vector<blink::WebCompositionUnderline>& underlines,
      const gfx::Range& selection_range) override;
  void CommitText(const base::string16& text,
                  const gfx::Range& replacement_range) override;
  base::string16 GetSelectionText() const override;
  bool GetSelectedText(base::string16* text) const override;

  // LegacyTouchEditingController implementation
  void HideAndDisallowShowingAutomatically() override;

  // RenderWidgetHostViewContainer implementation
  void AttachLayer(scoped_refptr<cc::Layer> layer) override;
  void DetachLayer(scoped_refptr<cc::Layer> layer) override;
  void CursorChanged(RenderWidgetHostView* view) override;
  gfx::Size GetViewSizeInPixels() const override;
  bool IsFullscreen() const override;
  float GetTopControlsHeight() override;
  std::unique_ptr<ui::TouchHandleDrawable> CreateTouchHandleDrawable() override;
  void TouchEditingStatusChanged(RenderWidgetHostView* view) override;
  void TouchInsertionHandleTapped(RenderWidgetHostView* view) override;
  void TextInputStateChanged(RenderWidgetHostView* view,
                             const content::TextInputState* state) override;
  void ImeCancelComposition(RenderWidgetHostView* view) override;
  void SelectionBoundsChanged(
      RenderWidgetHostView* view,
      const content::TextInputManager::SelectionRegion* region) override;
  void TextSelectionChanged(
      RenderWidgetHostView* view,
      content::RenderWidgetHostViewBase* focused_view,
      const content::TextInputManager::TextSelection* selection) override;
  void FocusedNodeChanged(RenderWidgetHostView* view,
                          bool is_editable_node) override;

  // ScreenObserver implementation
  void OnDisplayPropertiesChanged(const display::Display& display) override;

  WebContentsViewClient* client_;
  base::Closure editing_capabilities_changed_callback_;

  std::unique_ptr<Compositor> compositor_;
  scoped_refptr<cc::SolidColorLayer> root_layer_;

  scoped_refptr<CompositorFrameHandle> current_compositor_frame_;
  std::vector<scoped_refptr<CompositorFrameHandle>> previous_compositor_frames_;
  std::queue<SwapAckCallback> compositor_ack_callbacks_;

  RenderWidgetHostWeakPtr rwh_at_last_commit_;
  base::CallbackList<void(const CompositorFrameData*,
                          const cc::CompositorFrameMetadata&)>
      swap_compositor_frame_callbacks_;

  std::unique_ptr<content::DropData> current_drop_data_;
  blink::WebDragOperationsMask current_drag_allowed_ops_;
  gfx::Point current_drag_location_;
  gfx::Point current_drag_screen_location_;
  blink::WebDragOperation current_drag_op_;
  RenderWidgetHostWeakPtr current_drag_target_;

  std::unique_ptr<DragSource> drag_source_;
  RenderWidgetHostWeakPtr drag_source_rwh_;

  std::unique_ptr<WebPopupMenuHost> active_popup_menu_;
  std::unique_ptr<WebContextMenuHost> active_context_menu_;

  MouseEventState mouse_state_;

  RenderWidgetHostWeakPtr interstitial_rwh_;

  std::unique_ptr<ChromeController> chrome_controller_;

  LegacyTouchEditingClient* legacy_touch_editing_client_;

  RenderWidgetHostWeakPtr last_focused_widget_for_text_selection_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
