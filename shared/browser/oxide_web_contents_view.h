// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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
#include "shared/port/content/browser/web_contents_view_oxide.h"

namespace content {
class WebContents;
}

namespace oxide {

class RenderWidgetHostViewContainer;

class WebContentsView final : public content::WebContentsViewOxide {
 public:
  ~WebContentsView();
  static content::WebContentsViewOxide* Create(
      content::WebContents* web_contents);

  static WebContentsView* FromWebContents(content::WebContents* contents);

  void SetContainer(RenderWidgetHostViewContainer* container);

  // content::WebContentsView
  gfx::NativeView GetNativeView() const final;
  gfx::NativeView GetContentNativeView() const final;
  gfx::NativeWindow GetTopLevelNativeWindow() const final;

  void GetContainerBounds(gfx::Rect* out) const final;

  void SizeContents(const gfx::Size& size) final;

  void Focus() final;
  void SetInitialFocus() final;
  void StoreFocus() final;
  void RestoreFocus() final;

  content::DropData* GetDropData() const final;

  gfx::Rect GetViewBounds() const final;

  void CreateView(const gfx::Size& initial_size,
                  gfx::NativeView context) final;
  content::RenderWidgetHostViewBase* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host,
      bool is_guest_view_hack) final;
  content::RenderWidgetHostViewBase* CreateViewForPopupWidget(
      content::RenderWidgetHost* render_widget_host) final;

  void SetPageTitle(const base::string16& title) final;

  void RenderViewCreated(content::RenderViewHost* host) final;
  void RenderViewSwappedIn(content::RenderViewHost* host) final;

  void SetOverscrollControllerEnabled(bool enabled) final;

  // content::RenderViewHostDelegateView
  void ShowContextMenu(content::RenderFrameHost* render_frame_host,
                       const content::ContextMenuParams& params) final;
  void StartDragging(const content::DropData& drop_data,
                     blink::WebDragOperationsMask allowed_ops,
                     const gfx::ImageSkia& image,
                     const gfx::Vector2d& image_offset,
                     const content::DragEventSourceInfo& event_info) final;
  void ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                     const gfx::Rect& bounds,
                     int item_height,
                     double item_font_size,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool right_aligned,
                     bool allow_multiple_selection) final;
  void HidePopupMenu() final;

 private:
  WebContentsView(content::WebContents* web_contents);

  content::WebContents* web_contents_;

  RenderWidgetHostViewContainer* container_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebContentsView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
