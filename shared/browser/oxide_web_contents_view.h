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

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/drop_data.h"
#include "shared/port/content/browser/web_contents_view_oxide.h"

#include "shared/browser/oxide_drag_source_client.h"
#include "shared/browser/oxide_mouse_event_state.h"
#include "shared/browser/oxide_render_object_id.h"
#include "shared/browser/oxide_touch_event_state.h"

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
}

namespace content {
class NativeWebKeyboardEvent;
class RenderWidgetHost;
class WebContents;
class WebContentsImpl;
}

namespace ui {
class TouchEvent;
class TouchSelectionController;
}

namespace oxide {

class DragSource;
class RenderWidgetHostView;
class WebContentsViewClient;
class WebPopupMenu;

class WebContentsView : public content::WebContentsViewOxide,
                        public content::WebContentsObserver,
                        public DragSourceClient {
 public:
  ~WebContentsView();
  static content::WebContentsViewOxide* Create(
      content::WebContents* web_contents);

  static WebContentsView* FromWebContents(content::WebContents* contents);

  WebContentsViewClient* client() const { return client_; }

  void SetClient(WebContentsViewClient* client);

  gfx::Rect GetBoundsPix() const;
  gfx::Rect GetBoundsDip() const;

  blink::WebScreenInfo GetScreenInfo() const;

  void HandleKeyEvent(const content::NativeWebKeyboardEvent& event);
  void HandleMouseEvent(const blink::WebMouseEvent& event);
  void HandleTouchEvent(const ui::TouchEvent& event);
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

 private:
  WebContentsView(content::WebContents* web_contents);

  content::WebContentsImpl* web_contents_impl() const;

  // Return the TouchSelectionController for the current RWHV. Ignores
  // fullscreen RWHVs
  ui::TouchSelectionController* GetTouchSelectionController() const;

  // Return the current RWHV. Will return the fullscreen RWHV if there is one
  RenderWidgetHostView* GetRenderWidgetHostView() const;

  // Return the current RWH. Will return the fullscreen RWH if there is one
  content::RenderWidgetHost* GetRenderWidgetHost() const;

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
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;

  // DragSourceClient implementaion
  void EndDrag(blink::WebDragOperation operation) override;

  WebContentsViewClient* client_;

  scoped_ptr<content::DropData> current_drop_data_;
  blink::WebDragOperationsMask current_drag_allowed_ops_;
  blink::WebDragOperation current_drag_op_;
  RenderWidgetHostID current_drag_target_;

  scoped_ptr<DragSource> drag_source_;

  base::WeakPtr<WebPopupMenu> active_popup_menu_;

  MouseEventState mouse_state_;
  TouchEventState touch_state_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
