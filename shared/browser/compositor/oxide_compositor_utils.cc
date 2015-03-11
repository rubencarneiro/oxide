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
                const scoped_refptr<cc::ContextProvider>& context_provider)
      : GLFrameData(mailbox, texture_id),
        context_provider_(context_provider) {}

  virtual ~GLFrameHandle() override;

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
        callback(callback),
        task_runner(task_runner) {
    route_id = content::oxide_gpu_shim::GetContextProviderRouteID(
        static_cast<content::ContextProviderCommandBuffer*>(
          this->context_provider.get()));
  }

  ~FetchTextureResourcesTaskInfo();

  int32 client_id;
  int32 route_id;
  scoped_refptr<cc::ContextProvider> context_provider;
  gpu::Mailbox mailbox;
  uint32 sync_point;
  CompositorUtils::CreateGLFrameHandleCallback callback;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner;
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
  void Initialize() override;
  void Shutdown() override;
  scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner() override;
  void CreateGLFrameHandle(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner) override;
  gfx::GLSurfaceHandle GetSharedSurfaceHandle() override;
  bool CanUseGpuCompositing() override;

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
  void ShutdownOnGpuThread();

  void FetchTextureResourcesOnGpuThread(FetchTextureResourcesTaskInfo* info);
  void ContinueFetchTextureResourcesOnGpuThread(int id);
  void ContinueFetchTextureResourcesOnGpuThread_Locked(int id);

  void SendCreateGLFrameHandleResponse(int id, GLuint texture_id);
  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) override;
  void DidProcessTask(const base::PendingTask& pending_task) override;

  // gpu::gles2::TextureManager::DestructionObserver implementation
  void OnTextureManagerDestroying(gpu::gles2::TextureManager* manager) override;
  void OnTextureRefDestroying(gpu::gles2::TextureRef* texture) override;

  // The client ID for this process's GPU channel
  int32 client_id_;

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

FetchTextureResourcesTaskInfo::~FetchTextureResourcesTaskInfo() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnMainOrCompositorThread());
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
    : client_id_(-1) {
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

void CompositorUtilsImpl::ShutdownOnGpuThread() {
  DCHECK(!gpu().has_shutdown);

  base::MessageLoop::current()->RemoveTaskObserver(this);
  gpu().has_shutdown = true;

  base::AutoLock lock1(incoming_texture_resource_fetches_.lock);
  base::AutoLock lock2(texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.queue.empty());
  DCHECK(texture_resource_fetches_.info_map.empty());
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

  base::AutoLock lock(texture_resource_fetches_.lock);
  ContinueFetchTextureResourcesOnGpuThread_Locked(id);
}

void CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread_Locked(
    int id) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  texture_resource_fetches_.lock.AssertAcquired();

  auto it = texture_resource_fetches_.info_map.find(id);
  if (it == texture_resource_fetches_.info_map.end()) {
    return;
  }

  FetchTextureResourcesTaskInfo* info = it->second;

  GLuint service_id = 0;
  if (RefTexture(info->client_id, info->route_id, info->mailbox)) {
    service_id = GetTextureIDForMailbox(info->mailbox);
  }

  // If the destination thread has quit already, this task may never run.
  // That's ok though - at some point, the TextureManager owned by the
  // ContextGroup that this texture belongs to will be deleted, and we drop
  // our texture ref there
  info->task_runner->PostTask(
      FROM_HERE,
      base::Bind(&CompositorUtilsImpl::SendCreateGLFrameHandleResponse,
                 base::Unretained(this), id, service_id));
}

void CompositorUtilsImpl::SendCreateGLFrameHandleResponse(int id,
                                                          GLuint service_id) {
  DCHECK(CalledOnMainOrCompositorThread());

  scoped_ptr<GLFrameHandle> handle;
  CompositorUtils::CreateGLFrameHandleCallback callback;
  {
    base::AutoLock lock(texture_resource_fetches_.lock);

    auto it = texture_resource_fetches_.info_map.find(id);
    if (it == texture_resource_fetches_.info_map.end()) {
      // Should only happen at shutdown, in which case we won't be leaking
      // the texture by returning early because we'll drop our reference when
      // the TextureManager owned by the ContextGroup that it belongs to is
      // deleted
      return;
    }

    FetchTextureResourcesTaskInfo* info = it->second;

    DCHECK(info->task_runner->RunsTasksOnCurrentThread());

    if (service_id > 0) {
      // If we arrive here, then we're guaranteed to have an owning reference
      // to the underlying texture
      handle.reset(new GLFrameHandle(info->mailbox,
                                     service_id,
                                     info->context_provider));
    }

    callback = info->callback;
    texture_resource_fetches_.id_allocator.FreeId(id);
    delete info;
    texture_resource_fetches_.info_map.erase(it);
  }

  callback.Run(handle.Pass());
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

  base::AutoLock lock1(incoming_texture_resource_fetches_.lock);
  DCHECK(!incoming_texture_resource_fetches_.gpu_needs_wakeup);
  incoming_texture_resource_fetches_.gpu_needs_wakeup = true;

  if (!incoming_texture_resource_fetches_.fetch_pending) {
    return;
  }

  incoming_texture_resource_fetches_.fetch_pending = false;

  base::AutoLock lock2(texture_resource_fetches_.lock);

  while (!incoming_texture_resource_fetches_.queue.empty()) {
    FetchTextureResourcesTaskInfo* info =
        incoming_texture_resource_fetches_.queue.front();
    incoming_texture_resource_fetches_.queue.pop();
    FetchTextureResourcesOnGpuThread(info);
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

void CompositorUtilsImpl::Initialize() {
  DCHECK(!main().compositor_thread);

  main().compositor_thread.reset(new CompositorThread());
  main().compositor_thread->Start();

  main_task_runner_ = base::MessageLoopProxy::current();
  compositor_task_runner_ = main().compositor_thread->message_loop_proxy();

  client_id_ =
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId();

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

  {
    base::AutoLock lock1(incoming_texture_resource_fetches_.lock);
    base::AutoLock lock2(texture_resource_fetches_.lock);

    incoming_texture_resource_fetches_.fetch_pending = false;
    incoming_texture_resource_fetches_.can_queue = false;
    while (incoming_texture_resource_fetches_.queue.size() > 0) {
      FetchTextureResourcesTaskInfo* info =
          incoming_texture_resource_fetches_.queue.front();
      incoming_texture_resource_fetches_.queue.pop();
      delete info;
    }

    for (auto it = texture_resource_fetches_.info_map.begin();
         it != texture_resource_fetches_.info_map.end(); ++it) {
      texture_resource_fetches_.id_allocator.FreeId(it->first);
      delete it->second;
    }
    texture_resource_fetches_.info_map.clear();
  }

  scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner =
      content::GpuChildThread::GetTaskRunner();
  if (gpu_task_runner.get()) {
    gpu_task_runner->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtilsImpl::ShutdownOnGpuThread,
                   base::Unretained(this)));
  }

  main().compositor_thread.reset();
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

gfx::GLSurfaceHandle CompositorUtilsImpl::GetSharedSurfaceHandle() {
  gfx::GLSurfaceHandle handle(gfx::kNullPluginWindow, gfx::NULL_TRANSPORT);
  handle.parent_client_id = client_id_;

  return handle;
}

bool CompositorUtilsImpl::CanUseGpuCompositing() {
  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return false;
  }

  if (!BrowserPlatformIntegration::GetInstance()->GetGLShareContext()) {
    return false;
  }

  return true;
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
