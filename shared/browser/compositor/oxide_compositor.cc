// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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
#include "cc/layers/layer.h"
#include "cc/output/context_provider.h"
#include "cc/scheduler/begin_frame_source.h"
#include "cc/trees/layer_tree_host.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "content/browser/gpu/browser_gpu_memory_buffer_manager.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "content/common/gpu/client/gpu_channel_host.h"
#include "content/common/gpu/client/webgraphicscontext3d_command_buffer_impl.h"
#include "content/common/gpu/gpu_process_launch_causes.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "url/gurl.h"

#include "oxide_compositor_client.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_observer.h"
#include "oxide_compositor_output_surface_gl.h"
#include "oxide_compositor_output_surface_software.h"
#include "oxide_compositor_software_output_device.h"
#include "oxide_compositor_thread_proxy.h"
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
      num_failed_recreate_attempts_(0),
      device_scale_factor_(1.0f),
      root_layer_(cc::Layer::Create(cc::LayerSettings())),
      proxy_(new CompositorThreadProxy(this)),
      next_output_surface_id_(1),
      weak_factory_(this) {
  DCHECK(CalledOnValidThread());
}

void Compositor::SendSwapCompositorFrameToClient(
    scoped_ptr<CompositorFrameData> frame) {
  DCHECK(CalledOnValidThread());
  // XXX: What if we are hidden?
  // XXX: Should we check that surface_id matches the last created
  //  surface?
  scoped_refptr<CompositorFrameHandle> handle =
      new CompositorFrameHandle(proxy_, frame.Pass());
  client_->CompositorSwapFrame(handle.get());
}

scoped_ptr<cc::OutputSurface> Compositor::CreateOutputSurface() {
  DCHECK(CalledOnValidThread());

  uint32 output_surface_id = next_output_surface_id_++;

  if (CompositorUtils::GetInstance()->CanUseGpuCompositing()) {
    scoped_refptr<cc::ContextProvider> context_provider =
        content::ContextProviderCommandBuffer::Create(
          CreateOffscreenContext3D(), content::CONTEXT_TYPE_UNKNOWN);
    if (!context_provider.get()) {
      return scoped_ptr<cc::OutputSurface>();
    }
    scoped_ptr<CompositorOutputSurfaceGL> output(
        new CompositorOutputSurfaceGL(output_surface_id,
                                      context_provider,
                                      proxy_));
    return output.Pass();
  }

  scoped_ptr<CompositorSoftwareOutputDevice> output_device(
      new CompositorSoftwareOutputDevice());
  scoped_ptr<CompositorOutputSurfaceSoftware> output(
      new CompositorOutputSurfaceSoftware(
        output_surface_id,
        output_device.Pass(),
        proxy_));
  return output.Pass();
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
void Compositor::Layout() {}
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

  layer_tree_host_->SetOutputSurface(surface.Pass());
}

void Compositor::DidInitializeOutputSurface() {
  num_failed_recreate_attempts_ = 0;
}

void Compositor::DidFailToInitializeOutputSurface() {
  num_failed_recreate_attempts_++;

  CHECK_LE(num_failed_recreate_attempts_, 4);

  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&Compositor::RequestNewOutputSurface,
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

// static
scoped_ptr<Compositor> Compositor::Create(CompositorClient* client) {
  if (!client) {
    return scoped_ptr<Compositor>();
  }

  return make_scoped_ptr(new Compositor(client));
}

Compositor::~Compositor() {
  FOR_EACH_OBSERVER(CompositorObserver, observers_, OnCompositorDestruction());
  proxy_->CompositorDestroyed();
}

bool Compositor::IsActive() const {
  return layer_tree_host_.get() != nullptr;
}

void Compositor::SetVisibility(bool visible) {
  DCHECK(CalledOnValidThread());
  if (!visible) {
    layer_tree_host_.reset();
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

    layer_tree_host_ = cc::LayerTreeHost::CreateThreaded(
        CompositorUtils::GetInstance()->GetTaskRunner(),
        &params);

    layer_tree_host_->SetRootLayer(root_layer_);
    layer_tree_host_->SetVisible(true);
    layer_tree_host_->SetViewportSize(size_);
    layer_tree_host_->SetDeviceScaleFactor(device_scale_factor_);
  }
}

void Compositor::SetDeviceScaleFactor(float scale) {
  DCHECK(CalledOnValidThread());
  if (scale == device_scale_factor_) {
    return;
  }

  device_scale_factor_ = scale;

  if (layer_tree_host_) {
    layer_tree_host_->SetDeviceScaleFactor(scale);
  }
}

void Compositor::SetViewportSize(const gfx::Size& size) {
  DCHECK(CalledOnValidThread());
  if (size == size_) {
    return;
  }

  size_ = size;

  if (layer_tree_host_) {
    layer_tree_host_->SetViewportSize(size);
  }
  root_layer_->SetBounds(size);
}

void Compositor::SetRootLayer(scoped_refptr<cc::Layer> layer) {
  DCHECK(CalledOnValidThread());

  root_layer_->RemoveAllChildren();
  if (layer.get()) {
    root_layer_->AddChild(layer);
  }
}

void Compositor::DidSwapCompositorFrame(uint32 surface_id,
                                        FrameHandleVector returned_frames) {
  proxy_->DidSwapCompositorFrame(surface_id, std::move(returned_frames));
}

} // namespace oxide
