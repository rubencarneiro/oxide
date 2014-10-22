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

#include "oxide_render_widget_host_view.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/scoped_vector.h"
#include "cc/layers/delegated_frame_provider.h"
#include "cc/layers/delegated_renderer_layer.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/delegated_frame_data.h"
#include "cc/quads/render_pass.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_utils.h"

#include "oxide_browser_process_main.h"
#include "oxide_renderer_frame_evictor.h"
#include "oxide_render_widget_host_view_delegate.h"
#include "oxide_web_view.h"

namespace oxide {

void RenderWidgetHostView::OnTextInputStateChanged(
    ui::TextInputType type,
    bool show_ime_if_needed) {
  if (type != current_text_input_type_ ||
      show_ime_if_needed != show_ime_if_needed_) {
    current_text_input_type_ = type;
    show_ime_if_needed_ = show_ime_if_needed;

    if (delegate_) {
      delegate_->TextInputStateChanged(current_text_input_type_,
                                       show_ime_if_needed_);
    }
  }
}

void RenderWidgetHostView::OnSelectionBoundsChanged(
    const gfx::Rect& anchor_rect,
    const gfx::Rect& focus_rect,
    bool is_anchor_first) {
  caret_rect_ = gfx::UnionRects(anchor_rect, focus_rect);

  if (selection_range_.IsValid()) {
    if (is_anchor_first) {
      selection_cursor_position_ =
          selection_range_.GetMax() - selection_text_offset_;
      selection_anchor_position_ =
          selection_range_.GetMin() - selection_text_offset_;
    } else {
      selection_cursor_position_ =
          selection_range_.GetMin() - selection_text_offset_;
      selection_anchor_position_ =
          selection_range_.GetMax() - selection_text_offset_;
    }
  }

  if (delegate_) {
    delegate_->SelectionBoundsChanged(caret_rect_,
                                      selection_cursor_position_,
                                      selection_anchor_position_);
  }
}

void RenderWidgetHostView::SelectionChanged(const base::string16& text,
                                            size_t offset,
                                            const gfx::Range& range) {
  if ((range.GetMin() - offset) > text.length()) {
    // Got an invalid selection (see https://launchpad.net/bugs/1375900).
    // The issue lies in content::RenderFrameImpl::SyncSelectionIfRequired(…)
    // where the selection text and the corresponding range are computed
    // separately. If the word that just got committed is at the beginning of a
    // new line, the selection range includes the trailing newline character(s)
    // whereas the selection text truncates them.
    // This looks very similar to https://crbug.com/101435.
    return;
  }

  content::RenderWidgetHostViewBase::SelectionChanged(text, offset, range);

  if (delegate_) {
    delegate_->SelectionChanged();
  }
}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  if (!delegate_) {
    return gfx::Size();
  }

  return delegate_->GetWebView()->GetViewSizePix();
}

void RenderWidgetHostView::FocusedNodeChanged(bool is_editable_node) {
  focused_node_is_editable_ = is_editable_node;
  if (delegate_) {
    delegate_->FocusedNodeChanged(is_editable_node);
  }
}

void RenderWidgetHostView::OnSwapCompositorFrame(
    uint32 output_surface_id,
    scoped_ptr<cc::CompositorFrame> frame) {
  if (!frame->delegated_frame_data) {
    DLOG(ERROR) << "Non delegated renderer path is not supported";
    host_->GetProcess()->ReceivedBadMessage();
    return;
  }

  scoped_ptr<cc::DelegatedFrameData> frame_data =
      frame->delegated_frame_data.Pass();

  if (frame_data->render_pass_list.empty()) {
    DLOG(ERROR) << "Invalid delegated frame";
    host_->GetProcess()->ReceivedBadMessage();
    return;
  }

  Compositor* compositor = delegate_ ? delegate_->GetCompositor() : NULL;
  CompositorLock lock(compositor);

  if (output_surface_id != last_output_surface_id_) {
    resource_collection_->SetClient(NULL);
    if (resource_collection_->LoseAllResources()) {
      SendReturnedDelegatedResources();
    }
    resource_collection_ = new cc::DelegatedFrameResourceCollection();
    resource_collection_->SetClient(this);

    DestroyDelegatedContent();

    // XXX(chrisccoulson): Should we clear ack_callbacks_ here as well?

    last_output_surface_id_ = output_surface_id;
  }

  base::Closure ack_callback =
      base::Bind(&RenderWidgetHostView::SendDelegatedFrameAck,
                 AsWeakPtr(), output_surface_id);
  ack_callbacks_.push(ack_callback);

  float device_scale_factor = frame->metadata.device_scale_factor;
  cc::RenderPass* root_pass = frame_data->render_pass_list.back();

  gfx::Size frame_size = root_pass->output_rect.size();
  gfx::Size frame_size_dip = gfx::Size(
      std::lround(frame_size.width() / device_scale_factor),
      std::lround(frame_size.height() / device_scale_factor));

  gfx::Rect damage_rect_dip = gfx::ToEnclosingRect(
      gfx::ScaleRect(root_pass->damage_rect, 1.0f / device_scale_factor));

  if (frame_size.IsEmpty()) {
    DestroyDelegatedContent();
  } else {
    if (!frame_provider_.get() ||
        frame_size != frame_provider_->frame_size() ||
        frame_size_dip != last_frame_size_dip_) {
      DetachLayer();
      frame_provider_ = new cc::DelegatedFrameProvider(resource_collection_,
                                                       frame_data.Pass());
      layer_ = cc::DelegatedRendererLayer::Create(frame_provider_);
      AttachLayer();
    } else {
      frame_provider_->SetFrameData(frame_data.Pass());
    }
  }

  last_frame_size_dip_ = frame_size_dip;

  if (layer_.get()) {
    layer_->SetIsDrawable(true);
    layer_->SetContentsOpaque(true);
    layer_->SetBounds(frame_size_dip);
    layer_->SetNeedsDisplayRect(damage_rect_dip);
  }

  if (frame_is_evicted_) {
    frame_is_evicted_ = false;
    RendererFrameEvictor::GetInstance()->AddFrame(this, IsShowing());
  }

  if (delegate_) {
    delegate_->UpdateFrameMetadata(frame->metadata);
  }

  if (!compositor || !compositor->IsActive()) {
    RunAckCallbacks();
  }
}

void RenderWidgetHostView::InitAsPopup(
    content::RenderWidgetHostView* parent_host_view,
    const gfx::Rect& pos) {
  NOTREACHED() << "Popup RenderWidgetHostView's are not supported";
}

void RenderWidgetHostView::InitAsFullscreen(
    content::RenderWidgetHostView* reference_host_view) {
  NOTREACHED() << "Fullscreen RenderWidgetHostView's are not supported";
}

void RenderWidgetHostView::WasShown() {
  DCHECK(delegate_);

  if (!frame_is_evicted_) {
    RendererFrameEvictor::GetInstance()->LockFrame(this);
  }
  host_->WasShown(ui::LatencyInfo());
}

void RenderWidgetHostView::WasHidden() {
  if (!frame_is_evicted_) {
    RendererFrameEvictor::GetInstance()->UnlockFrame(this);
  }
  host_->WasHidden();
  RunAckCallbacks();
}

void RenderWidgetHostView::MovePluginWindows(
    const std::vector<content::WebPluginGeometry>& moves) {}

void RenderWidgetHostView::UpdateCursor(const content::WebCursor& cursor) {
  if (cursor.IsEqual(current_cursor_)) {
    return;
  }

  current_cursor_ = cursor;
  UpdateCursorOnWebView();
}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {
  if (is_loading == is_loading_) {
    return;
  }

  is_loading_ = is_loading;
  UpdateCursorOnWebView();
}

void RenderWidgetHostView::ImeCancelComposition() {
  if (!delegate_) {
    return;
  }

  delegate_->ImeCancelComposition();
}

void RenderWidgetHostView::RenderProcessGone(base::TerminationStatus status,
                                             int error_code) {
  Destroy();
}

void RenderWidgetHostView::Destroy() {
  host_ = NULL;
  delete this;
}

void RenderWidgetHostView::SetTooltipText(const base::string16& tooltip_text) {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback,
    const SkColorType color_type) {
  callback.Run(false, SkBitmap());
}

void RenderWidgetHostView::CopyFromCompositingSurfaceToVideoFrame(
    const gfx::Rect& src_subrect,
    const scoped_refptr<media::VideoFrame>& target,
    const base::Callback<void(bool)>& callback) {
  NOTIMPLEMENTED();
  callback.Run(false);
}

bool RenderWidgetHostView::CanCopyToVideoFrame() const {
  return false;
}

bool RenderWidgetHostView::HasAcceleratedSurface(
    const gfx::Size& desired_size) {
  return false;
}

void RenderWidgetHostView::GetScreenInfo(blink::WebScreenInfo* result) {
  if (!delegate_) {
    *result = BrowserProcessMain::GetInstance()->GetDefaultScreenInfo();
    return;
  }

  *result = delegate_->GetWebView()->GetScreenInfo();
}

gfx::Rect RenderWidgetHostView::GetBoundsInRootWindow() {
  return GetViewBounds();
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface() {
  if (shared_surface_handle_.is_null()) {
    shared_surface_handle_ =
        CompositorUtils::GetInstance()->GetSharedSurfaceHandle();
  }

  return shared_surface_handle_;
}

void RenderWidgetHostView::ShowDisambiguationPopup(
    const gfx::Rect& rect_pixels,
    const SkBitmap& zoomed_bitmap) {}

void RenderWidgetHostView::ProcessAckedTouchEvent(
    const content::TouchEventWithLatencyInfo& touch,
    content::InputEventAckState ack_result) {
  if (!delegate_) {
    return;
  }

  delegate_->ProcessAckedTouchEvent(
      ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED);
}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {
  NOTREACHED() << "InitAsChild() isn't used. Please use Init() instead";
}

gfx::Vector2dF RenderWidgetHostView::GetLastScrollOffset() const {
  NOTREACHED();
  return gfx::Vector2dF();
}

gfx::NativeView RenderWidgetHostView::GetNativeView() const {
  return NULL;
}

gfx::NativeViewId RenderWidgetHostView::GetNativeViewId() const {
  return 0;
}

gfx::NativeViewAccessible RenderWidgetHostView::GetNativeViewAccessible() {
  return NULL;
}

bool RenderWidgetHostView::HasFocus() const {
  if (!delegate_) {
    return false;
  }

  return delegate_->GetWebView()->HasFocus();
}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

void RenderWidgetHostView::Show() {
  if (is_showing_) {
    DCHECK(delegate_);
    return;
  }
  if (!delegate_) {
    return;
  }

  is_showing_ = true;

  WasShown();
}

void RenderWidgetHostView::Hide() {
  if (!is_showing_) {
    return;
  }

  is_showing_ = false;

  WasHidden();
}

bool RenderWidgetHostView::IsShowing() {
  DCHECK(!is_showing_ || delegate_);
  return is_showing_;
}

gfx::Rect RenderWidgetHostView::GetViewBounds() const {
  if (!delegate_) {
    return gfx::Rect(last_size_);
  }

  return delegate_->GetWebView()->GetViewBoundsDip();
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::UnusedResourcesAreAvailable() {
  if (ack_callbacks_.empty()) {
    SendReturnedDelegatedResources();
  }
}

void RenderWidgetHostView::EvictCurrentFrame() {
  frame_is_evicted_ = true;
  DestroyDelegatedContent();

  if (delegate_) {
    delegate_->EvictCurrentFrame();
  }
}

void RenderWidgetHostView::UpdateCursorOnWebView() {
  if (!delegate_) {
    return;
  }

  if (is_loading_) {
    content::WebCursor::CursorInfo busy_cursor_info(
        blink::WebCursorInfo::TypeWait);
    content::WebCursor busy_cursor(busy_cursor_info);
    delegate_->UpdateCursor(busy_cursor);
  } else {
    delegate_->UpdateCursor(current_cursor_);
  }
}

void RenderWidgetHostView::DestroyDelegatedContent() {
  DetachLayer();
  frame_provider_ = NULL;
  layer_ = NULL;
}

void RenderWidgetHostView::SendDelegatedFrameAck(uint32 surface_id) {
  cc::CompositorFrameAck ack;
  resource_collection_->TakeUnusedResourcesForChildCompositor(&ack.resources);

  content::RenderWidgetHostImpl::SendSwapCompositorFrameAck(
      host_->GetRoutingID(),
      surface_id,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::SendReturnedDelegatedResources() {
  cc::CompositorFrameAck ack;
  resource_collection_->TakeUnusedResourcesForChildCompositor(&ack.resources);

  DCHECK(!ack.resources.empty());

  content::RenderWidgetHostImpl::SendReclaimCompositorResources(
      host_->GetRoutingID(),
      last_output_surface_id_,
      host_->GetProcess()->GetID(),
      ack);
}

void RenderWidgetHostView::RunAckCallbacks() {
  while (!ack_callbacks_.empty()) {
    ack_callbacks_.front().Run();
    ack_callbacks_.pop();
  }
}

void RenderWidgetHostView::AttachLayer() {
  if (!delegate_) {
    return;
  }
  if (!layer_.get()) {
    return;
  }

  delegate_->GetCompositor()->SetRootLayer(layer_.get());
}

void RenderWidgetHostView::DetachLayer() {
  if (!delegate_) {
    return;
  }
  if (!layer_.get()) {
    return;
  }

  delegate_->GetCompositor()->SetRootLayer(scoped_refptr<cc::Layer>());
}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    host_(content::RenderWidgetHostImpl::From(host)),
    delegate_(NULL),
    resource_collection_(new cc::DelegatedFrameResourceCollection()),
    last_output_surface_id_(0),
    frame_is_evicted_(true),
    selection_cursor_position_(0),
    selection_anchor_position_(0),
    current_text_input_type_(ui::TEXT_INPUT_TYPE_NONE),
    show_ime_if_needed_(false),
    focused_node_is_editable_(false),
    is_loading_(false),
    is_showing_(false) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  resource_collection_->SetClient(this);
  host_->SetView(this);
}

RenderWidgetHostView::~RenderWidgetHostView() {
  DCHECK(ack_callbacks_.empty());
  resource_collection_->SetClient(NULL);
  SetDelegate(NULL);
}

void RenderWidgetHostView::CompositorDidCommit() {
  RunAckCallbacks();
}

void RenderWidgetHostView::SetDelegate(
    RenderWidgetHostViewDelegate* delegate) {
  if (delegate == delegate_) {
    return;
  }

  DetachLayer();
  delegate_ = delegate;
  AttachLayer();

  if (delegate_) {
    DCHECK(host_) <<
        "Shouldn't be attaching to a delegate when we're already destroyed";
    host_->SendScreenRects();
    host_->WasResized();

    if (delegate_->GetWebView()->IsVisible()) {
      Show();
    } else {
      Hide();
    }

    UpdateCursorOnWebView();
    delegate_->TextInputStateChanged(current_text_input_type_,
                                     show_ime_if_needed_);
    delegate_->FocusedNodeChanged(focused_node_is_editable_);
    delegate_->SelectionBoundsChanged(caret_rect_,
                                      selection_cursor_position_,
                                      selection_anchor_position_);
    delegate_->SelectionChanged();
  } else if (host_) {
    Hide();
  }
}

void RenderWidgetHostView::Blur() {
  host_->SetInputMethodActive(false);

  host_->SetActive(false);
  host_->Blur();
}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  last_size_ = size;
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
}

void RenderWidgetHostView::Focus() {
  host_->Focus();
  host_->SetActive(true);

  // XXX: Should we have a run-time check to see if this is required?
  host_->SetInputMethodActive(true);
}

} // namespace oxide
