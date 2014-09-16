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

#include "oxide_web_contents_view.h"

#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/web_contents.h"

#include "oxide_render_widget_host_view.h"
#include "oxide_web_view.h"

namespace content {

WebContentsView* CreateWebContentsView(
    WebContentsImpl* web_contents,
    WebContentsViewDelegate* delegate,
    RenderViewHostDelegateView** render_view_host_delegate_view) {
  oxide::WebContentsView* rv = new oxide::WebContentsView(web_contents);
  *render_view_host_delegate_view = rv;
  return rv;
}

} // namespace content

namespace oxide {

WebContentsView::WebContentsView(content::WebContents* web_contents) :
    web_contents_(web_contents) {}

WebContentsView::~WebContentsView() {}

WebView* WebContentsView::GetWebView() const {
  return WebView::FromWebContents(web_contents_);
}

gfx::NativeView WebContentsView::GetNativeView() const {
  return NULL;
}

gfx::NativeView WebContentsView::GetContentNativeView() const {
  return NULL;
}

gfx::NativeWindow WebContentsView::GetTopLevelNativeWindow() const {
  return NULL;
}

void WebContentsView::GetContainerBounds(gfx::Rect* out) const {
  *out = GetWebView()->GetContainerBoundsDip();
}

void WebContentsView::SizeContents(const gfx::Size& size) {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(size);
  }
}

void WebContentsView::Focus() {
  NOTREACHED();
}

void WebContentsView::SetInitialFocus() {
  NOTREACHED();
}

void WebContentsView::StoreFocus() {
  NOTREACHED();
}

void WebContentsView::RestoreFocus() {
  NOTREACHED();
}

content::DropData* WebContentsView::GetDropData() const {
  return NULL;
}

gfx::Rect WebContentsView::GetViewBounds() const {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetRenderWidgetHostView();
  if (rwhv) {
    return rwhv->GetViewBounds();
  }

  return gfx::Rect();
}

void WebContentsView::CreateView(const gfx::Size& initial_size,
                                 gfx::NativeView context) {}

content::RenderWidgetHostViewBase* WebContentsView::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  RenderWidgetHostView* rwhv = new RenderWidgetHostView(render_widget_host);

  WebView* view = GetWebView();
  if (view) {
    // As RWHV contains the plumbing from WebView::VisibilityChanged to
    // RenderWidgetHostImpl::Was{Shown,Hidden}, RWHI::is_hidden could be
    // out of date. This ensures that we sync RWHI::is_hidden with the
    // real visibility of the webview - see https://launchpad.net/bugs/1322622
    view->VisibilityChanged();
  }

  return rwhv;
}

content::RenderWidgetHostViewBase* WebContentsView::CreateViewForPopupWidget(
    content::RenderWidgetHost* render_widget_host) {
  return NULL;
}

void WebContentsView::SetPageTitle(const base::string16& title) {}

void WebContentsView::RenderViewCreated(content::RenderViewHost* host) {}

void WebContentsView::RenderViewSwappedIn(content::RenderViewHost* host) {}

void WebContentsView::SetOverscrollControllerEnabled(bool enabled) {}

void WebContentsView::ShowPopupMenu(
    content::RenderFrameHost* render_frame_host,
    const gfx::Rect& bounds,
    int item_height,
    double item_font_size,
    int selected_item,
    const std::vector<content::MenuItem>& items,
    bool right_aligned,
    bool allow_multiple_selection) {
  GetWebView()->ShowPopupMenu(render_frame_host,
                              bounds, selected_item, items,
                              allow_multiple_selection);
}

void WebContentsView::HidePopupMenu() {
  GetWebView()->HidePopupMenu();
}

} // namespace oxide
