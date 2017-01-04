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

#include "oxide_compositor.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/trace_event.h"
#include "cc/animation/animation_host.h"
#include "cc/layers/layer.h"
#include "cc/output/compositor_frame_sink.h"
#include "cc/output/context_provider.h"
#include "cc/output/output_surface.h"
#include "cc/output/renderer_settings.h"
#include "cc/output/texture_mailbox_deleter.h"
#include "cc/scheduler/begin_frame_source.h"
#include "cc/scheduler/delay_based_time_source.h"
#include "cc/surfaces/direct_compositor_frame_sink.h"
#include "cc/surfaces/display.h"
#include "cc/surfaces/display_scheduler.h"
#include "cc/trees/layer_tree.h"
#include "cc/trees/layer_tree_host.h"
#include "cc/trees/layer_tree_host_in_process.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h" // nogncheck
#include "content/browser/gpu/browser_gpu_memory_buffer_manager.h" // nogncheck
#include "content/browser/gpu/gpu_data_manager_impl.h" // nogncheck
#include "content/common/host_shared_bitmap_manager.h" // nogncheck
#include "gpu/command_buffer/client/context_support.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/client/shared_memory_limits.h"
#include "gpu/command_buffer/common/gles2_cmd_utils.h"
#include "gpu/GLES2/gl2extchromium.h"
#include "gpu/ipc/client/gpu_channel_host.h"
#include "services/ui/public/cpp/gpu/command_buffer_metrics.h"
#include "services/ui/public/cpp/gpu/context_provider_command_buffer.h"
#include "ui/gfx/geometry/rect.h"
#include "url/gurl.h"

#include "oxide_compositor_client.h"
#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_gpu_shims.h"
#include "oxide_compositor_observer.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_output_surface_gl.h"
#include "oxide_compositor_output_surface_software.h"
#include "oxide_compositor_software_output_device.h"
#include "oxide_compositor_utils.h"

namespace oxide {

namespace {

scoped_refptr<cc::ContextProvider> CreateOffscreenContextProvider() {
  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return nullptr;
  }

  scoped_refptr<gpu::GpuChannelHost> gpu_channel_host(
      content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync());
  if (!gpu_channel_host.get()) {
    return nullptr;
  }

  gpu::gles2::ContextCreationAttribHelper attrs;
  attrs.alpha_size = -1;
  attrs.depth_size = 0;
  attrs.stencil_size = 0;
  attrs.samples = 0;
  attrs.sample_buffers = 0;
  attrs.bind_generates_resource = false;
  attrs.lose_context_when_out_of_memory = true;

  return make_scoped_refptr(
      new ui::ContextProviderCommandBuffer(
          gpu_channel_host.get(),
          gpu::GPU_STREAM_DEFAULT,
          gpu::GpuStreamPriority::NORMAL,
          gpu::kNullSurfaceHandle,
          GURL(),
          false, // automatic_flushes
          false, // support_locking
          gpu::SharedMemoryLimits(),
          attrs,
          nullptr,
          ui::command_buffer_metrics::CONTEXT_TYPE_UNKNOWN));
}

} // namespace

Compositor::Compositor(CompositorClient* client)
    : mode_(CompositorUtils::GetInstance()->GetCompositingMode()),
      client_(client),
      task_runner_(base::ThreadTaskRunnerHandle::Get()),
      output_surface_(nullptr),
      mailbox_buffer_map_(mode_),
      frame_sink_id_(CompositorUtils::GetInstance()->AllocateFrameSinkId()),
      begin_frame_source_(
          new cc::DelayBasedBeginFrameSource(
              base::MakeUnique<cc::DelayBasedTimeSource>(
                  base::ThreadTaskRunnerHandle::Get().get()))),
      animation_host_(cc::AnimationHost::CreateMainInstance()),
      layer_tree_host_eviction_pending_(false),
      can_evict_layer_tree_host_(false),
      num_failed_recreate_attempts_(0),
      visible_(false),
      device_scale_factor_(1.0f),
      root_layer_(cc::Layer::Create()),
      next_output_surface_id_(1),
      pending_swaps_(0),
      frames_waiting_for_completion_(0),
      mailbox_resource_fetches_in_progress_(0),
      weak_factory_(this) {
  CompositorUtils::GetInstance()
      ->GetSurfaceManager()
      ->RegisterFrameSinkId(frame_sink_id_);
}

bool Compositor::SurfaceIdIsCurrent(uint32_t surface_id) {
  return output_surface_ && output_surface_->surface_id() == surface_id;
}

void Compositor::DidCompleteGLFrame(uint32_t surface_id,
                                    std::unique_ptr<CompositorFrameData> frame) {
  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::Compositor:frames_waiting_for_completion",
      this,
      "frames_waiting_for_completion", frames_waiting_for_completion_);
  --frames_waiting_for_completion_;

  ContinueSwapGLFrame(surface_id, std::move(frame));
}

void Compositor::ContinueSwapGLFrame(
    uint32_t surface_id,
    std::unique_ptr<CompositorFrameData> frame) {
  DCHECK(frame->gl_frame_data);

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (!queued_gl_frame_swaps_.empty()) {
    QueueGLFrameSwap(std::move(frame));
    return;
  }

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE: {
      GLuint texture =
          mailbox_buffer_map_.ConsumeTextureFromMailbox(
            frame->gl_frame_data->mailbox);
      if (texture == 0U) {
        QueueGLFrameSwap(std::move(frame));
        return;
      }
      frame->gl_frame_data->resource.texture = texture;
      frame->gl_frame_data->type = GLFrameData::Type::TEXTURE;
      break;
    }

    case COMPOSITING_MODE_EGLIMAGE: {
      EGLImageKHR egl_image =
          mailbox_buffer_map_.ConsumeEGLImageFromMailbox(
            frame->gl_frame_data->mailbox);
      if (egl_image == EGL_NO_IMAGE_KHR) {
        QueueGLFrameSwap(std::move(frame));
        return;
      }
      frame->gl_frame_data->resource.egl_image = egl_image;
      frame->gl_frame_data->type = GLFrameData::Type::EGLIMAGE;
      break;
    }

    default:
      NOTREACHED();
  }

  SendSwapCompositorFrameToClient(std::move(frame));
}

void Compositor::QueueGLFrameSwap(std::unique_ptr<CompositorFrameData> frame) {
  queued_gl_frame_swaps_.push(std::move(frame));
}

void Compositor::DispatchQueuedGLFrameSwaps() {
  DCHECK(output_surface_);

  std::queue<std::unique_ptr<CompositorFrameData>> swaps;

  std::swap(swaps, queued_gl_frame_swaps_);
  while (!swaps.empty()) {
    ContinueSwapGLFrame(output_surface_->surface_id(),
                        std::move(swaps.front()));
    swaps.pop();
  }
}

void Compositor::SendSwapCompositorFrameToClient(
    std::unique_ptr<CompositorFrameData> frame) {
  DCHECK(output_surface_);
  DCHECK(!swap_ack_callback_.is_null());

  for (auto& observer : observers_) {
    observer.CompositorWillRequestSwapFrame();
  }

  // XXX: What if we are hidden?
  TRACE_EVENT_ASYNC_BEGIN1("cc", "oxide::Compositor:pending_swaps",
                           this,
                           "pending_swaps", pending_swaps_);
  ++pending_swaps_;

  can_evict_layer_tree_host_ = false;

  scoped_refptr<CompositorFrameHandle> handle =
      new CompositorFrameHandle(output_surface_->surface_id(),
                                task_runner_,
                                weak_factory_.GetWeakPtr(),
                                std::move(frame));
  client_->CompositorSwapFrame(handle.get(), swap_ack_callback_);

  for (auto& observer : observers_) {
    observer.CompositorDidRequestSwapFrame();
  }
}

// static
void Compositor::SwapCompositorFrameAckFromClientThunk(
    base::WeakPtr<Compositor> compositor,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  if (task_runner->BelongsToCurrentThread()) {
    if (compositor) {
      compositor->SwapCompositorFrameAckFromClient(surface_id,
                                                   std::move(returned_frames));
    }
  } else {
    // SwapAckCallback may be called from another thread where the application
    // UI is composited on a dedicated render thread. Although we expect this
    // to happen whilst the UI thread is blocked, we delegate this to the UI
    // thread to avoid hitting DCHECKs everywhere
    task_runner->PostTask(
        FROM_HERE,
        base::Bind(&Compositor::SwapCompositorFrameAckFromClient,
                   compositor, surface_id, base::Passed(&returned_frames)));
  }
}

void Compositor::SwapCompositorFrameAckFromClient(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  TRACE_EVENT_ASYNC_END1("cc", "oxide::Compositor:pending_swaps",
                         this,
                         "pending_swaps", pending_swaps_);
  --pending_swaps_;

  for (auto& frame : returned_frames) {
    CHECK(frame->HasOneRef()) <<
        "Client returned frame handles that it still references!";
    DCHECK_EQ(CompositorFrameCollector::FromFrameHandle(frame.get()), this);
  }

  if (SurfaceIdIsCurrent(surface_id)) {
    output_surface_->DidSwapBuffers();
  }

  while (!returned_frames.empty()) {
    scoped_refptr<CompositorFrameHandle> handle(returned_frames.back());
    returned_frames.pop_back();

    if (!handle.get()) {
      continue;
    }

    std::unique_ptr<CompositorFrameData> frame = TakeFrameData(handle.get());
    ReclaimResourcesForFrame(FrameHandleSurfaceId(handle.get()), frame.get());
  }
}

void Compositor::OutputSurfaceChanged() {
  mailbox_buffer_map_.DropAllResources();

  while (!queued_gl_frame_swaps_.empty()) {
    queued_gl_frame_swaps_.pop();
  }
}

std::unique_ptr<cc::CompositorFrameSink>
Compositor::CreateCompositorFrameSink() {
  uint32_t output_surface_id = next_output_surface_id_++;

  scoped_refptr<cc::ContextProvider> context_provider;
  std::unique_ptr<cc::OutputSurface> output_surface;
  if (CompositorUtils::GetInstance()->CanUseGpuCompositing()) {
    context_provider = CreateOffscreenContextProvider();
    if (!context_provider.get()) {
      return nullptr;
    }
    output_surface =
        base::MakeUnique<CompositorOutputSurfaceGL>(output_surface_id,
                                                    context_provider,
                                                    this);
  } else {
    std::unique_ptr<CompositorSoftwareOutputDevice> output_device(
        new CompositorSoftwareOutputDevice());
    output_surface =
        base::MakeUnique<CompositorOutputSurfaceSoftware>(
            output_surface_id,
            std::move(output_device),
            this);
  }

  std::unique_ptr<cc::DisplayScheduler> scheduler(
      new cc::DisplayScheduler(
          base::ThreadTaskRunnerHandle::Get().get(),
          output_surface->capabilities().max_frames_pending));

  display_ =
      base::MakeUnique<cc::Display>(
          content::HostSharedBitmapManager::current(),
          content::BrowserGpuMemoryBufferManager::current(),
          cc::RendererSettings(),
          frame_sink_id_,
          begin_frame_source_.get(),
          std::move(output_surface),
          std::move(scheduler),
          base::MakeUnique<cc::TextureMailboxDeleter>(
              base::ThreadTaskRunnerHandle::Get().get()));

  std::unique_ptr<cc::DirectCompositorFrameSink> sink =
      base::MakeUnique<cc::DirectCompositorFrameSink>(
          frame_sink_id_,
          CompositorUtils::GetInstance()->GetSurfaceManager(),
          display_.get(),
          context_provider,
          nullptr,
          content::BrowserGpuMemoryBufferManager::current(),
          content::HostSharedBitmapManager::current());

  display_->Resize(layer_tree_host_->GetLayerTree()->device_viewport_size());
  display_->SetVisible(layer_tree_host_->IsVisible());

  return std::move(sink);
}

void Compositor::AddObserver(CompositorObserver* observer) {
  observers_.AddObserver(observer);
}

void Compositor::RemoveObserver(CompositorObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Compositor::EnsureLayerTreeHost() {
  DCHECK(!layer_tree_host_eviction_pending_);

  if (layer_tree_host_) {
    return;
  }

  cc::LayerTreeSettings settings;
  settings.renderer_settings.allow_antialiasing = false;

  cc::LayerTreeHostInProcess::InitParams params;
  params.client = this;
  params.task_graph_runner =
      CompositorUtils::GetInstance()->GetTaskGraphRunner();
  params.settings = &settings;
  params.main_task_runner = base::ThreadTaskRunnerHandle::Get();
  params.mutator_host = animation_host_.get();

  layer_tree_host_ =
      cc::LayerTreeHostInProcess::CreateSingleThreaded(this, &params);
  DCHECK(layer_tree_host_);

  can_evict_layer_tree_host_ = true;

  layer_tree_host_->GetLayerTree()->SetRootLayer(root_layer_);
  layer_tree_host_->SetVisible(visible_);
  layer_tree_host_->GetLayerTree()->SetViewportSize(size_);
  layer_tree_host_->GetLayerTree()->SetDeviceScaleFactor(device_scale_factor_);
}

void Compositor::MaybeEvictLayerTreeHost() {
  if (!layer_tree_host_eviction_pending_ ||
      (!can_evict_layer_tree_host_ && mode_ != COMPOSITING_MODE_SOFTWARE)) {
    return;
  }

  layer_tree_host_eviction_pending_ = false;
  can_evict_layer_tree_host_ = false;

  layer_tree_host_.reset();
  display_.reset();
}

void Compositor::GetTextureFromMailboxResponse(uint32_t surface_id,
                                               const gpu::Mailbox& mailbox,
                                               GLuint texture) {
  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::Compositor:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  --mailbox_resource_fetches_in_progress_;

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (texture == 0) {
    // FIXME: This just causes the compositor to hang
    return;
  }

  mailbox_buffer_map_.AddTextureMapping(mailbox, texture);
  DispatchQueuedGLFrameSwaps();
}

// static
void Compositor::CreateEGLImageFromMailboxResponseThunk(
    base::WeakPtr<Compositor> compositor,
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    EGLImageKHR egl_image) {
  if (!compositor) {
    if (egl_image != EGL_NO_IMAGE_KHR) {
      GpuUtils::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                     GpuUtils::GetHardwareEGLDisplay(),
                     egl_image));
    }
    return;
  }

  compositor->CreateEGLImageFromMailboxResponse(surface_id, mailbox, egl_image);
}

void Compositor::CreateEGLImageFromMailboxResponse(uint32_t surface_id,
                                                   const gpu::Mailbox& mailbox,
                                                   EGLImageKHR egl_image) {
  TRACE_EVENT_ASYNC_END1(
      "cc",
      "oxide::Compositor:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  --mailbox_resource_fetches_in_progress_;

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  if (egl_image == EGL_NO_IMAGE_KHR) {
    // FIXME: This just causes the compositor to hang
    return;
  }

  mailbox_buffer_map_.AddEGLImageMapping(mailbox, egl_image);
  DispatchQueuedGLFrameSwaps();
}

void Compositor::WillBeginMainFrame() {}
void Compositor::BeginMainFrame(const cc::BeginFrameArgs& args) {}
void Compositor::BeginMainFrameNotExpectedSoon() {}
void Compositor::DidBeginMainFrame() {}
void Compositor::UpdateLayerTreeHost() {}
void Compositor::ApplyViewportDeltas(
    const gfx::Vector2dF& inner_delta,
    const gfx::Vector2dF& outer_delta,
    const gfx::Vector2dF& elastic_overscroll_delta,
    float page_scale,
    float top_controls_delta) {}

void Compositor::RequestNewCompositorFrameSink() {
  std::unique_ptr<cc::CompositorFrameSink> sink(CreateCompositorFrameSink());
  if (!sink) {
    DidFailToInitializeCompositorFrameSink();
    return;
  }

  layer_tree_host_->SetCompositorFrameSink(std::move(sink));
}

void Compositor::DidInitializeCompositorFrameSink() {
  num_failed_recreate_attempts_ = 0;
}

void Compositor::DidFailToInitializeCompositorFrameSink() {
  num_failed_recreate_attempts_++;

  CHECK_LE(num_failed_recreate_attempts_, 4);

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&Compositor::RequestNewCompositorFrameSink,
                 weak_factory_.GetWeakPtr()));
}

void Compositor::WillCommit() {}

void Compositor::DidCommit() {
  for (auto& observer : observers_) {
    observer.CompositorDidCommit();
  }
}

void Compositor::DidCommitAndDrawFrame() {}
void Compositor::DidReceiveCompositorFrameAck() {}
void Compositor::DidCompletePageScaleAnimation() {}

void Compositor::DidSubmitCompositorFrame() {}
void Compositor::DidLoseCompositorFrameSink() {}

void Compositor::OutputSurfaceBound(CompositorOutputSurface* output_surface) {
  DCHECK(!output_surface_);

  output_surface_ = output_surface;

  swap_ack_callback_ =
      base::Bind(&Compositor::SwapCompositorFrameAckFromClientThunk,
                 weak_factory_.GetWeakPtr(),
                 task_runner_,
                 output_surface_->surface_id());

  OutputSurfaceChanged();
}

void Compositor::OutputSurfaceDestroyed(
    CompositorOutputSurface* output_surface) {
  if (output_surface != output_surface_) {
    return;
  }

  output_surface_ = nullptr;
  swap_ack_callback_.Reset();

  OutputSurfaceChanged();
}

void Compositor::MailboxBufferCreated(const gpu::Mailbox& mailbox,
                                      uint64_t sync_point) {
  DCHECK(output_surface_);

  TRACE_EVENT_ASYNC_BEGIN1(
      "cc",
      "oxide::Compositor:mailbox_resource_fetches_in_progress",
      this,
      "mailbox_resource_fetches_in_progress",
      mailbox_resource_fetches_in_progress_);
  ++mailbox_resource_fetches_in_progress_;

  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    CompositorUtils::GetInstance()->GetTextureFromMailbox(
        output_surface_->context_provider(),
        mailbox,
        sync_point,
        base::Bind(&Compositor::GetTextureFromMailboxResponse,
                   weak_factory_.GetWeakPtr(),
                   output_surface_->surface_id(), mailbox));
  } else {
    DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
    CompositorUtils::GetInstance()->CreateEGLImageFromMailbox(
        output_surface_->context_provider(),
        mailbox,
        sync_point,
        base::Bind(&Compositor::CreateEGLImageFromMailboxResponseThunk,
                   weak_factory_.GetWeakPtr(),
                   output_surface_->surface_id(), mailbox));
  }
}

void Compositor::MailboxBufferDestroyed(const gpu::Mailbox& mailbox) {
  DCHECK_NE(mode_, COMPOSITING_MODE_SOFTWARE);
  mailbox_buffer_map_.MailboxBufferDestroyed(mailbox);
}

void Compositor::SwapCompositorFrame(std::unique_ptr<CompositorFrameData> frame) {
  DCHECK(output_surface_);

  TRACE_EVENT0("cc", "oxide::Compositor::SwapCompositorFrame");

  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE: {
      DCHECK(frame->gl_frame_data);

      TRACE_EVENT_ASYNC_BEGIN1(
          "cc",
          "oxide::Compositor:frames_waiting_for_completion",
          this,
          "frames_waiting_for_completion", frames_waiting_for_completion_);
      ++frames_waiting_for_completion_;

      cc::ContextProvider* context_provider =
          output_surface_->context_provider();
      gpu::gles2::GLES2Interface* gl = context_provider->ContextGL();

      if (!context_provider->ContextCapabilities().sync_query) {
        gl->Finish();
        DidCompleteGLFrame(output_surface_->surface_id(), std::move(frame));
      } else {
        uint32_t query_id;
        gl->GenQueriesEXT(1, &query_id);
        gl->BeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query_id);
        context_provider->ContextSupport()->SignalQuery(
            query_id,
            base::Bind(&Compositor::DidCompleteGLFrame,
                       weak_factory_.GetWeakPtr(),
                       output_surface_->surface_id(),
                       base::Passed(&frame)));
        gl->EndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);
        gl->Flush();
        gl->DeleteQueriesEXT(1, &query_id);
      }
      break;
    }
    case COMPOSITING_MODE_SOFTWARE:
      DCHECK(frame->software_frame_data);
      SendSwapCompositorFrameToClient(std::move(frame));
      break;
  }
}

void Compositor::AllFramesReturnedFromClient()  {
  can_evict_layer_tree_host_ = true;
  task_runner_->PostTask(FROM_HERE,
                         base::Bind(&Compositor::MaybeEvictLayerTreeHost,
                                    weak_factory_.GetWeakPtr()));
}

void Compositor::ReclaimResourcesForFrame(uint32_t surface_id,
                                          CompositorFrameData* frame) {
  CompositorFrameAck ack;
  switch (mode_) {
    case COMPOSITING_MODE_SOFTWARE:
      ack.software_frame_id = frame->software_frame_data->id;
      break;
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE:
      ack.gl_frame_mailbox = frame->gl_frame_data->mailbox;
      mailbox_buffer_map_.ReclaimMailboxBufferResources(
          ack.gl_frame_mailbox);
      break;
  }

  if (!SurfaceIdIsCurrent(surface_id)) {
    return;
  }

  output_surface_->ReclaimResources(ack);
}

// static
std::unique_ptr<Compositor> Compositor::Create(CompositorClient* client) {
  DCHECK(client);
  return base::WrapUnique(new Compositor(client));
}

Compositor::~Compositor() {
  for (auto& observer : observers_) {
    observer.OnCompositorDestruction();
  }

  layer_tree_host_.reset();
  display_.reset();

  CompositorUtils::GetInstance()
      ->GetSurfaceManager()
      ->InvalidateFrameSinkId(frame_sink_id_);
}

void Compositor::SetVisibility(bool visible) {
  if (visible == visible_) {
    return;
  }

  visible_ = visible;

  if (visible) {
    layer_tree_host_eviction_pending_ = false;
    EnsureLayerTreeHost();
  }

  DCHECK(!layer_tree_host_eviction_pending_);

  layer_tree_host_->SetVisible(visible);
  if (display_) {
    display_->SetVisible(visible);
  }

  if (!visible) {
    layer_tree_host_eviction_pending_ = true;
    for (auto& observer : observers_) {
      observer.CompositorEvictResources();
    }
    MaybeEvictLayerTreeHost();
  }
}

void Compositor::SetDeviceScaleFactor(float scale) {
  if (scale == device_scale_factor_) {
    return;
  }

  device_scale_factor_ = scale;

  if (layer_tree_host_) {
    layer_tree_host_->GetLayerTree()->SetDeviceScaleFactor(scale);
  }
}

void Compositor::SetViewportSize(const gfx::Size& size) {
  if (size == size_) {
    return;
  }

  size_ = size;

  if (layer_tree_host_) {
    layer_tree_host_->GetLayerTree()->SetViewportSize(size);
  }
  if (display_) {
    display_->Resize(size);
  }
  root_layer_->SetBounds(size);
}

void Compositor::SetRootLayer(scoped_refptr<cc::Layer> layer) {
  root_layer_->RemoveAllChildren();
  if (layer.get()) {
    root_layer_->AddChild(layer);
  }
}

void Compositor::SetNeedsRedraw() {
  if (!layer_tree_host_) {
    return;
  }

  layer_tree_host_->SetNeedsRedrawRect(gfx::Rect(size_));
}

} // namespace oxide
