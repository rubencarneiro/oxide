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

#include "oxide_web_contents_view.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/web_contents.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "third_party/WebKit/public/web/WebDragOperation.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/touch_selection/touch_selection_controller.h"

#include "shared/common/oxide_unowned_user_data.h"

#include "oxide_browser_platform_integration.h"
#include "oxide_drag_source.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_render_widget_host_view_container.h"
#include "oxide_screen_client.h"

namespace oxide {

namespace {
int kUserDataKey;
}

WebContentsView::WebContentsView(content::WebContents* web_contents)
    : web_contents_(static_cast<content::WebContentsImpl*>(web_contents)),
      container_(nullptr) {
  web_contents_->SetUserData(&kUserDataKey,
                             new UnownedUserData<WebContentsView>(this));
}

ui::TouchSelectionController* WebContentsView::GetTouchSelectionController() {
  // We don't care about checking for a fullscreen view here - we're called
  // from StartDragging which is only supported in RenderViews
  content::RenderWidgetHostView* view =
      web_contents_->GetRenderWidgetHostView();
  if (!view) {
    return nullptr;
  }

  return static_cast<RenderWidgetHostView*>(view)->selection_controller();
}

void WebContentsView::SetContainer(RenderWidgetHostViewContainer* container) {
  container_ = container;
}

gfx::NativeView WebContentsView::GetNativeView() const {
  return nullptr;
}

gfx::NativeView WebContentsView::GetContentNativeView() const {
  return nullptr;
}

gfx::NativeWindow WebContentsView::GetTopLevelNativeWindow() const {
  return nullptr;
}

void WebContentsView::GetContainerBounds(gfx::Rect* out) const {
  *out = container_->GetViewBoundsDip();
}

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
  if (!rwhv) {
    return;
  }

  rwhv->Focus();
}

void WebContentsView::SetInitialFocus() {
  NOTIMPLEMENTED();
}

void WebContentsView::StoreFocus() {}

void WebContentsView::RestoreFocus() {}

content::DropData* WebContentsView::GetDropData() const {
  return nullptr;
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
    content::RenderWidgetHost* render_widget_host,
    bool is_guest_view_hack) {
  return new RenderWidgetHostView(
      content::RenderWidgetHostImpl::From(render_widget_host));
}

content::RenderWidgetHostViewBase* WebContentsView::CreateViewForPopupWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostView(
      content::RenderWidgetHostImpl::From(render_widget_host));
}

void WebContentsView::SetPageTitle(const base::string16& title) {}

void WebContentsView::RenderViewCreated(content::RenderViewHost* host) {}

void WebContentsView::RenderViewSwappedIn(content::RenderViewHost* host) {}

void WebContentsView::SetOverscrollControllerEnabled(bool enabled) {}

void WebContentsView::ShowContextMenu(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params) {
  container_->ShowContextMenu(render_frame_host, params);
}

void WebContentsView::StartDragging(
    const content::DropData& drop_data,
    blink::WebDragOperationsMask allowed_ops,
    const gfx::ImageSkia& image,
    const gfx::Vector2d& image_offset,
    const content::DragEventSourceInfo& event_info) {
  if (drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag when one is already in progress";
    web_contents_->SystemDragEnded();
    return;
  }

  drag_source_ =
      BrowserPlatformIntegration::GetInstance()->CreateDragSource(this);
  if (!drag_source_) {
    LOG(WARNING) <<
        "Rejecting request to start a drag - not supported";
    web_contents_->SystemDragEnded();
    return;
  }

  ui::TouchSelectionController* selection_controller =
      GetTouchSelectionController();
  if (selection_controller) {
    selection_controller->HideAndDisallowShowingAutomatically();
  }

  float scale = container_->GetScreenInfo().deviceScaleFactor;
  gfx::Vector2d image_offset_pix(image_offset.x() * scale,
                                 image_offset.y() * scale);

  drag_source_->StartDragging(web_contents_,
                              drop_data,
                              allowed_ops,
                              *image.bitmap(),
                              image_offset_pix);
}

void WebContentsView::ShowPopupMenu(
    content::RenderFrameHost* render_frame_host,
    const gfx::Rect& bounds,
    int item_height,
    double item_font_size,
    int selected_item,
    const std::vector<content::MenuItem>& items,
    bool right_aligned,
    bool allow_multiple_selection) {
  container_->ShowPopupMenu(render_frame_host,
                            bounds, selected_item, items,
                            allow_multiple_selection);
}

void WebContentsView::HidePopupMenu() {
  container_->HidePopupMenu();
}

void WebContentsView::EndDrag(blink::WebDragOperation operation) {
  DCHECK(drag_source_);

  gfx::Point screen_point =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  gfx::Point view_point =
      screen_point - gfx::Vector2d(GetViewBounds().origin().x(),
                                   GetViewBounds().origin().y());
  web_contents_->DragSourceEndedAt(view_point.x(), view_point.y(),
                                   screen_point.x(), screen_point.y(),
                                   operation);

  web_contents_->SystemDragEnded();

  drag_source_.reset();
}

WebContentsView::~WebContentsView() {
  web_contents_->RemoveUserData(&kUserDataKey);
}

// static
content::WebContentsViewOxide* WebContentsView::Create(
    content::WebContents* web_contents) {
  return new WebContentsView(web_contents);
}

// static
WebContentsView* WebContentsView::FromWebContents(
    content::WebContents* contents) {
  UnownedUserData<WebContentsView>* data =
      static_cast<UnownedUserData<WebContentsView>*>(
        contents->GetUserData(&kUserDataKey));
  if (!data) {
    return nullptr;
  }

  return data->get();
}

} // namespace oxide
