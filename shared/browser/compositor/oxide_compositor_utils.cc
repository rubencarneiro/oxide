// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include "base/auto_reset.h"
#include "base/bind.h"
#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/thread_task_runner_handle.h"
#include "cc/output/context_provider.h"
#include "cc/raster/single_thread_task_graph_runner.h"
#include "cc/surfaces/surface_id_allocator.h"
#include "cc/surfaces/surface_manager.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "gpu/command_buffer/common/command_buffer_id.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/ipc/client/command_buffer_proxy_impl.h"
#include "gpu/ipc/client/gpu_channel_host.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_switches.h"

#include "shared/browser/oxide_browser_platform_integration.h"
#include "shared/common/oxide_id_allocator.h"

#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_gpu_shims.h"

namespace oxide {

namespace {

uint32_t g_surface_id_namespace = 0;

void WakeUpGpuThread() {}

void DestroyEGLImage(EGLImageKHR egl_image) {
  EGL::DestroyImageKHR(GpuUtils::GetHardwareEGLDisplay(), egl_image);
}

} // namespace

class FetchTextureResourcesTaskInfo {
 public:
  virtual ~FetchTextureResourcesTaskInfo();

  virtual void DoFetch() = 0;
  virtual void RunCallback() = 0;

  gpu::CommandBufferId command_buffer_id() const {
    return command_buffer_id_;
  }
  const gpu::Mailbox& mailbox() const { return mailbox_; }
  uint64_t sync_point() const { return sync_point_; }

 protected:
  FetchTextureResourcesTaskInfo(
      const gpu::CommandBufferId& command_buffer_id,
      const gpu::Mailbox& mailbox,
      uint64_t sync_point)
      : command_buffer_id_(command_buffer_id),
        mailbox_(mailbox),
        sync_point_(sync_point) {}

 private:
  gpu::CommandBufferId command_buffer_id_;
  gpu::Mailbox mailbox_;
  uint64_t sync_point_;
};

class FetchTextureIDTaskInfo : public FetchTextureResourcesTaskInfo {
 public:
  FetchTextureIDTaskInfo(
      const gpu::CommandBufferId& command_buffer_id,
      const gpu::Mailbox& mailbox,
      uint64_t sync_point,
      const CompositorUtils::GetTextureFromMailboxCallback& callback)
      : FetchTextureResourcesTaskInfo(command_buffer_id,
                                      mailbox,
                                      sync_point),
        callback_(callback),
        texture_(0) {}
  ~FetchTextureIDTaskInfo() override;

  void DoFetch() override;
  void RunCallback() override;

 private:
  CompositorUtils::GetTextureFromMailboxCallback callback_;
  GLuint texture_;
};

class FetchEGLImageTaskInfo : public FetchTextureResourcesTaskInfo {
 public:
  FetchEGLImageTaskInfo(
      const gpu::CommandBufferId& command_buffer_id,
      const gpu::Mailbox& mailbox,
      uint64_t sync_point,
      const CompositorUtils::CreateEGLImageFromMailboxCallback& callback)
      : FetchTextureResourcesTaskInfo(command_buffer_id,
                                      mailbox,
                                      sync_point),
        callback_(callback),
        egl_image_(EGL_NO_IMAGE_KHR) {}
  ~FetchEGLImageTaskInfo() override;

  void DoFetch() override;
  void RunCallback() override;

 private:
  CompositorUtils::CreateEGLImageFromMailboxCallback callback_;
  EGLImageKHR egl_image_;
};

class CompositorThread : public base::Thread {
 public:
  CompositorThread();
  ~CompositorThread() override;

 private:
  // base::Thread implementation
  void Init() override;
};

class CompositorUtilsImpl : public CompositorUtils,
                            public base::MessageLoop::TaskObserver {
 public:
  static CompositorUtilsImpl* GetInstance();

  // CompositorUtils implementation
  void Initialize(bool has_share_context) override;
  void Shutdown() override;
  void GetTextureFromMailbox(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint64_t sync_point,
      const CompositorUtils::GetTextureFromMailboxCallback& callback) override;
  void CreateEGLImageFromMailbox(
      cc::ContextProvider* context_provider,
      const gpu::Mailbox& mailbox,
      uint64_t sync_point,
      const CompositorUtils::CreateEGLImageFromMailboxCallback& callback) override;
  bool CanUseGpuCompositing() const override;
  CompositingMode GetCompositingMode() const override;
  cc::TaskGraphRunner* GetTaskGraphRunner() const override;
  cc::SurfaceManager* GetSurfaceManager() const override;
  std::unique_ptr<cc::SurfaceIdAllocator> CreateSurfaceIdAllocator() override;

  bool CalledOnMainThread() const;
  bool CalledOnGpuThread() const;

 private:
  friend struct base::DefaultSingletonTraits<CompositorUtilsImpl>;

  CompositorUtilsImpl();
  ~CompositorUtilsImpl() override;

  void InitializeOnGpuThread();
  void ShutdownOnGpuThread(base::WaitableEvent* shutdown_event);

  void FetchTextureResourcesOnGpuThread(FetchTextureResourcesTaskInfo* info);
  void ContinueFetchTextureResourcesOnGpuThread(int id);
  void ContinueFetchTextureResourcesOnGpuThread_Locked(int id);

  void SendFetchTextureResourcesResponse(int id);

  // base::MessageLoop::TaskObserver implementation
  void WillProcessTask(const base::PendingTask& pending_task) override;
  void DidProcessTask(const base::PendingTask& pending_task) override;

  // The client ID for this process's GPU channel
  int32_t client_id_;

  // Whether or not we have a shared GL context
  bool has_share_context_;

  // ThreadChecker for the thread that called Initialize()
  base::ThreadChecker main_thread_checker_;
  scoped_refptr<base::SingleThreadTaskRunner> main_thread_task_runner_;

  base::ThreadChecker gpu_thread_checker_;

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
    std::unique_ptr<cc::SingleThreadTaskGraphRunner> task_graph_runner;
    std::unique_ptr<cc::SurfaceManager> surface_manager;
  } main_unsafe_access_;

  struct GpuData {
    GpuData()
        : has_shutdown(false),
          in_fetch_resources(false) {}

    bool has_shutdown;
    bool in_fetch_resources;
  } gpu_unsafe_access_;

  const MainData& main() const;
  MainData& main();

  GpuData& gpu();

  DISALLOW_COPY_AND_ASSIGN(CompositorUtilsImpl);
};

FetchTextureResourcesTaskInfo::~FetchTextureResourcesTaskInfo() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnMainThread());
}

FetchTextureIDTaskInfo::~FetchTextureIDTaskInfo() {}

void FetchTextureIDTaskInfo::DoFetch() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnGpuThread());
  texture_ = GpuUtils::GetTextureFromMailbox(command_buffer_id(),
                                             mailbox());
}

void FetchTextureIDTaskInfo::RunCallback() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnMainThread());
  callback_.Run(texture_);
}

FetchEGLImageTaskInfo::~FetchEGLImageTaskInfo() {
  if (egl_image_ != EGL_NO_IMAGE_KHR) {
    // XXX: Do we need to do this on a specific thread?
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(&DestroyEGLImage, egl_image_));
  }
}

void FetchEGLImageTaskInfo::DoFetch() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnGpuThread());
  egl_image_ = GpuUtils::CreateEGLImageFromMailbox(command_buffer_id(),
                                                   mailbox());
}

void FetchEGLImageTaskInfo::RunCallback() {
  DCHECK(CompositorUtilsImpl::GetInstance()->CalledOnMainThread());
  callback_.Run(egl_image_);
  egl_image_ = EGL_NO_IMAGE_KHR;
}

CompositorUtilsImpl::CompositorUtilsImpl()
    : client_id_(-1),
      has_share_context_(false) {
  main_thread_checker_.DetachFromThread();
  gpu_thread_checker_.DetachFromThread();
}

CompositorUtilsImpl::~CompositorUtilsImpl() {}

void CompositorUtilsImpl::InitializeOnGpuThread() {
  DCHECK(GpuUtils::GetTaskRunner()->BelongsToCurrentThread());
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  incoming_texture_resource_fetches_.gpu_needs_wakeup = false;

  base::MessageLoop::current()->AddTaskObserver(this);
}

void CompositorUtilsImpl::ShutdownOnGpuThread(
    base::WaitableEvent* shutdown_event) {
  DCHECK(!gpu().has_shutdown);
  DCHECK(!gpu().in_fetch_resources);

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

  if (GpuUtils::IsSyncPointRetired(info->command_buffer_id(),
                                   info->sync_point())) {
    ContinueFetchTextureResourcesOnGpuThread_Locked(id);
    return;
  }

  DCHECK(!gpu().in_fetch_resources);
  base::AutoReset<bool> in_fetch_resources(&gpu().in_fetch_resources, true);

  if (!GpuUtils::WaitForSyncPoint(
          info->command_buffer_id(),
          info->sync_point(),
          base::Bind(
            &CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread,
            base::Unretained(this),
            id))) {
    LOG(WARNING) << "Failed to wait for invalid fence sync";
  }
}

void CompositorUtilsImpl::ContinueFetchTextureResourcesOnGpuThread(int id) {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());

  if (gpu().has_shutdown) {
    return;
  }

  if (gpu().in_fetch_resources) {
    ContinueFetchTextureResourcesOnGpuThread_Locked(id);
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

  info->DoFetch();

  main_thread_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&CompositorUtilsImpl::SendFetchTextureResourcesResponse,
                 base::Unretained(this), id));
}

void CompositorUtilsImpl::SendFetchTextureResourcesResponse(int id) {
  DCHECK(main_thread_checker_.CalledOnValidThread());

  FetchTextureResourcesTaskInfo* info = nullptr;
  {
    base::AutoLock lock(texture_resource_fetches_.lock);

    auto it = texture_resource_fetches_.info_map.find(id);
    DCHECK(it != texture_resource_fetches_.info_map.end());

    info = it->second;

    texture_resource_fetches_.id_allocator.FreeId(id);
    texture_resource_fetches_.info_map.erase(it);
  }

  info->RunCallback();
  delete info;
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

CompositorUtilsImpl::GpuData& CompositorUtilsImpl::gpu() {
  DCHECK(gpu_thread_checker_.CalledOnValidThread());
  return gpu_unsafe_access_;
}

const CompositorUtilsImpl::MainData& CompositorUtilsImpl::main() const {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  return main_unsafe_access_;
}

CompositorUtilsImpl::MainData& CompositorUtilsImpl::main() {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  return main_unsafe_access_;
}

// static
CompositorUtilsImpl* CompositorUtilsImpl::GetInstance() {
  return base::Singleton<CompositorUtilsImpl>::get();
}

void CompositorUtilsImpl::Initialize(bool has_share_context) {
  has_share_context_ = has_share_context;
  client_id_ =
      content::BrowserGpuChannelHostFactory::instance()->GetGpuChannelId();

  main_thread_task_runner_ = base::ThreadTaskRunnerHandle::Get();

  main().task_graph_runner.reset(new cc::SingleThreadTaskGraphRunner());
  main().task_graph_runner->Start("CompositorTileWorker1",
                                  base::SimpleThread::Options());

  main().surface_manager.reset(new cc::SurfaceManager());

  content::CauseForGpuLaunch cause =
      content::CAUSE_FOR_GPU_LAUNCH_WEBGRAPHICSCONTEXT3DCOMMANDBUFFERIMPL_INITIALIZE;
  scoped_refptr<gpu::GpuChannelHost> gpu_channel_host(
      content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(cause));
  if (gpu_channel_host.get()) {
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtilsImpl::InitializeOnGpuThread,
                   base::Unretained(this)));
    base::AutoLock lock(incoming_texture_resource_fetches_.lock);
    incoming_texture_resource_fetches_.can_queue = true;
  }
}

void CompositorUtilsImpl::Shutdown() {
  main().task_graph_runner->Shutdown();
  main().task_graph_runner.reset();

  main().surface_manager.reset();

  // Detach the GPU thread MessageLoop::TaskObserver, to stop processing
  // and existing incoming requests
  scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner =
      GpuUtils::GetTaskRunner();
  if (gpu_task_runner.get()) {
    base::ThreadRestrictions::ScopedAllowWait allow_wait;
    base::WaitableEvent event(false, false);
    gpu_task_runner->PostTask(
        FROM_HERE,
        base::Bind(&CompositorUtilsImpl::ShutdownOnGpuThread,
                   base::Unretained(this), &event));
    event.Wait();
  }

  // The GPU thread is no longer touching this, so we don't need a lock
  while (incoming_texture_resource_fetches_.queue.size() > 0) {
    FetchTextureResourcesTaskInfo* info =
        incoming_texture_resource_fetches_.queue.front();
    incoming_texture_resource_fetches_.queue.pop();
    delete info;
  }

  // The GPU thread is no longer touching this, so we don't need a lock
  for (auto it = texture_resource_fetches_.info_map.begin();
       it != texture_resource_fetches_.info_map.end(); ++it) {
    texture_resource_fetches_.id_allocator.FreeId(it->first);
    delete it->second;
  }
  texture_resource_fetches_.info_map.clear();
}

void CompositorUtilsImpl::GetTextureFromMailbox(
    cc::ContextProvider* context_provider,
    const gpu::Mailbox& mailbox,
    uint64_t sync_point,
    const CompositorUtils::GetTextureFromMailboxCallback& callback) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  DCHECK(context_provider);
  DCHECK(!callback.is_null());
  DCHECK(!mailbox.IsZero());

  FetchTextureIDTaskInfo* info =
      new FetchTextureIDTaskInfo(
        static_cast<content::ContextProviderCommandBuffer*>(
            context_provider)->GetCommandBufferProxy()->GetCommandBufferID(),
        mailbox,
        sync_point,
        callback);

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.can_queue);

  if (!incoming_texture_resource_fetches_.fetch_pending) {
    incoming_texture_resource_fetches_.fetch_pending = true;
    if (incoming_texture_resource_fetches_.gpu_needs_wakeup) {
      // We assert |can_queue| above, so this should never fail
      GpuUtils::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(&WakeUpGpuThread));
    }
  }

  incoming_texture_resource_fetches_.queue.push(info);
}

void CompositorUtilsImpl::CreateEGLImageFromMailbox(
    cc::ContextProvider* context_provider,
    const gpu::Mailbox& mailbox,
    uint64_t sync_point,
    const CompositorUtils::CreateEGLImageFromMailboxCallback& callback) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  DCHECK(context_provider);
  DCHECK(!callback.is_null());
  DCHECK(!mailbox.IsZero());

  FetchEGLImageTaskInfo* info =
      new FetchEGLImageTaskInfo(
        static_cast<content::ContextProviderCommandBuffer*>(
            context_provider)->GetCommandBufferProxy()->GetCommandBufferID(),
        mailbox,
        sync_point,
        callback);

  base::AutoLock lock(incoming_texture_resource_fetches_.lock);
  DCHECK(incoming_texture_resource_fetches_.can_queue);

  if (!incoming_texture_resource_fetches_.fetch_pending) {
    incoming_texture_resource_fetches_.fetch_pending = true;
    if (incoming_texture_resource_fetches_.gpu_needs_wakeup) {
      // We assert |can_queue| above, so this should never fail
      GpuUtils::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(&WakeUpGpuThread));
    }
  }

  incoming_texture_resource_fetches_.queue.push(info);
}

bool CompositorUtilsImpl::CanUseGpuCompositing() const {
  return GetCompositingMode() != COMPOSITING_MODE_SOFTWARE;
}

CompositingMode CompositorUtilsImpl::GetCompositingMode() const {
  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return COMPOSITING_MODE_SOFTWARE;
  }

  if (gfx::GetGLImplementation() == gfx::kGLImplementationEGLGLES2 &&
      GpuUtils::CanUseEGLImage()) {
    return COMPOSITING_MODE_EGLIMAGE;
  }

  if (has_share_context_) {
    return COMPOSITING_MODE_TEXTURE;
  }

  return COMPOSITING_MODE_SOFTWARE;
}

cc::TaskGraphRunner* CompositorUtilsImpl::GetTaskGraphRunner() const {
  return main().task_graph_runner.get();
}

cc::SurfaceManager* CompositorUtilsImpl::GetSurfaceManager() const {
  return main().surface_manager.get();
}

std::unique_ptr<cc::SurfaceIdAllocator>
CompositorUtilsImpl::CreateSurfaceIdAllocator() {
  std::unique_ptr<cc::SurfaceIdAllocator> allocator(
      new cc::SurfaceIdAllocator(++g_surface_id_namespace));
  allocator->RegisterSurfaceIdNamespace(GetSurfaceManager());
  return allocator;
}

bool CompositorUtilsImpl::CalledOnMainThread() const {
  return main_thread_checker_.CalledOnValidThread();
}

bool CompositorUtilsImpl::CalledOnGpuThread() const {
  return gpu_thread_checker_.CalledOnValidThread();
}

CompositorUtils::~CompositorUtils() {}

// static
CompositorUtils* CompositorUtils::GetInstance() {
  return CompositorUtilsImpl::GetInstance();
}

} // namespace oxide
