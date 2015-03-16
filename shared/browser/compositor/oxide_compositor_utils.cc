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

#include <map>
#include <set>
#include <utility>

#include "base/bind.h"
#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/singleton.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/output/context_provider.h"
#include "cc/output/output_surface.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "ui/gl/gl_surface_egl.h"

#include "shared/browser/oxide_browser_platform_integration.h"
#include "shared/common/oxide_id_allocator.h"
#include "shared/port/content/common/gpu_thread_shim_oxide.h"

#include "oxide_compositor_frame_handle.h"

namespace oxide {

namespace {
void WakeUpGpuThread() {}
}

typedef std::pair<content::oxide_gpu_shim::Texture*, int> TextureHolder;

class GLFrameHandle : public GLFrameData {
 public:
  GLFrameHandle(const gpu::Mailbox& mailbox,
                GLuint texture_id,
                cc::ContextProvider* context_provider)
      : GLFrameData(mailbox, texture_id),
        context_provider_(context_provider) {}

  ~GLFrameHandle() override;

 private:
  scoped_refptr<cc::ContextProvider> context_provider_;
};

class ImageFrameHandle : public ImageFrameData {
 public:
  ImageFrameHandle(const gpu::Mailbox& mailbox,
                   EGLImageKHR image,
                   cc::ContextProvider* context_provider)
      : ImageFrameData(mailbox, image),
        context_provider_(context_provider) {}

  ~ImageFrameHandle() override;

 private:
  scoped_refptr<cc::ContextProvider> context_provider_;
};

struct FetchTextureResourcesTaskInfo {
  FetchTextureResourcesTaskInfo(
      int32 client_id,
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CompositorUtils::CreateGLFrameHandleCallback& callback,
      base::SingleThreadTaskRunner* task_runner)
      : client_id(client_id),
        route_id(-1),
        context_provider(context_provider),
        mailbox(mailbox),
        sync_point(sync_point),
        task_runner(task_runner),
        want_egl_image(false) {
    route_id = content::oxide_gpu_shim::GetContextProviderRouteID(
        static_cast<content::ContextProviderCommandBuffer*>(
          this->context_provider.get()));
    gl.callback = callback;
    gl.service_id = 0;
  }

  FetchTextureResourcesTaskInfo(
      int32 client_id,
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CompositorUtils::CreateImageFrameHandleCallback& callback,
      base::SingleThreadTaskRunner* task_runner)
      : client_id(client_id),
        route_id(-1),
        context_provider(context_provider),
        mailbox(mailbox),
        sync_point(sync_point),
        task_runner(task_runner),
        want_egl_image(true) {
    route_id = content::oxide_gpu_shim::GetContextProviderRouteID(
        static_cast<content::ContextProviderCommandBuffer*>(
          this->context_provider.get()));
    image.callback = callback;
    image.egl_image = EGL_NO_IMAGE_KHR;
  }

  ~FetchTextureResourcesTaskInfo();

  int32 client_id;
  int32 route_id;
  scoped_refptr<cc::ContextProvider> context_provider;
  gpu::Mailbox mailbox;
  uint32 sync_point;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner;
  bool want_egl_image;

  struct {
    CompositorUtils::CreateGLFrameHandleCallback callback;
    GLuint service_id;
  } gl;
  struct {
    CompositorUtils::CreateImageFrameHandleCallback callback;
    EGLImageKHR egl_image;
  } image;
};

class CompositorThread : public base::Thread {
 public:
  CompositorThread();
  ~CompositorThread() override;

 private:
  // base::Thread implementation
  void Init() override;
};

class CompositorUtilsImpl
    : public CompositorUtils,
      public base::MessageLoop::TaskObserver,
      public gpu::gles2::TextureManager::DestructionObserver {
 public:
  static CompositorUtilsImpl* GetInstance();

  // CompositorUtils implementation
  void Initialize(bool has_share_context) override;
  void Shutdown() override;
  scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner() override;
  void CreateGLFrameHandle(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) override;
  void CreateImageFrameHandle(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateImageFrameHandleCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) override;
  gfx::GLSurfaceHandle GetSharedSurfaceHandle() override;
  bool CanUseGpuCompositing() const override;
  CompositingMode GetCompositingMode() const override;

  bool RefTexture(int32 client_id,
                  int32 route_id,
                  const gpu::Mailbox& mailbox);
  void UnrefTexture(const gpu::Mailbox& mailbox);
  GLuint GetTextureIDForMailbox(const gpu::Mailbox& mailbox);

  bool CalledOnMainOrCompositorThread() const;

 private:
  friend struct DefaultSingletonTraits<CompositorUtilsImpl>;

  CompositorUtilsImpl();
  ~CompositorUtilsImpl() override;

  void InitializeOnGpuThread();
  void ShutdownOnGpuThread(base::WaitableEvent* shutdown_event);

  void FetchTextureResourcesOnGpuThread(FetchTextureResourcesTaskInfo* info);
  void ContinueFetchTextureResourcesOnGpuThread(int id);
  void ContinueFetchTextureResourcesOnGpuThread_Locked(int id);

  void SendCreateFrameHandleResponse(int id);

  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) override;
  void DidProcessTask(const base::PendingTask& pending_task) override;

  // gpu::gles2::TextureManager::DestructionObserver implementation
  void OnTextureManagerDestroying(gpu::gles2::TextureManager* manager) override;
  void OnTextureRefDestroying(gpu::gles2::TextureRef* texture) override;

  // The client ID for this process's GPU channel
  int32 client_id_;

  // Whether or not we have a shared GL context
  bool has_share_context_;

  // ThreadChecker for the thread that called Initialize()
  base::ThreadChecker main_thread_checker_;

  base::ThreadChecker gpu_thread_checker_;

  // Used only for checking the task_runner provided to CreateGLFrameHandle
  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;

  scoped_refptr<base::SingleThreadTaskRunner> compositor_task_runner_;

  struct IncomingData {
    IncomingData()
        : fetch_pending(false),
          gpu_needs_wakeup(true),
          can_queue(false) {}

    base::Lock lock;
    bool fetch_pending;
    bool gpu_needs_wakeup;
    bool can_queue;
    std::queue<FetchTextureResourcesTaskInfo*> queue;
  } incoming_texture_resource_fetches_;

  struct {
    base::Lock lock;
    IdAllocator id_allocator;
    base::hash_map<int, FetchTextureResourcesTaskInfo*> info_map;
  } texture_resource_fetches_;

  struct MainData {
    scoped_ptr<CompositorThread> compositor_thread;
  } main_unsafe_access_;

  struct GpuData {
    GpuData() : has_shutdown(false) {}

    bool has_shutdown;
    std::map<gpu::Mailbox, TextureHolder> textures;
    std::set<gpu::gles2::TextureManager*> texture_managers;
  } gpu_unsafe_access_;

  MainData& main();
  GpuData& gpu();

  DISALLOW_COPY_AND_ASSIGN(CompositorUtilsImpl);
};

GLFrameHandle::~GLFrameHandle() {
  content::GpuChildThread::GetTaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&CompositorUtilsImpl::UnrefTexture,
                 base::Unretained(CompositorUtilsImpl::GetInstance()),
                 mailbox()));
}

ImageFrameHandle::~ImageFrameHandle() {
  content::GpuChildThread::GetTaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&CompositorUtilsImpl::UnrefTexture,
                 base::Unretained(CompositorUtilsImpl::GetInstance()),
                 mailbox()));
}

FetchTextureResourcesTaskInfo::~FetchTextureResourcesTaskInfo() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnMainOrCompositorThread());
  if (image.egl_image != EGL_NO_IMAGE_KHR) {
    eglDestroyImageKHR(gfx::GLSurfaceEGL::GetHardwareDisplay(),
                       image.egl_image);
  }
}

void CompositorThread::Init() {
  base::ThreadRestrictions::SetIOAllowed(false);
}

CompositorThread::CompositorThread()
    : base::Thread("Oxide_CompositorThread") {}

CompositorThread::~CompositorThread() {
  Stop();
}

CompositorUtilsImpl::CompositorUtilsImpl()
    : client_id_(-1),
      has_share_context_(false) {
  main_thread_checker_.DetachFromThread();
  gpu_thread_checker_.DetachFromThread();
}

CompositorUtilsImpl::~CompositorUtilsImpl() {}

void CompositorUtilsImpl::InitializeOnGpuThread() {
  DCHECK(content::GpuChildThread::GetTaskRunner()->BelongsToCurrentThread());
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  incoming_texture_resource_fetches_.gpu_needs_wakeup = false;

  base::MessageLoop::current()->AddTaskObserver(this);
}

void CompositorUtilsImpl::ShutdownOnGpuThread(
    base::WaitableEvent* shutdown_event) {
  DCHECK(!gpu().has_shutdown);

  base::MessageLoop::current()->RemoveTaskObserver(this);
  gpu().has_shutdown = true;

  shutdown_event->Signal();
  // |shutdown_event| might be deleted now
}

void CompositorUtilsImpl::FetchTextureResourcesOnGpuThread(
    FetchTextureResourcesTaskInfo* info) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  texture_resource_fetches_.lock.AssertAcquired();

  int id = texture_resource_fetches_.id_allocator.AllocateId();
  DCHECK_NE(id, kInvalidId);

  DCHECK(texture_resource_fetches_.info_map.find(id) ==
         texture_resource_fetches_.info_map.end());
  texture_resource_fetches_.info_map[id] = info;

  gpu::SyncPointManager* sync_point_manager =
      content::GpuChildThread::GetChannelManager()->sync_point_manager();
  if (sync_point_manager->IsSyncPointRetired(info->sync_point)) {
    ContinueFetchTextureResourcesOnGpuThread_Locked(id);
    return;
  }

  sync_point_manager->AddSyncPointCallback(
      info->sync_point,
      base::Bind(&CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread,
                 base::Unretained(this), id));
}

void CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread(int id) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  if (gpu().has_shutdown) {
    return;
  }

  base::AutoLock lock(texture_resource_fetches_.lock);
  ContinueFetchTextureResourcesOnGpuThread_Locked(id);
}

void CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread_Locked(
    int id) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  texture_resource_fetches_.lock.AssertAcquired();

  auto it = texture_resource_fetches_.info_map.find(id);
  DCHECK(it != texture_resource_fetches_.info_map.end());

  FetchTextureResourcesTaskInfo* info = it->second;

  GLuint service_id = 0;
  if (RefTexture(info->client_id, info->route_id, info->mailbox)) {
    service_id = GetTextureIDForMailbox(info->mailbox);
  }

  if (info->want_egl_image) {
    if (service_id > 0) {
      // TODO(chrisccoulson): Do we need to do this on every frame?
      info->image.egl_image =
          content::oxide_gpu_shim::CreateImageFromTexture(
            info->client_id,
            info->route_id,
            service_id);
      if (info->image.egl_image == EGL_NO_IMAGE_KHR) {
        UnrefTexture(info->mailbox);
      }
    }
  } else {
    info->gl.service_id = service_id;
  }

  // If the destination thread has quit already, this task may never run.
  // That's ok though - at some point, the TextureManager owned by the
  // ContextGroup that this texture belongs to will be deleted, and we drop
  // our texture ref there
  info->task_runner->PostTask(
      FROM_HERE,
      base::Bind(&CompositorUtilsImpl::SendCreateFrameHandleResponse,
                 base::Unretained(this), id));
}

void CompositorUtilsImpl::SendCreateFrameHandleResponse(int id) {
  DCHECK(CalledOnMainOrCompositorThread());

  scoped_ptr<GLFrameHandle> gl_handle;
  scoped_ptr<ImageFrameHandle> image_handle;
  CompositorUtils::CreateGLFrameHandleCallback gl_callback;
  CompositorUtils::CreateImageFrameHandleCallback image_callback;

  {
    base::AutoLock lock(texture_resource_fetches_.lock);

    auto it = texture_resource_fetches_.info_map.find(id);
    DCHECK(it != texture_resource_fetches_.info_map.end());

    FetchTextureResourcesTaskInfo* info = it->second;

    DCHECK(info->task_runner->RunsTasksOnCurrentThread());

    if (info->want_egl_image) {
      if (info->image.egl_image != EGL_NO_IMAGE_KHR) {
        image_handle.reset(new ImageFrameHandle(info->mailbox,
                                                info->image.egl_image,
                                                info->context_provider.get()));
        info->image.egl_image = EGL_NO_IMAGE_KHR;
      }
      image_callback = info->image.callback;
    } else {
      if (info->gl.service_id > 0) {
        // If we arrive here, then we're guaranteed to have an owning reference
        // to the underlying texture
        gl_handle.reset(new GLFrameHandle(info->mailbox,
                                          info->gl.service_id,
                                          info->context_provider.get()));
      }
      gl_callback = info->gl.callback;
    }

    texture_resource_fetches_.id_allocator.FreeId(id);
    delete info;
    texture_resource_fetches_.info_map.erase(it);
  }

  if (!gl_callback.is_null()) {
    gl_callback.Run(gl_handle.Pass());
  } else {
    DCHECK(!image_callback.is_null());
    image_callback.Run(image_handle.Pass());
  }
}

void CompositorUtilsImpl::WillProcessTask(
    const base::PendingTask& pending_task) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.gpu_needs_wakeup);
  incoming_texture_resource_fetches_.gpu_needs_wakeup = false;
}

void CompositorUtilsImpl::DidProcessTask(
    const base::PendingTask& pending_task) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  std::queue<FetchTextureResourcesTaskInfo*> queue;
  {
    base::AutoLock lock(incoming_texture_resource_fetches_.lock);
    DCHECK(!incoming_texture_resource_fetches_.gpu_needs_wakeup);
    incoming_texture_resource_fetches_.gpu_needs_wakeup = true;

    if (!incoming_texture_resource_fetches_.fetch_pending) {
      return;
    }
    incoming_texture_resource_fetches_.fetch_pending = false;

    std::swap(queue, incoming_texture_resource_fetches_.queue);
  }

  {
    base::AutoLock lock(texture_resource_fetches_.lock);

    while (!queue.empty()) {
      FetchTextureResourcesTaskInfo* info = queue.front();
      queue.pop();
      FetchTextureResourcesOnGpuThread(info);
    }
  }
}

void CompositorUtilsImpl::OnTextureManagerDestroying(
    gpu::gles2::TextureManager* manager) {
  if (gpu().texture_managers.erase(manager) == 0) {
    return;
  }

  for (auto it = gpu().textures.begin(); it != gpu().textures.end(); ) {
    if (it->second.first->GetTextureManager() != manager) {
      ++it;
      continue;
    }

    delete it->second.first;
    gpu().textures.erase(it++);
  }
}

void CompositorUtilsImpl::OnTextureRefDestroying(
    gpu::gles2::TextureRef* texture) {}

CompositorUtilsImpl::GpuData& CompositorUtilsImpl::gpu() {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  return gpu_unsafe_access_;
}

CompositorUtilsImpl::MainData& CompositorUtilsImpl::main() {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  return main_unsafe_access_;
}

// static
CompositorUtilsImpl* CompositorUtilsImpl::GetInstance() {
  return Singleton<CompositorUtilsImpl>::get();
}

void CompositorUtilsImpl::Initialize(bool has_share_context) {
  DCHECK(!main().compositor_thread);

  has_share_context_ = has_share_context;
  client_id_ =
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId();
  main_task_runner_ = base::MessageLoopProxy::current();

  main().compositor_thread.reset(new CompositorThread());
  main().compositor_thread->Start();

  compositor_task_runner_ = main().compositor_thread->message_loop_proxy();

  content::CauseForGpuLaunch cause =
      content::CAUSE_FOR_GPU_LAUNCH_WEBGRAPHICSCONTEXT3DCOMMANDBUFFERIMPL_INITIALIZE;
  scoped_refptr<content::GpuChannelHost> gpu_channel_host(
      content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(cause));
  if (gpu_channel_host.get()) {
    content::GpuChildThread::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtilsImpl::InitializeOnGpuThread,
                   base::Unretained(this)));
    base::AutoLock lock(incoming_texture_resource_fetches_.lock);
    incoming_texture_resource_fetches_.can_queue = true;
  }
}

void CompositorUtilsImpl::Shutdown() {
  DCHECK(main().compositor_thread);

  // Shut down the compositor thread first, to prevent more calls in to
  // CreateGLFrameHandle
  main().compositor_thread.reset();

  // Detach the GPU thread MessageLoop::TaskObserver, to stop processing
  // and existing incoming requests
  scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner =
      content::GpuChildThread::GetTaskRunner();
  if (gpu_task_runner.get()) {
    base::ThreadRestrictions::ScopedAllowWait allow_wait;
    base::WaitableEvent event(false, false);
    gpu_task_runner->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtilsImpl::ShutdownOnGpuThread,
                   base::Unretained(this), &event));
    event.Wait();
  }

  // Because we assert that CreateGLFrameHandle has to be called on the
  // current thread or the compositor thread, at this point we're guaranteed
  // to get no new incoming requests. It's safe to just delete any queued
  // incoming requests without a lock
  while (incoming_texture_resource_fetches_.queue.size() > 0) {
    FetchTextureResourcesTaskInfo* info =
        incoming_texture_resource_fetches_.queue.front();
    incoming_texture_resource_fetches_.queue.pop();
    delete info;
  }

  // We assert that the callback task runner passed to CreateGLFrameHandle
  // is for the current thread or the compositor thread, which means it's
  // guaranteed to not process any more tasks. It's safe to just delete
  // the pending requests without a lock
  for (auto it = texture_resource_fetches_.info_map.begin();
       it != texture_resource_fetches_.info_map.end(); ++it) {
    texture_resource_fetches_.id_allocator.FreeId(it->first);
    delete it->second;
  }
  texture_resource_fetches_.info_map.clear();
}

scoped_refptr<base::SingleThreadTaskRunner>
CompositorUtilsImpl::GetTaskRunner() {
  return compositor_task_runner_;
}

void CompositorUtilsImpl::CreateGLFrameHandle(
    cc::ContextProvider* context_provider,
    const gpu::Mailbox& mailbox,
    uint32 sync_point,
    const CreateGLFrameHandleCallback& callback,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  DCHECK(context_provider);
  DCHECK(!callback.is_null());
  DCHECK(!mailbox.IsZero());
  DCHECK(task_runner);

  DCHECK(CalledOnMainOrCompositorThread());

  FetchTextureResourcesTaskInfo* info =
      new FetchTextureResourcesTaskInfo(
        client_id_,
        context_provider,
        mailbox,
        sync_point,
        callback,
        task_runner.get());

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.can_queue);

  if (!incoming_texture_resource_fetches_.fetch_pending) {
    incoming_texture_resource_fetches_.fetch_pending = true;
    if (incoming_texture_resource_fetches_.gpu_needs_wakeup) {
      // We assert |can_queue| above, so this should never fail
      content::GpuChildThread::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(&WakeUpGpuThread));
    }
  }

  incoming_texture_resource_fetches_.queue.push(info);
}

void CompositorUtilsImpl::CreateImageFrameHandle(
    cc::ContextProvider* context_provider,
    const gpu::Mailbox& mailbox,
    uint32 sync_point,
    const CreateImageFrameHandleCallback& callback,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  DCHECK(context_provider);
  DCHECK(!callback.is_null());
  DCHECK(!mailbox.IsZero());
  DCHECK(task_runner);

  DCHECK(CalledOnMainOrCompositorThread());

  FetchTextureResourcesTaskInfo* info =
      new FetchTextureResourcesTaskInfo(
        client_id_,
        context_provider,
        mailbox,
        sync_point,
        callback,
        task_runner.get());

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.can_queue);

  if (!incoming_texture_resource_fetches_.fetch_pending) {
    incoming_texture_resource_fetches_.fetch_pending = true;
    if (incoming_texture_resource_fetches_.gpu_needs_wakeup) {
      // We assert |can_queue| above, so this should never fail
      content::GpuChildThread::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(&WakeUpGpuThread));
    }
  }

  incoming_texture_resource_fetches_.queue.push(info);
}

gfx::GLSurfaceHandle CompositorUtilsImpl::GetSharedSurfaceHandle() {
  gfx::GLSurfaceHandle handle(gfx::kNullPluginWindow, gfx::NULL_TRANSPORT);
  handle.parent_client_id = client_id_;

  return handle;
}

bool CompositorUtilsImpl::CanUseGpuCompositing() const {
  return GetCompositingMode() != COMPOSITING_MODE_SOFTWARE;
}

CompositingMode CompositorUtilsImpl::GetCompositingMode() const {
  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return COMPOSITING_MODE_SOFTWARE;
  }

  if (has_share_context_) {
    return COMPOSITING_MODE_TEXTURE;
  }

  if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2) {
    // TODO(chrisccoulson): Make sure driver supports this
    return COMPOSITING_MODE_IMAGE;
  }

  return COMPOSITING_MODE_SOFTWARE;
}

bool CompositorUtilsImpl::RefTexture(int32 client_id,
                                     int32 route_id,
                                     const gpu::Mailbox& mailbox) {
  if (gpu().has_shutdown) {
    return false;
  }

  auto it = gpu().textures.find(mailbox);
  if (it != gpu().textures.end()) {
    DCHECK_GT(it->second.second, 0);
    it->second.second++;
    return true;
  }

  content::oxide_gpu_shim::Texture* texture =
      content::oxide_gpu_shim::ConsumeTextureFromMailbox(client_id,
                                                         route_id,
                                                         mailbox);
  if (!texture) {
    return false;
  }

  gpu().textures[mailbox] = std::make_pair(texture, 1);

  gpu::gles2::TextureManager* texture_manager = texture->GetTextureManager();
  if (gpu().texture_managers.insert(texture_manager).second) {
    texture_manager->AddObserver(this);
  }

  return true;
}

void CompositorUtilsImpl::UnrefTexture(const gpu::Mailbox& mailbox) {
  auto it = gpu().textures.find(mailbox);
  if (it == gpu().textures.end()) {
    DCHECK(gpu().has_shutdown);
    return;
  }

  DCHECK_GT(it->second.second, 0);

  if (--it->second.second == 0 && it->second.first->Destroy()) {
    delete it->second.first;
    size_t removed = gpu().textures.erase(mailbox);
    DCHECK_GT(removed, 0U);
  }
}

GLuint CompositorUtilsImpl::GetTextureIDForMailbox(const gpu::Mailbox& mailbox) {
  if (gpu().has_shutdown) {
    return 0;
  }

  auto it = gpu().textures.find(mailbox);
  DCHECK(it != gpu().textures.end());
  return it->second.first->GetServiceID();
}

bool CompositorUtilsImpl::CalledOnMainOrCompositorThread() const {
  return main_thread_checker_.CalledOnValidThread() ||
         compositor_task_runner_->RunsTasksOnCurrentThread();
}

CompositorUtils::~CompositorUtils() {}

// static
CompositorUtils* CompositorUtils::GetInstance() {
  return CompositorUtilsImpl::GetInstance();
}

} // namespace oxide
