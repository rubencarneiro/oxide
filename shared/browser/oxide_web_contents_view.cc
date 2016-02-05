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
      container_(nullptr),
      current_drag_allowed_ops_(blink::WebDragOperationNone),
      current_drag_op_(blink::WebDragOperationNone) {
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

void WebContentsView::HandleDragEnter(
    const content::DropData& drop_data,
    const gfx::Point& location,
    blink::WebDragOperationsMask allowed_ops,
    int key_modifiers) {
  current_drop_data_.reset(new content::DropData(drop_data));
  current_drag_allowed_ops_ = allowed_ops;

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  current_drag_target_ = RenderWidgetHostID(rvh->GetWidget());

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDragEnter(*current_drop_data_,
                           location,
                           screen_location,
                           current_drag_allowed_ops_,
                           key_modifiers);
}

blink::WebDragOperation WebContentsView::HandleDragMove(
    const gfx::Point& location,
    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    location,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDragOver(location,
                          screen_location,
                          current_drag_allowed_ops_,
                          key_modifiers);

  return current_drag_op_;
}

void WebContentsView::HandleDragLeave() {
  if (!current_drop_data_) {
    return;
  }

  current_drop_data_.reset();

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    return;
  }

  rvh->DragTargetDragLeave();
}

blink::WebDragOperation WebContentsView::HandleDrop(const gfx::Point& location,
                                                    int key_modifiers) {
  if (!current_drop_data_) {
    return blink::WebDragOperationNone;
  }

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (RenderWidgetHostID(rvh->GetWidget()) != current_drag_target_) {
    HandleDragEnter(*current_drop_data_,
                    location,
                    current_drag_allowed_ops_,
                    key_modifiers);
  }

  gfx::Point screen_location =
      BrowserPlatformIntegration::GetInstance()
        ->GetScreenClient()
        ->GetCursorScreenPoint();
  rvh->DragTargetDrop(location, screen_location, key_modifiers);

  return current_drag_op_;
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
  return current_drop_data_.get();
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

void WebContentsView::UpdateDragCursor(blink::WebDragOperation operation) {
  current_drag_op_ = operation;
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
