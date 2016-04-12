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
#include "base/thread_task_runner_handle.h"
#include "base/trace_event/trace_event.h"
#include "cc/layers/layer.h"
#include "cc/output/context_provider.h"
#include "cc/output/renderer_settings.h"
#include "cc/scheduler/begin_frame_source.h"
#include "cc/surfaces/onscreen_display_client.h"
#include "cc/surfaces/surface_display_output_surface.h"
#include "cc/trees/layer_tree_host.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/gpu/browser_gpu_memory_buffer_manager.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "content/common/gpu/client/gpu_channel_host.h"
#include "content/common/gpu/client/webgraphicscontext3d_command_buffer_impl.h"
#include "content/common/gpu_process_launch_causes.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "url/gurl.h"

#include "oxide_compositor_client.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_observer.h"
#include "oxide_compositor_output_surface_gl.h"
#include "oxide_compositor_output_surface_software.h"
#include "oxide_compositor_single_thread_proxy.h"
#include "oxide_compositor_software_output_device.h"
#include "oxide_compositor_utils.h"

namespace oxide {

namespace {

typedef content::WebGraphicsContext3DCommandBufferImpl WGC3DCBI;

scoped_ptr<WGC3DCBI> CreateOffscreenContext3D() {
  if (!content::GpuDataManagerImpl::GetInstance()->CanUseGpuBrowserCompositor()) {
    return scoped_ptr<WGC3DCBI>();
  }

  content::CauseForGpuLaunch cause =
      content::CAUSE_FOR_GPU_LAUNCH_WEBGRAPHICSCONTEXT3DCOMMANDBUFFERIMPL_INITIALIZE;
  scoped_refptr<content::GpuChannelHost> gpu_channel_host(
      content::BrowserGpuChannelHostFactory::instance()->EstablishGpuChannelSync(cause));
  if (!gpu_channel_host.get()) {
    return scoped_ptr<WGC3DCBI>();
  }

  blink::WebGraphicsContext3D::Attributes attrs;
  attrs.shareResources = true;
  attrs.depth = false;
  attrs.stencil = false;
  attrs.antialias = false;
  attrs.noAutomaticFlushes = true;

  return make_scoped_ptr(WGC3DCBI::CreateOffscreenContext(
      gpu_channel_host.get(), attrs, false, GURL(),
      content::WebGraphicsContext3DCommandBufferImpl::SharedMemoryLimits(),
      nullptr));
}

} // namespace

Compositor::Compositor(CompositorClient* client)
    : client_(client),
      proxy_(new CompositorSingleThreadProxy(this)),
      surface_id_allocator_(
          CompositorUtils::GetInstance()->CreateSurfaceIdAllocator()),
      num_failed_recreate_attempts_(0),
      device_scale_factor_(1.0f),
      root_layer_(cc::Layer::Create()),
      next_output_surface_id_(1),
      pending_swaps_(0),
      weak_factory_(this) {}

void Compositor::SwapCompositorFrameAckFromClient(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  TRACE_EVENT_ASYNC_END1("cc", "oxide::Compositor:pending_swaps",
                         this,
                         "pending_swaps", pending_swaps_);
  --pending_swaps_;

  for (auto& frame : returned_frames) {
    CHECK(frame->HasOneRef());
    DCHECK_EQ(frame->proxy_.get(), proxy_.get());
  }
  proxy_->DidSwapCompositorFrame(surface_id, std::move(returned_frames));
}

scoped_ptr<cc::OutputSurface> Compositor::CreateOutputSurface() {
  uint32_t output_surface_id = next_output_surface_id_++;

  scoped_refptr<cc::ContextProvider> context_provider;
  scoped_ptr<cc::OutputSurface> surface;
  if (CompositorUtils::GetInstance()->CanUseGpuCompositing()) {
    context_provider =
        content::ContextProviderCommandBuffer::Create(
            CreateOffscreenContext3D(), content::CONTEXT_TYPE_UNKNOWN);
    if (!context_provider.get()) {
      return nullptr;
    }
    surface.reset(new CompositorOutputSurfaceGL(output_surface_id,
                                                context_provider,
                                                proxy_));
  } else {
    scoped_ptr<CompositorSoftwareOutputDevice> output_device(
        new CompositorSoftwareOutputDevice());
    surface.reset(new CompositorOutputSurfaceSoftware(output_surface_id,
                                                      std::move(output_device),
                                                      proxy_));
  }

  cc::SurfaceManager* manager =
      CompositorUtils::GetInstance()->GetSurfaceManager();
  display_client_.reset(
      new cc::OnscreenDisplayClient(
          std::move(surface),
          manager,
          content::HostSharedBitmapManager::current(),
          content::BrowserGpuMemoryBufferManager::current(),
          cc::RendererSettings(),
          base::ThreadTaskRunnerHandle::Get()));
  scoped_ptr<cc::SurfaceDisplayOutputSurface> output_surface(
      new cc::SurfaceDisplayOutputSurface(
          manager,
          surface_id_allocator_.get(),
          context_provider,
          nullptr));
  display_client_->set_surface_output_surface(output_surface.get());
  output_surface->set_display_client(display_client_.get());
  display_client_->display()->Resize(layer_tree_host_->device_viewport_size());

  return std::move(output_surface);
}

void Compositor::AddObserver(CompositorObserver* observer) {
  observers_.AddObserver(observer);
}

void Compositor::RemoveObserver(CompositorObserver* observer) {
  observers_.RemoveObserver(observer);
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

void Compositor::RequestNewOutputSurface() {
  scoped_ptr<cc::OutputSurface> surface(CreateOutputSurface());
  if (!surface) {
    DidFailToInitializeOutputSurface();
    return;
  }

  layer_tree_host_->SetOutputSurface(std::move(surface));
}

void Compositor::DidInitializeOutputSurface() {
  num_failed_recreate_attempts_ = 0;
}

void Compositor::DidFailToInitializeOutputSurface() {
  num_failed_recreate_attempts_++;

  CHECK_LE(num_failed_recreate_attempts_, 4);

  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE,
      base::Bind(&Compositor::RequestNewOutputSurface,
                 weak_factory_.GetWeakPtr()));
}

void Compositor::WillCommit() {}

void Compositor::DidCommit() {
  FOR_EACH_OBSERVER(CompositorObserver, observers_, CompositorDidCommit());
}

void Compositor::DidCommitAndDrawFrame() {}
void Compositor::DidCompleteSwapBuffers() {}
void Compositor::RecordFrameTimingEvents(
    scoped_ptr<cc::FrameTimingTracker::CompositeTimingSet> composite_events,
    scoped_ptr<cc::FrameTimingTracker::MainFrameTimingSet> main_frame_events) {}
void Compositor::DidCompletePageScaleAnimation() {}

void Compositor::DidPostSwapBuffers() {}
void Compositor::DidAbortSwapBuffers() {}

void Compositor::SwapCompositorFrameFromProxy(
    uint32_t surface_id,
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(CalledOnValidThread());
  FOR_EACH_OBSERVER(CompositorObserver,
                    observers_,
                    CompositorWillRequestSwapFrame());

  // XXX: What if we are hidden?
  // XXX: Should we check that surface_id matches the last created
  //  surface?
  TRACE_EVENT_ASYNC_BEGIN1("cc", "oxide::Compositor:pending_swaps",
                           this,
                           "pending_swaps", pending_swaps_);
  ++pending_swaps_;

  scoped_refptr<CompositorFrameHandle> handle =
      new CompositorFrameHandle(surface_id, proxy_, std::move(frame));
  client_->CompositorSwapFrame(
      handle.get(),
      base::Bind(&Compositor::SwapCompositorFrameAckFromClient,
                 weak_factory_.GetWeakPtr(),
                 surface_id));

  FOR_EACH_OBSERVER(CompositorObserver,
                    observers_,
                    CompositorDidRequestSwapFrame());
}

// static
scoped_ptr<Compositor> Compositor::Create(CompositorClient* client) {
  DCHECK(client);
  return make_scoped_ptr(new Compositor(client));
}

Compositor::~Compositor() {
  FOR_EACH_OBSERVER(CompositorObserver, observers_, OnCompositorDestruction());
  proxy_->ClientDestroyed();
}

bool Compositor::IsActive() const {
  return layer_tree_host_.get() != nullptr;
}

void Compositor::SetVisibility(bool visible) {
  if (!visible) {
    layer_tree_host_.reset();
    display_client_.reset();
  } else if (!layer_tree_host_) {
    cc::LayerTreeSettings settings;
    settings.use_external_begin_frame_source = false;
    settings.renderer_settings.allow_antialiasing = false;

    cc::LayerTreeHost::InitParams params;
    params.client = this;
    params.shared_bitmap_manager = content::HostSharedBitmapManager::current();
    params.gpu_memory_buffer_manager =
        content::BrowserGpuMemoryBufferManager::current();
    params.task_graph_runner =
        CompositorUtils::GetInstance()->GetTaskGraphRunner();
    params.settings = &settings;
    params.main_task_runner = base::ThreadTaskRunnerHandle::Get();

    layer_tree_host_ = cc::LayerTreeHost::CreateSingleThreaded(this, &params);

    layer_tree_host_->SetRootLayer(root_layer_);
    layer_tree_host_->SetVisible(true);
    layer_tree_host_->SetViewportSize(size_);
    layer_tree_host_->SetDeviceScaleFactor(device_scale_factor_);
  }
}

void Compositor::SetDeviceScaleFactor(float scale) {
  if (scale == device_scale_factor_) {
    return;
  }

  device_scale_factor_ = scale;

  if (layer_tree_host_) {
    layer_tree_host_->SetDeviceScaleFactor(scale);
  }
}

void Compositor::SetViewportSize(const gfx::Size& size) {
  if (size == size_) {
    return;
  }

  size_ = size;

  if (layer_tree_host_) {
    layer_tree_host_->SetViewportSize(size);
  }
  if (display_client_) {
    display_client_->display()->Resize(size);
  }
  root_layer_->SetBounds(size);
}

void Compositor::SetRootLayer(scoped_refptr<cc::Layer> layer) {
  root_layer_->RemoveAllChildren();
  if (layer.get()) {
    root_layer_->AddChild(layer);
  }
}

} // namespace oxide
