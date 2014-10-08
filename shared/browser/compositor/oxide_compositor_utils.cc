// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_compositor_utils.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/output/context_provider.h"
#include "cc/output/output_surface.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/common/gpu/client/command_buffer_proxy_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "content/common/gpu/gpu_channel.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/common/gpu/sync_point_manager.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"

#include "oxide_compositor_frame_handle.h"

namespace oxide {

namespace {

void WakeUpGpuThread() {}

content::GpuCommandBufferStub* LookupCommandBuffer(int32 client_id,
                                                   int32 route_id) {
  DCHECK(content::GpuChildThread::instance());
  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::instance()->gpu_channel_manager();
  content::GpuChannel* channel =
      gpu_channel_manager->LookupChannel(client_id);
  if (!channel) {
    return NULL;
  }

  return channel->LookupCommandBuffer(route_id);
}

void ReleaseTextureRefOnGpuThread(gpu::gles2::TextureRef* ref,
                                  int32 client_id,
                                  int32 route_id) {
  if (content::GpuCommandBufferStub* command_buffer =
          LookupCommandBuffer(client_id, route_id)) {
    command_buffer->decoder()->MakeCurrent();
  }
  ref->Release();
}

void InitializeOnCompositorThread() {
  base::ThreadRestrictions::SetIOAllowed(false);
}

}

class GLFrameHandle : public GLFrameData {
 public:
  GLFrameHandle(const gpu::Mailbox& mailbox,
                GLuint texture_id,
                int32 client_id,
                int32 route_id,
                gpu::gles2::TextureRef* ref,
                const scoped_refptr<cc::ContextProvider>& context_provider)
      : GLFrameData(mailbox, texture_id),
        client_id_(client_id),
        route_id_(route_id),
        ref_(ref),
        context_provider_(context_provider) {}

  virtual ~GLFrameHandle() {
    content::GpuChildThread::message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ReleaseTextureRefOnGpuThread,
                   base::Unretained(ref_),
                   client_id_, route_id_));

    cc::ContextProvider* context_provider = context_provider_.get();
    context_provider->AddRef();
    context_provider_ = NULL;
    CompositorUtils::GetInstance()->GetTaskRunner()->ReleaseSoon(
        FROM_HERE, context_provider);
  }

 private:
  int32 client_id_;
  int32 route_id_;
  gpu::gles2::TextureRef* ref_;
  scoped_refptr<cc::ContextProvider> context_provider_;
};

class CompositorUtils::FetchTextureResourcesTask :
    public base::RefCountedThreadSafe<FetchTextureResourcesTask> {
 public:
  FetchTextureResourcesTask(
      int32 client_id,
      cc::OutputSurface* output_surface,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CompositorUtils::CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::TaskRunner> task_runner)
      : client_id_(client_id),
        route_id_(-1),
        context_provider_(output_surface->context_provider()),
        mailbox_(mailbox),
        sync_point_(sync_point),
        callback_(callback),
        task_runner_(task_runner) {
    DCHECK(task_runner_.get());
    DCHECK(!callback_.is_null());
    DCHECK(context_provider_.get());

    route_id_ =
        static_cast<content::ContextProviderCommandBuffer*>(
          context_provider_.get())->GetCommandBufferProxy()->GetRouteID();
  }

  virtual ~FetchTextureResourcesTask() {
    DCHECK(callback_.is_null());
    DCHECK(!context_provider_.get());
  }

  void FetchTextureResourcesOnGpuThread() {
    content::SyncPointManager* manager =
        content::GpuChildThread::instance()->gpu_channel_manager()->sync_point_manager();
    if (manager->IsSyncPointRetired(sync_point_)) {
      OnSyncPointRetired();
      return;
    }

    manager->AddSyncPointCallback(
        sync_point_,
        base::Bind(&FetchTextureResourcesTask::OnSyncPointRetired,
                   this));
  }

 private:
  void OnSyncPointRetired() {
    gpu::gles2::TextureRef* ref = NULL;
    GLuint service_id = 0;

    content::GpuCommandBufferStub* command_buffer =
        LookupCommandBuffer(client_id_, route_id_);
    if (command_buffer) {
      gpu::gles2::ContextGroup* group =
          command_buffer->decoder()->GetContextGroup();
      gpu::gles2::Texture* texture =
          group->mailbox_manager()->ConsumeTexture(GL_TEXTURE_2D, mailbox_);
      if (texture) {
        ref = new gpu::gles2::TextureRef(group->texture_manager(),
                                         client_id_,
                                         texture);
        ref->AddRef();
        service_id = texture->service_id();
      }
    }

    task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&FetchTextureResourcesTask::SendResponseOnOriginatingThread,
                   this, base::Unretained(ref), service_id));
  }

  void SendResponseOnOriginatingThread(gpu::gles2::TextureRef* ref,
                                       GLuint service_id) {
    if (!ref) {
      callback_.Run(scoped_ptr<GLFrameData>());
      return;
    }

    scoped_ptr<GLFrameHandle> handle(
        new GLFrameHandle(mailbox_,
                          service_id,
                          client_id_,
                          route_id_,
                          ref,
                          context_provider_));
    callback_.Run(handle.PassAs<GLFrameData>());

    callback_.Reset();
    context_provider_ = NULL;
  }

  int32 client_id_;
  int32 route_id_;
  scoped_refptr<cc::ContextProvider> context_provider_;
  gpu::Mailbox mailbox_;
  uint32 sync_point_;
  CompositorUtils::CreateGLFrameHandleCallback callback_;
  scoped_refptr<base::TaskRunner> task_runner_;
};

CompositorUtils::CompositorUtils()
    : client_id_(-1),
      fetch_texture_resources_pending_(false),
      gpu_thread_is_processing_task_(false),
      can_use_gpu_(false) {}

CompositorUtils::~CompositorUtils() {}

void CompositorUtils::InitializeOnGpuThread() {
  base::AutoLock lock(fetch_texture_resources_lock_);
  gpu_thread_is_processing_task_ = true;
  content::GpuChildThread::instance()->message_loop()->AddTaskObserver(this);
}

void CompositorUtils::WillProcessTask(const base::PendingTask& pending_task) {
  base::AutoLock lock(fetch_texture_resources_lock_);
  DCHECK(!gpu_thread_is_processing_task_);
  gpu_thread_is_processing_task_ = true;
}

void CompositorUtils::DidProcessTask(const base::PendingTask& pending_task) {
  std::queue<scoped_refptr<FetchTextureResourcesTask> > queue;
  {
    base::AutoLock lock(fetch_texture_resources_lock_);
    DCHECK(gpu_thread_is_processing_task_);
    gpu_thread_is_processing_task_ = false;
    if (!fetch_texture_resources_pending_) {
      return;
    }
    fetch_texture_resources_pending_ = false;
    std::swap(queue, fetch_texture_resources_queue_);
  }

  while (!queue.empty()) {
    scoped_refptr<FetchTextureResourcesTask> task = queue.front();
    queue.pop();
    task->FetchTextureResourcesOnGpuThread();
  }
}

// static
CompositorUtils* CompositorUtils::GetInstance() {
  return Singleton<CompositorUtils>::get();
}

void CompositorUtils::Initialize() {
  if (compositor_thread_) {
    return;
  }

  compositor_thread_.reset(new base::Thread("Oxide_CompositorThread"));
  compositor_thread_->Start();

  task_runner_ = compositor_thread_->message_loop_proxy();

  client_id_ =
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId();

  task_runner_->PostTask(
      FROM_HERE, base::Bind(&InitializeOnCompositorThread));

  content::CauseForGpuLaunch cause =
      content::CAUSE_FOR_GPU_LAUNCH_WEBGRAPHICSCONTEXT3DCOMMANDBUFFERIMPL_INITIALIZE;
  scoped_refptr<content::GpuChannelHost> gpu_channel_host(
      content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(cause));
  if (gpu_channel_host.get()) {
    can_use_gpu_ = true;
    content::GpuChildThread::message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtils::InitializeOnGpuThread,
                   base::Unretained(this)));
  }
}

void CompositorUtils::Destroy() {
  compositor_thread_.reset();
}

scoped_refptr<base::SingleThreadTaskRunner> CompositorUtils::GetTaskRunner() {
  return task_runner_;
}

void CompositorUtils::CreateGLFrameHandle(
    cc::OutputSurface* output_surface,
    const gpu::Mailbox& mailbox,
    uint32 sync_point,
    const CreateGLFrameHandleCallback& callback,
    scoped_refptr<base::TaskRunner> task_runner) {
  DCHECK(!mailbox.IsZero());
  DCHECK(can_use_gpu_);

  scoped_refptr<FetchTextureResourcesTask> task =
      new FetchTextureResourcesTask(
        client_id_, output_surface,
        mailbox, sync_point, callback, task_runner);

  base::AutoLock lock(fetch_texture_resources_lock_);

  if (!fetch_texture_resources_pending_) {
    fetch_texture_resources_pending_ = true;
    if (!gpu_thread_is_processing_task_ &&
        !content::GpuChildThread::message_loop_proxy()->PostTask(
          FROM_HERE, base::Bind(&WakeUpGpuThread))) {
      // FIXME: Send an error asynchronously
      return;
    }
  }

  fetch_texture_resources_queue_.push(task);
}

gfx::GLSurfaceHandle CompositorUtils::GetSharedSurfaceHandle() {
  gfx::GLSurfaceHandle handle(gfx::kNullPluginWindow, gfx::NULL_TRANSPORT);
  handle.parent_client_id = client_id_;

  return handle;
}

} // namespace oxide
