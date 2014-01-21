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

#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"

#include "oxide_web_popup_menu.h"
#include "oxide_web_view.h"

namespace oxide {

WebContentsView::WebContentsView(content::WebContents* web_contents) :
    web_contents_(web_contents) {}

WebContentsView::~WebContentsView() {}

WebView* WebContentsView::GetWebView() const {
  return WebView::FromWebContents(web_contents_);
}

void WebContentsView::CreateView(const gfx::Size& initial_size,
                                 gfx::NativeView context) {}

content::RenderWidgetHostView* WebContentsView::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  content::RenderWidgetHostView* rvh =
      GetWebView()->CreateViewForWidget(render_widget_host);
  if (!rvh) {
    return NULL;
  }

  if (!content::RenderWidgetHostImpl::From(render_widget_host)->is_hidden()) {
    rvh->Show();
  } else {
    rvh->Hide();
  }

  return rvh;
}

content::RenderWidgetHostView* WebContentsView::CreateViewForPopupWidget(
    content::RenderWidgetHost* render_widget_host) {
  return NULL;
}

void WebContentsView::SetPageTitle(const base::string16& title) {}

void WebContentsView::RenderViewCreated(content::RenderViewHost* host) {}

void WebContentsView::RenderViewSwappedIn(content::RenderViewHost* host) {}

void WebContentsView::SetOverscrollControllerEnabled(bool enabled) {}

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
  *out = GetWebView()->GetContainerBounds();
}

void WebContentsView::OnTabCrashed(base::TerminationStatus status,
                                   int error_code) {}

void WebContentsView::SizeContents(const gfx::Size& size) {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(size);
  }
}

void WebContentsView::Focus() {
  content::RenderWidgetHostView* rwhv =
      web_contents_->GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->Focus();
  }
}

void WebContentsView::SetInitialFocus() {}

void WebContentsView::StoreFocus() {}

void WebContentsView::RestoreFocus() {}

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

void WebContentsView::ShowPopupMenu(
    const gfx::Rect& bounds,
    int item_height,
    double item_font_size,
    int selected_item,
    const std::vector<content::MenuItem>& items,
    bool right_aligned,
    bool allow_multiple_selection) {
  // XXX: Can this happen? Should we cancel an existing popup?
  DCHECK(!active_popup_menu_);

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();

  WebPopupMenu* menu = GetWebView()->CreatePopupMenu(rvh);
  if (!menu) {
    static_cast<content::RenderViewHostImpl *>(rvh)->DidCancelPopupMenu();
    return;
  }
  active_popup_menu_ = menu->GetWeakPtr();

  active_popup_menu_->ShowPopup(bounds, items, selected_item,
                                allow_multiple_selection);
}

} // namespace oxide
