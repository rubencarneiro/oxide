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

#include "oxide_render_widget_host_view.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/common/gpu/client/gpu_channel_host.h"
#include "content/common/gpu/client/webgraphicscontext3d_command_buffer_impl.h"
#include "content/common/gpu/gpu_channel.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/gpu/gpu_process_launch_causes.h"
#include "content/gpu/gpu_child_thread.h"
#include "content/public/browser/browser_thread.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "ui/gfx/rect.h"
#include "url/gurl.h"

namespace oxide {

class OffscreenGraphicsContextRef;
namespace {
OffscreenGraphicsContextRef* g_offscreen_context;
}

class OffscreenGraphicsContextRef :
    public base::RefCounted<OffscreenGraphicsContextRef> {
 public:
  ~OffscreenGraphicsContextRef() {
    DCHECK_EQ(g_offscreen_context, this);
    g_offscreen_context = NULL;
  }

  static scoped_refptr<OffscreenGraphicsContextRef> GetInstance() {
    if (!g_offscreen_context) {
      scoped_refptr<OffscreenGraphicsContextRef> context =
          new OffscreenGraphicsContextRef();
      DCHECK(g_offscreen_context);
      return context;
    }

    return make_scoped_refptr(g_offscreen_context);
  }

  content::WebGraphicsContext3DCommandBufferImpl* context3d() const {
    return context3d_.get();
  }

 private:
  friend class base::RefCounted<OffscreenGraphicsContextRef>;

  OffscreenGraphicsContextRef() {
    DCHECK(!g_offscreen_context);
    g_offscreen_context = this;

    blink::WebGraphicsContext3D::Attributes attrs;
    attrs.shareResources = true;
    attrs.depth = false;
    attrs.stencil = false;
    attrs.antialias = false;
    attrs.noAutomaticFlushes = true;

    content::CauseForGpuLaunch cause =
        content::CAUSE_FOR_GPU_LAUNCH_WEBGRAPHICSCONTEXT3DCOMMANDBUFFERIMPL_INITIALIZE;
    scoped_refptr<content::GpuChannelHost> gpu_channel_host(
        content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(cause));
    if (!gpu_channel_host) {
      return;
    }

    GURL url("oxide://shared/OffscreenGraphicsContextRef::OffscreenGraphicsContextRef");
    bool use_echo_for_swap_ack = true;

    context3d_.reset(
      new content::WebGraphicsContext3DCommandBufferImpl(
          0,
          url,
          gpu_channel_host.get(),
          use_echo_for_swap_ack,
          attrs,
          false,
          content::WebGraphicsContext3DCommandBufferImpl::SharedMemoryLimits()));
    context3d_->makeContextCurrent();
  }

  scoped_ptr<content::WebGraphicsContext3DCommandBufferImpl> context3d_;
};

TextureInfo::TextureInfo(GLuint id, const gfx::Size& size_in_pixels) :
    id_(id),
    size_in_pixels_(size_in_pixels) {}

TextureInfo::~TextureInfo() {}

TextureHandle::GpuThreadCallbackContext::~GpuThreadCallbackContext() {
  base::AutoLock lock(lock_);
  CHECK(!handle_);
}

TextureHandle::GpuThreadCallbackContext::GpuThreadCallbackContext(
    TextureHandle* handle) :
    handle_(handle) {}

void TextureHandle::GpuThreadCallbackContext::Invalidate() {
  base::AutoLock lock(lock_);
  handle_ = NULL;
}

void TextureHandle::GpuThreadCallbackContext::FetchTextureResources() {
  base::AutoLock lock(lock_);
  if (!handle_) {
    return;
  }

  handle_->FetchTextureResourcesOnGpuThread();
}

void TextureHandle::FetchTextureResourcesOnGpuThread() {
  base::AutoLock lock(lock_);
  DCHECK(is_fetch_texture_resources_pending_);
  is_fetch_texture_resources_pending_ = false;

  if (mailbox_name_.empty()) {
    resources_available_.Signal();
    return;
  }

  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::instance()->gpu_channel_manager();
  gpu::gles2::Texture* texture =
      gpu_channel_manager->mailbox_manager()->ConsumeTexture(
        GL_TEXTURE_2D,
        *reinterpret_cast<const gpu::gles2::MailboxName *>(
          mailbox_name_.data()));

  if (texture) {
    content::GpuChannel* channel =
        gpu_channel_manager->LookupChannel(client_id_);
    if (channel) {
      content::GpuCommandBufferStub* command_buffer =
          channel->LookupCommandBuffer(route_id_);
      if (command_buffer) {
        ref_ = new gpu::gles2::TextureRef(
            command_buffer->decoder()->GetContextGroup()->texture_manager(),
            client_id_,
            texture);
        ref_->AddRef();
        id_ = texture->service_id();
      }
    }
  }

  resources_available_.Signal();
}

void TextureHandle::ReleaseTextureRef() {
  if (ref_) {
    content::GpuChildThread::instance()->message_loop()->PostTask(
        FROM_HERE,
        base::Bind(&TextureHandle::ReleaseTextureRefOnGpuThread,
                   base::Unretained(ref_)));
  }

  id_ = 0;
  ref_ = NULL;
  mailbox_name_.clear();
}

// static
void TextureHandle::ReleaseTextureRefOnGpuThread(gpu::gles2::TextureRef* ref) {
  DCHECK_EQ(base::MessageLoop::current(),
            content::GpuChildThread::instance()->message_loop());
  ref->Release();
}

TextureHandle::TextureHandle() :
    resources_available_(&lock_),
    client_id_(-1),
    route_id_(-1),
    is_fetch_texture_resources_pending_(false),
    id_(0),
    ref_(NULL),
    callback_context_(new GpuThreadCallbackContext(this)) {}

TextureHandle::~TextureHandle() {
  callback_context_->Invalidate();
  ReleaseTextureRef();
}

void TextureHandle::Initialize(
    content::WebGraphicsContext3DCommandBufferImpl* context) {
  DCHECK(client_id_ == -1 && route_id_ == -1);
  client_id_ = context->GetChannelID();
  route_id_ = context->GetContextID();
}

void TextureHandle::Update(const std::string& name,
                           const gfx::Size& size_in_pixels) {
  DCHECK(client_id_ != -1 && route_id_ != -1);

  size_in_pixels_ = size_in_pixels;

  if (name == mailbox_name_) {
    return;
  }

  base::AutoLock lock(lock_);
  ReleaseTextureRef();
  mailbox_name_ = name;

  if (!is_fetch_texture_resources_pending_) {
    is_fetch_texture_resources_pending_ = true;
    content::GpuChildThread::instance()->message_loop()->PostTask(
        FROM_HERE,
        base::Bind(&TextureHandle::GpuThreadCallbackContext::FetchTextureResources,
                   callback_context_));
  }
}

TextureInfo TextureHandle::GetTextureInfo() {
  base::AutoLock lock(lock_);
  if (is_fetch_texture_resources_pending_) {
    resources_available_.Wait();
    DCHECK(!is_fetch_texture_resources_pending_);
  }

  return TextureInfo(id_, size_in_pixels_);
}

void RenderWidgetHostView::Paint(const gfx::Rect& dirty_rect) {}

void RenderWidgetHostView::BuffersSwapped(
    const AcknowledgeBufferPresentCallback& ack) {
  SendAcknowledgeBufferPresent(ack, true);
}

void RenderWidgetHostView::SendAcknowledgeBufferPresentImpl(
    int32 route_id,
    int gpu_host_id,
    const std::string& mailbox_name,
    bool skipped) {
  AcceleratedSurfaceMsg_BufferPresented_Params ack;
  ack.sync_point = 0;
  if (skipped) {
    ack.mailbox_name = mailbox_name;
    std::swap(backbuffer_texture_handle_, frontbuffer_texture_handle_);
  }

  content::RenderWidgetHostImpl::AcknowledgeBufferPresent(
      route_id,
      gpu_host_id,
      ack);
}

// static
void RenderWidgetHostView::SendAcknowledgeBufferPresentOnMainThread(
    const AcknowledgeBufferPresentCallback& ack,
    bool skipped) {
  ack.Run(skipped);
}

RenderWidgetHostView::RenderWidgetHostView(content::RenderWidgetHost* host) :
    content::RenderWidgetHostViewBase(),
    is_hidden_(false),
    host_(content::RenderWidgetHostImpl::From(host)),
    graphics_context_ref_(OffscreenGraphicsContextRef::GetInstance()),
    frontbuffer_texture_handle_(&texture_handles_[0]),
    backbuffer_texture_handle_(&texture_handles_[1]) {
  CHECK(host_) << "Implementation didn't supply a RenderWidgetHost";

  frontbuffer_texture_handle_->Initialize(graphics_context_ref_->context3d());
  backbuffer_texture_handle_->Initialize(graphics_context_ref_->context3d());

  host_->SetView(this);
}

// static
void RenderWidgetHostView::SendAcknowledgeBufferPresent(
    const AcknowledgeBufferPresentCallback& ack,
    bool skipped) {
  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    SendAcknowledgeBufferPresentOnMainThread(ack, skipped);
    return;
  }

  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresentOnMainThread,
                 ack, skipped));  
}

RenderWidgetHostView::~RenderWidgetHostView() {}

void RenderWidgetHostView::InitAsPopup(
    content::RenderWidgetHostView* parent_host_view,
    const gfx::Rect& pos) {
  NOTIMPLEMENTED() <<
      "InitAsPopup() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::InitAsFullscreen(
    content::RenderWidgetHostView* reference_host_view) {
  NOTIMPLEMENTED() <<
      "InitAsFullScreen() shouldn't be called until "
      "WebContentsViewPort::CreateViewForPopupWidget() is implemented";
}

void RenderWidgetHostView::WasShown() {
  if (!is_hidden_) {
    return;
  }

  is_hidden_ = false;

  GetRenderWidgetHostImpl()->WasShown();
}

void RenderWidgetHostView::WasHidden() {
  if (is_hidden_) {
    return;
  }

  is_hidden_ = true;

  GetRenderWidgetHostImpl()->WasHidden();
}

void RenderWidgetHostView::MovePluginWindows(
    const gfx::Vector2d& scroll_offset,
    const std::vector<content::WebPluginGeometry>& moves) {}

void RenderWidgetHostView::Blur() {}

void RenderWidgetHostView::UpdateCursor(const WebCursor& cursor) {}

void RenderWidgetHostView::SetIsLoading(bool is_loading) {}

void RenderWidgetHostView::TextInputTypeChanged(ui::TextInputType type,
                                                ui::TextInputMode mode,
                                                bool can_compose_inline) {}

void RenderWidgetHostView::ImeCancelComposition() {}

void RenderWidgetHostView::ImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::vector<gfx::Rect>& character_bounds) {}

void RenderWidgetHostView::DidUpdateBackingStore(
    const gfx::Rect& scroll_rect,
    const gfx::Vector2d& scroll_delta,
    const std::vector<gfx::Rect>& copy_rects,
    const ui::LatencyInfo& latency_info) {
  if (is_hidden_) {
    return;
  }

  Paint(scroll_rect);

  for (size_t i = 0; i < copy_rects.size(); ++i) {
    gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
    if (rect.IsEmpty()) {
      continue;
    }

    Paint(rect);
  }
}

void RenderWidgetHostView::RenderProcessGone(base::TerminationStatus status,
                                             int error_code) {
  Destroy();
}

void RenderWidgetHostView::Destroy() {
  host_ = NULL;
  delete this;
}

void RenderWidgetHostView::SetTooltipText(const string16& tooltip_text) {}

void RenderWidgetHostView::SelectionBoundsChanged(
    const ViewHostMsg_SelectionBounds_Params& params) {}

void RenderWidgetHostView::ScrollOffsetChanged() {}

void RenderWidgetHostView::CopyFromCompositingSurface(
    const gfx::Rect& src_subrect,
    const gfx::Size& dst_size,
    const base::Callback<void(bool, const SkBitmap&)>& callback) {
  GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
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

void RenderWidgetHostView::OnAcceleratedCompositingStateChange() {}

void RenderWidgetHostView::AcceleratedSurfaceInitialized(
    int host_id, int route_id) {}

void RenderWidgetHostView::AcceleratedSurfaceBuffersSwapped(
    const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params_in_pixel,
    int gpu_host_id) {
  std::swap(backbuffer_texture_handle_, frontbuffer_texture_handle_);
  frontbuffer_texture_handle_->Update(params_in_pixel.mailbox_name,
                                      params_in_pixel.size);

  AcknowledgeBufferPresentCallback ack(
      base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresentImpl,
                 AsWeakPtr(),
                 params_in_pixel.route_id,
                 gpu_host_id,
                 params_in_pixel.mailbox_name));

  // FIXME: GetViewBounds() should be in DIP
  //        See https://launchpad.net/bugs/1257721
  if (params_in_pixel.size != GetViewBounds().size()) {
    SendAcknowledgeBufferPresent(ack, true);
    return;
  }

  BuffersSwapped(ack);
}

void RenderWidgetHostView::AcceleratedSurfacePostSubBuffer(
      const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params_in_pixel,
      int gpu_host_id) {
  NOTIMPLEMENTED() << "PostSubBuffer is not implemented";

  AcknowledgeBufferPresentCallback ack(
      base::Bind(&RenderWidgetHostView::SendAcknowledgeBufferPresentImpl,
                 AsWeakPtr(),
                 params_in_pixel.route_id,
                 gpu_host_id,
                 params_in_pixel.mailbox_name));
  SendAcknowledgeBufferPresent(ack, true);
}

void RenderWidgetHostView::AcceleratedSurfaceSuspend() {}

void RenderWidgetHostView::AcceleratedSurfaceRelease() {}

bool RenderWidgetHostView::HasAcceleratedSurface(
    const gfx::Size& desired_size) {
  return false;
}

gfx::Size RenderWidgetHostView::GetPhysicalBackingSize() const {
  // XXX: This default implementation assumes a scale factor of 1.0
  //      Implementations that change this for DPI-awareness need
  //      to also set WebScreenInfo::deviceScaleFactor in
  //      GetScreenInfo()
  return GetViewBounds().size();
}

gfx::GLSurfaceHandle RenderWidgetHostView::GetCompositingSurface() {
  if (shared_surface_handle_.is_null()) {
    content::WebGraphicsContext3DCommandBufferImpl* context =
      graphics_context_ref_->context3d();
    if (!context) {
      return gfx::GLSurfaceHandle();
    }

    shared_surface_handle_ = gfx::GLSurfaceHandle(
        gfx::kNullPluginWindow, gfx::TEXTURE_TRANSPORT);
    shared_surface_handle_.parent_gpu_process_id = context->GetGPUProcessID();
    shared_surface_handle_.parent_client_id = context->GetChannelID();
  }

  return shared_surface_handle_;
}

void RenderWidgetHostView::SetHasHorizontalScrollbar(
    bool has_horizontal_scrollbar) {}

void RenderWidgetHostView::SetScrollOffsetPinning(bool is_pinned_to_left,
                                                  bool is_pinned_to_right) {}

void RenderWidgetHostView::OnAccessibilityEvents(
    const std::vector<AccessibilityHostMsg_EventParams>& params) {}

void RenderWidgetHostView::InitAsChild(gfx::NativeView parent_view) {}

content::RenderWidgetHost* RenderWidgetHostView::GetRenderWidgetHost() const {
  return host_;
}

void RenderWidgetHostView::SetSize(const gfx::Size& size) {
  GetRenderWidgetHostImpl()->SendScreenRects();
  GetRenderWidgetHost()->WasResized();
}

void RenderWidgetHostView::SetBounds(const gfx::Rect& rect) {
  SetSize(rect.size());
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

void RenderWidgetHostView::Focus() {}

bool RenderWidgetHostView::IsSurfaceAvailableForCopy() const {
  return true;
}

bool RenderWidgetHostView::LockMouse() {
  return false;
}

void RenderWidgetHostView::UnlockMouse() {}

void RenderWidgetHostView::OnFocus() {
  GetRenderWidgetHostImpl()->GotFocus();
  GetRenderWidgetHost()->SetActive(true);
}

void RenderWidgetHostView::OnBlur() {
  GetRenderWidgetHost()->SetActive(false);
  GetRenderWidgetHost()->Blur();
}

TextureInfo RenderWidgetHostView::GetFrontbufferTextureInfo() {
  return frontbuffer_texture_handle_->GetTextureInfo();
}

} // namespace oxide
