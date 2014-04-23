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

#include "oxide_gpu_utils.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/gl_frame_data.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/common/gpu/client/gpu_channel_host.h"
#include "content/common/gpu/client/webgraphicscontext3d_command_buffer_impl.h"
#include "content/common/gpu/gpu_channel.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/common/gpu/gpu_process_launch_causes.h"
#include "content/common/gpu/sync_point_manager.h"
#include "content/gpu/gpu_child_thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "url/gurl.h"

#include "oxide_render_widget_host_view.h"

namespace oxide {

namespace {

base::LazyInstance<scoped_refptr<GpuUtils> > g_instance =
    LAZY_INSTANCE_INITIALIZER;

content::GpuCommandBufferStub* LookupCommandBuffer(int32 client_id,
                                                   int32 route_id) {
  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::instance()->gpu_channel_manager();
  content::GpuChannel* channel =
      gpu_channel_manager->LookupChannel(client_id);
  if (!channel) {
    return NULL;
  }

  return channel->LookupCommandBuffer(route_id);
}

}

GpuUtils::GpuUtils() :
    is_fetch_texture_resources_pending_(false) {
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

  GURL url("oxide://OffscreenGraphicsContext");

  offscreen_context_.reset(new WGC3DCBI(
      0, url, gpu_channel_host.get(), attrs, false,
      content::WebGraphicsContext3DCommandBufferImpl::SharedMemoryLimits(),
      NULL));
  offscreen_context_->makeContextCurrent();
}

GpuUtils::~GpuUtils() {}

bool GpuUtils::FetchTextureResources(AcceleratedFrameHandle* handle) {
  base::AutoLock lock(fetch_texture_resources_lock_);

  if (!is_fetch_texture_resources_pending_) {
    if (!content::GpuChildThread::message_loop_proxy()->PostTask(
            FROM_HERE,
            base::Bind(&GpuUtils::FetchTextureResourcesOnGpuThread, this))) {
      return false;
    }
    is_fetch_texture_resources_pending_ = true;
  }

  fetch_texture_resources_queue_.push(handle);
  return true;
}

void GpuUtils::FetchTextureResourcesOnGpuThread() {
  std::queue<scoped_refptr<AcceleratedFrameHandle> > queue;
  {
    base::AutoLock lock(fetch_texture_resources_lock_);
    is_fetch_texture_resources_pending_ = false;
    std::swap(queue, fetch_texture_resources_queue_);
  }

  while (!queue.empty()) {
    scoped_refptr<AcceleratedFrameHandle> handle = queue.front();
    queue.pop();
    handle->UpdateTextureResourcesOnGpuThread();
  }
}

// static
void GpuUtils::Initialize() {
  DCHECK(!g_instance.Get());
  g_instance.Get() = new GpuUtils();
}

void GpuUtils::Destroy() {
  offscreen_context_.reset();
}

// static
scoped_refptr<GpuUtils> GpuUtils::instance() {
  return g_instance.Get();
}

gfx::GLSurfaceHandle GpuUtils::GetSharedSurfaceHandle() {
  if (!offscreen_context_) {
    return gfx::GLSurfaceHandle();
  }

  gfx::GLSurfaceHandle handle(gfx::kNullPluginWindow, gfx::TEXTURE_TRANSPORT);
  handle.parent_client_id =
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId();

  return handle;
}

AcceleratedFrameHandle* GpuUtils::GetAcceleratedFrameHandle(
    RenderWidgetHostView* rwhv,
    uint32 surface_id,
    const gpu::Mailbox& mailbox,
    uint32 sync_point,
    const gfx::Size& size,
    float scale) {
  return new AcceleratedFrameHandle(
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId(),
      offscreen_context_->GetCommandBufferProxy()->GetRouteID(),
      rwhv, surface_id, mailbox, sync_point, size, scale);
}

GLuint AcceleratedFrameHandle::GetTextureID() {
  base::AutoLock lock(lock_);
  if (did_fetch_texture_resources_) {
    return service_id_;
  }

  resources_available_condition_.Wait();
  DCHECK(did_fetch_texture_resources_);
  return service_id_;  
}

void AcceleratedFrameHandle::WasFreed() {
  base::AutoLock lock(lock_);
  mailbox_.SetZero();
  service_id_ = 0;
}

AcceleratedFrameHandle::AcceleratedFrameHandle(
    int32 client_id,
    int32 route_id,
    RenderWidgetHostView* rwhv,
    uint32 surface_id,
    const gpu::Mailbox& mailbox,
    uint32 sync_point,
    const gfx::Size& size,
    float scale)
    : client_id_(client_id),
      route_id_(route_id),
      rwhv_(rwhv->AsWeakPtr()),
      surface_id_(surface_id),
      mailbox_(mailbox),
      sync_point_(sync_point),
      size_in_pixels_(size),
      device_scale_factor_(scale),
      resources_available_condition_(&lock_),
      did_fetch_texture_resources_(false),
      service_id_(0) {
  GpuUtils::instance()->FetchTextureResources(this);
}

AcceleratedFrameHandle::~AcceleratedFrameHandle() {
  if (!mailbox_.IsZero() && rwhv_ && rwhv_->host()) {
    cc::CompositorFrameAck ack;
    ack.gl_frame_data.reset(new cc::GLFrameData());
    ack.gl_frame_data->mailbox = mailbox_;
    ack.gl_frame_data->sync_point = 0;
    ack.gl_frame_data->size = size_in_pixels_;
    content::RenderWidgetHostImpl::SendReclaimCompositorResources(
        rwhv_->host()->GetRoutingID(),
        surface_id_,
        rwhv_->host()->GetProcess()->GetID(),
        ack);
  }
}

void AcceleratedFrameHandle::UpdateTextureResourcesOnGpuThread() {
  content::SyncPointManager* manager =
      content::GpuChildThread::instance()->gpu_channel_manager()->sync_point_manager();
  if (manager->IsSyncPointRetired(sync_point_)) {
    OnSyncPointRetired();
    return;
  }

  manager->AddSyncPointCallback(
      sync_point_,
      base::Bind(&AcceleratedFrameHandle::OnSyncPointRetired,
                 this));
}

void AcceleratedFrameHandle::OnSyncPointRetired() {
  base::AutoLock lock(lock_);
  DCHECK(!did_fetch_texture_resources_);
  did_fetch_texture_resources_ = true;

  if (mailbox_.IsZero()) {
    resources_available_condition_.Signal();
    return;
  }

  content::GpuCommandBufferStub* command_buffer =
      LookupCommandBuffer(client_id_, route_id_);
  if (command_buffer) {
    gpu::gles2::ContextGroup* group =
        command_buffer->decoder()->GetContextGroup();
    gpu::gles2::Texture* texture =
        group->mailbox_manager()->ConsumeTexture(GL_TEXTURE_2D, mailbox_);
    if (texture) {
      service_id_ = texture->service_id();
      // XXX(chrisccoulson): Do we leak |texture| here?
    }
  }

  resources_available_condition_.Signal();
}

} // namespace oxide
