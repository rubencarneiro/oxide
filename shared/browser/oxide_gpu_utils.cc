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

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/common/gpu/client/gpu_channel_host.h"
#include "content/common/gpu/client/webgraphicscontext3d_command_buffer_impl.h"
#include "content/common/gpu/gpu_channel.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/common/gpu/gpu_process_launch_causes.h"
#include "content/gpu/gpu_child_thread.h"
#include "content/public/browser/browser_thread.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "url/gurl.h"

namespace oxide {

namespace {

GpuUtils* g_instance;

void ReleaseTextureRefOnGpuThread(gpu::gles2::TextureRef* ref) {
  DCHECK(content::GpuChildThread::message_loop_proxy()->RunsTasksOnCurrentThread());
  ref->Release();
}

}

// static
void TextureHandleTraits::Destruct(const TextureHandle* x) {
  if (content::BrowserThread::CurrentlyOn(content::BrowserThread::UI)) {
    delete x;
    return;
  }

  if (!content::BrowserThread::DeleteSoon(content::BrowserThread::UI,
                                          FROM_HERE, x)) {
    LOG(ERROR) <<
        "TextureHandle won't be deleted. This could be due to it "
        "being leaked until after Chromium shutdown has begun";
  }
}

class TextureHandleImpl : public TextureHandle {
 public:
  TextureHandleImpl(int32 client_id, int32 route_id);
  ~TextureHandleImpl();

  virtual void Consume(const gpu::Mailbox& mailbox,
                       const gfx::Size& size) OVERRIDE;

  virtual gfx::Size GetSize() const OVERRIDE;
  virtual GLuint GetID() OVERRIDE;

 private:
  void Clear();
  void FetchTextureResourcesOnGpuThread();

  base::Lock lock_;
  base::ConditionVariable resources_available_;

  bool is_fetch_texture_resources_pending_;

  int32 client_id_;
  int32 route_id_;

  gpu::gles2::TextureRef* ref_;
  gpu::Mailbox mailbox_;
  gfx::Size size_;
  GLuint service_id_;
};

void TextureHandleImpl::Clear() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if (ref_) {
    content::GpuChildThread::message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ReleaseTextureRefOnGpuThread, base::Unretained(ref_)));
  }

  service_id_ = 0;
  mailbox_.SetZero();
  ref_ = NULL;
}

void TextureHandleImpl::FetchTextureResourcesOnGpuThread() {
  base::AutoLock lock(lock_);
  DCHECK(is_fetch_texture_resources_pending_);
  is_fetch_texture_resources_pending_ = false;

  if (mailbox_.IsZero()) {
    resources_available_.Signal();
    return;
  }

  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::instance()->gpu_channel_manager();
  gpu::gles2::Texture* texture =
      gpu_channel_manager->mailbox_manager()->ConsumeTexture(
        GL_TEXTURE_2D, mailbox_);

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
        service_id_ = texture->service_id();
      }
    }
  }

  resources_available_.Signal();
}

TextureHandleImpl::TextureHandleImpl(int32 client_id, int32 route_id) :
    resources_available_(&lock_),
    is_fetch_texture_resources_pending_(false),
    client_id_(client_id),
    route_id_(route_id),
    ref_(NULL),
    service_id_(0) {}

TextureHandleImpl::~TextureHandleImpl() {
  base::AutoLock lock(lock_);
  DCHECK(!is_fetch_texture_resources_pending_);
  Clear();
}

void TextureHandleImpl::Consume(const gpu::Mailbox& mailbox,
                                const gfx::Size& size) {
  if (mailbox == mailbox_ && size == size_) {
    return;
  }

  base::AutoLock lock(lock_);
  Clear();
  size_ = size;
  mailbox_ = mailbox;

  if (!is_fetch_texture_resources_pending_) {
    is_fetch_texture_resources_pending_ = true;
    content::GpuChildThread::message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&TextureHandleImpl::FetchTextureResourcesOnGpuThread, this));
  }
}

gfx::Size TextureHandleImpl::GetSize() const {
  return size_;
}

GLuint TextureHandleImpl::GetID() {
  base::AutoLock lock(lock_);
  if (is_fetch_texture_resources_pending_) {
    resources_available_.Wait();
    DCHECK(!is_fetch_texture_resources_pending_);
  }

  return service_id_;
}

GpuUtils::GpuUtils() {
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

// static
void GpuUtils::Initialize() {
  DCHECK(!g_instance);
  g_instance = new GpuUtils();
}

// static
void GpuUtils::Terminate() {
  DCHECK(g_instance);
  delete g_instance;
}

// static
GpuUtils* GpuUtils::instance() {
  return g_instance;
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

TextureHandle* GpuUtils::CreateTextureHandle() {
  if (!offscreen_context_) {
    return NULL;
  }

  return new TextureHandleImpl(
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId(),
      offscreen_context_->GetCommandBufferProxy()->GetRouteID());
}

} // namespace oxide
