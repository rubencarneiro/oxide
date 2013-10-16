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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/port/browser/render_view_host_delegate_view.h"
#include "content/port/browser/web_contents_view_port.h"
#include "ui/gfx/size.h"

namespace content {
class WebContents;
}

namespace oxide {

class WebPopupMenu;
class WebView;

class WebContentsView FINAL : public content::WebContentsViewPort,
                              public content::RenderViewHostDelegateView {
 public:
  ~WebContentsView();
  WebContentsView(content::WebContents* web_contents);

  WebView* GetWebView() const;

  void CreateView(const gfx::Size& initial_size,
                  gfx::NativeView context) FINAL;
  content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;
  content::RenderWidgetHostView* CreateViewForPopupWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

  void SetPageTitle(const string16& title) FINAL;

  void RenderViewCreated(content::RenderViewHost* host) FINAL;
  void RenderViewSwappedIn(content::RenderViewHost* host) FINAL;

  void SetOverscrollControllerEnabled(bool enabled) FINAL;

  gfx::NativeView GetNativeView() const FINAL;
  gfx::NativeView GetContentNativeView() const FINAL;
  gfx::NativeWindow GetTopLevelNativeWindow() const FINAL;

  void GetContainerBounds(gfx::Rect* out) const FINAL;

  void OnTabCrashed(base::TerminationStatus status, int error_code) FINAL;

  void SizeContents(const gfx::Size& size) FINAL;

  void Focus() FINAL;
  void SetInitialFocus() FINAL;
  void StoreFocus() FINAL;
  void RestoreFocus() FINAL;

  content::DropData* GetDropData() const FINAL;

  gfx::Rect GetViewBounds() const FINAL;

  void ShowPopupMenu(const gfx::Rect& bounds,
                     int item_height,
                     double item_font_size,
                     int selected_item,
                     const std::vector<content::MenuItem>& items,
                     bool right_aligned,
                     bool allow_multiple_selection) FINAL;
  void PopupDone();

 private:
  content::WebContents* web_contents_;
  gfx::Size requested_size_;
  scoped_ptr<WebPopupMenu> active_popup_menu_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebContentsView);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_H_
