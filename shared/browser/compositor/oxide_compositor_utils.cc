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
#include "base/logging.h"
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

class FetchTextureResourcesTask
    : public base::RefCountedThreadSafe<FetchTextureResourcesTask> {
 public:
  FetchTextureResourcesTask(
      int32 client_id,
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const CompositorUtils::CreateGLFrameHandleCallback& callback,
      scoped_refptr<base::TaskRunner> task_runner)
      : client_id_(client_id),
        route_id_(-1),
        context_provider_(context_provider),
        mailbox_(mailbox),
        sync_point_(sync_point),
        callback_(callback),
        task_runner_(task_runner) {
    route_id_ = content::oxide_gpu_shim::GetContextProviderRouteID(
        static_cast<content::ContextProviderCommandBuffer*>(
          context_provider_.get()));
  }

  virtual ~FetchTextureResourcesTask() {
    DCHECK(callback_.is_null());
    DCHECK(!context_provider_.get());
  }

  void FetchTextureResourcesOnGpuThread();

 private:
  void OnSyncPointRetired();
  void SendResponseOnDestinationThread(GLuint service_id);

  int32 client_id_;
  int32 route_id_;
  scoped_refptr<cc::ContextProvider> context_provider_;
  gpu::Mailbox mailbox_;
  uint32 sync_point_;
  CompositorUtils::CreateGLFrameHandleCallback callback_;
  scoped_refptr<base::TaskRunner> task_runner_;
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
      scoped_refptr<base::TaskRunner> task_runner) override;
  gfx::GLSurfaceHandle GetSharedSurfaceHandle() override;
  bool CanUseGpuCompositing() override;

  bool RefTexture(int32 client_id,
                  int32 route_id,
                  const gpu::Mailbox& mailbox);
  void UnrefTexture(const gpu::Mailbox& mailbox);
  GLuint GetTextureIDForMailbox(const gpu::Mailbox& mailbox);

 private:
  friend struct DefaultSingletonTraits<CompositorUtilsImpl>;

  CompositorUtilsImpl();
  ~CompositorUtilsImpl() override;

  void InitializeOnGpuThread();
  void ShutdownOnGpuThread();
  bool AcquirePendingTextureResourceFetchesLocked(
      std::queue<scoped_refptr<FetchTextureResourcesTask> >* queue);
  void ProcessTextureResourceFetches(
      std::queue<scoped_refptr<FetchTextureResourcesTask> >* queue);

  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) override;
  void DidProcessTask(const base::PendingTask& pending_task) override;

  // gpu::gles2::TextureManager::DestructionObserver implementation
  void OnTextureManagerDestroying(gpu::gles2::TextureManager* manager) override;
  void OnTextureRefDestroying(gpu::gles2::TextureRef* texture) override;

  int32 client_id_;

  base::ThreadChecker main_thread_checker_;
  base::ThreadChecker gpu_thread_checker_;

  scoped_refptr<base::SingleThreadTaskRunner> compositor_task_runner_;

  base::Lock fetch_texture_resources_lock_;
  bool fetch_texture_resources_pending_;
  bool gpu_thread_is_processing_task_;
  bool can_fetch_texture_resources_;
  std::queue<scoped_refptr<FetchTextureResourcesTask> > fetch_texture_resources_queue_;

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

void FetchTextureResourcesTask::OnSyncPointRetired() {
  GLuint service_id = 0;
  CompositorUtilsImpl* utils = CompositorUtilsImpl::GetInstance();
  if (utils->RefTexture(client_id_, route_id_, mailbox_)) {
    service_id = utils->GetTextureIDForMailbox(mailbox_);
  }

  task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&FetchTextureResourcesTask::SendResponseOnDestinationThread,
                 this, service_id));
}

void FetchTextureResourcesTask::SendResponseOnDestinationThread(
    GLuint service_id) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());

  if (service_id == 0) {
    callback_.Run(scoped_ptr<GLFrameData>());
    return;
  }

  scoped_ptr<GLFrameHandle> handle(
      new GLFrameHandle(mailbox_,
                        service_id,
                        context_provider_));
  callback_.Run(handle.Pass());

  callback_.Reset();
  context_provider_ = nullptr;
}

void FetchTextureResourcesTask::FetchTextureResourcesOnGpuThread() {
  gpu::SyncPointManager* sync_point_manager =
      content::GpuChildThread::GetChannelManager()->sync_point_manager();
  if (sync_point_manager->IsSyncPointRetired(sync_point_)) {
    OnSyncPointRetired();
    return;
  }

  sync_point_manager->AddSyncPointCallback(
      sync_point_,
      base::Bind(&FetchTextureResourcesTask::OnSyncPointRetired,
                 this));
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
      fetch_texture_resources_pending_(false),
      gpu_thread_is_processing_task_(false),
      can_fetch_texture_resources_(false) {
  main_thread_checker_.DetachFromThread();
  gpu_thread_checker_.DetachFromThread();
}

CompositorUtilsImpl::~CompositorUtilsImpl() {
  base::AutoLock lock(fetch_texture_resources_lock_);
  DCHECK(fetch_texture_resources_queue_.empty());
}

void CompositorUtilsImpl::InitializeOnGpuThread() {
  DCHECK(content::GpuChildThread::GetTaskRunner()->BelongsToCurrentThread());
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  base::AutoLock lock(fetch_texture_resources_lock_);
  gpu_thread_is_processing_task_ = true;

  base::MessageLoop::current()->AddTaskObserver(this);
}

void CompositorUtilsImpl::ShutdownOnGpuThread() {
  DCHECK(!gpu().has_shutdown);

  base::MessageLoop::current()->RemoveTaskObserver(this);
  gpu().has_shutdown = true;

  std::queue<scoped_refptr<FetchTextureResourcesTask> > queue;
  {
    base::AutoLock lock(fetch_texture_resources_lock_);
    gpu_thread_is_processing_task_ = false;
    if (!AcquirePendingTextureResourceFetchesLocked(&queue)) {
      return;
    }
  }

  ProcessTextureResourceFetches(&queue);
}

bool CompositorUtilsImpl::AcquirePendingTextureResourceFetchesLocked(
    std::queue<scoped_refptr<FetchTextureResourcesTask> >* queue) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  fetch_texture_resources_lock_.AssertAcquired();

  if (!fetch_texture_resources_pending_) {
    return false;
  }
  fetch_texture_resources_pending_ = false;

  std::swap(*queue, fetch_texture_resources_queue_);
  return true;
}

void CompositorUtilsImpl::ProcessTextureResourceFetches(
    std::queue<scoped_refptr<FetchTextureResourcesTask> >* queue) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  while (!queue->empty()) {
    scoped_refptr<FetchTextureResourcesTask> task = queue->front();
    queue->pop();
    task->FetchTextureResourcesOnGpuThread();
  }
}

void CompositorUtilsImpl::WillProcessTask(
    const base::PendingTask& pending_task) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  base::AutoLock lock(fetch_texture_resources_lock_);
  DCHECK(!gpu_thread_is_processing_task_);
  gpu_thread_is_processing_task_ = true;
}

void CompositorUtilsImpl::DidProcessTask(
    const base::PendingTask& pending_task) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  std::queue<scoped_refptr<FetchTextureResourcesTask> > queue;
  {
    base::AutoLock lock(fetch_texture_resources_lock_);
    DCHECK(gpu_thread_is_processing_task_);
    gpu_thread_is_processing_task_ = false;

    if (!AcquirePendingTextureResourceFetchesLocked(&queue)) {
      return;
    }
  }

  ProcessTextureResourceFetches(&queue);
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
    base::AutoLock lock(fetch_texture_resources_lock_);
    can_fetch_texture_resources_ = true;
  }
}

void CompositorUtilsImpl::Shutdown() {
  DCHECK(main().compositor_thread);

  {
    base::AutoLock lock(fetch_texture_resources_lock_);
    can_fetch_texture_resources_ = false;
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
    scoped_refptr<base::TaskRunner> task_runner) {
  DCHECK(context_provider);
  DCHECK(!callback.is_null());
  DCHECK(!mailbox.IsZero());
  DCHECK(task_runner);

  scoped_refptr<FetchTextureResourcesTask> task =
      new FetchTextureResourcesTask(
        client_id_, context_provider,
        mailbox, sync_point, callback, task_runner);

  base::AutoLock lock(fetch_texture_resources_lock_);

  DCHECK(can_fetch_texture_resources_);

  if (!fetch_texture_resources_pending_) {
    fetch_texture_resources_pending_ = true;
    if (!gpu_thread_is_processing_task_) {
      // We assert |can_fetch_texture_resources_| above, so this should
      // never fail
      content::GpuChildThread::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(&WakeUpGpuThread));
    }
  }

  fetch_texture_resources_queue_.push(task);
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

CompositorUtils::~CompositorUtils() {}

// static
CompositorUtils* CompositorUtils::GetInstance() {
  return CompositorUtilsImpl::GetInstance();
}

} // namespace oxide
