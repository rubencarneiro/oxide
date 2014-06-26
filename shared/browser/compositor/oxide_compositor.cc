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

#include "oxide_compositor.h"

#include "base/logging.h"
#include "cc/layers/layer.h"
#include "cc/output/context_provider.h"
#include "cc/trees/layer_tree_host.h"
#include "cc/trees/layer_tree_settings.h"
#include "content/common/host_shared_bitmap_manager.h"

#include "oxide_compositor_client.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_output_surface_gl.h"
#include "oxide_compositor_output_surface_software.h"
#include "oxide_compositor_software_output_device.h"
#include "oxide_compositor_thread_proxy.h"
#include "oxide_compositor_utils.h"

namespace oxide {

Compositor::Compositor(CompositorClient* client, bool software)
    : client_(client),
      use_software_(software),
      device_scale_factor_(1.0f),
      root_layer_(cc::Layer::Create()),
      proxy_(new CompositorThreadProxy(this)),
      next_output_surface_id_(1),
      lock_count_(0)  {
  DCHECK(CalledOnValidThread());
}

void Compositor::SendSwapCompositorFrameToClient(
    uint32 surface_id,
    CompositorFrameHandle* frame) {
  DCHECK(CalledOnValidThread());
  // XXX: What if we are hidden?
  // XXX: Should we check that surface_id matches the last created
  //  surface?
  client_->CompositorSwapFrame(surface_id, frame);
}

void Compositor::LockCompositor() {
  DCHECK(CalledOnValidThread());
  if (lock_count_++ > 0) {
    return;
  }

  if (layer_tree_host_) {
    layer_tree_host_->SetDeferCommits(true);
  }
}

void Compositor::UnlockCompositor() {
  DCHECK(CalledOnValidThread());
  DCHECK(lock_count_ > 0);
  if (--lock_count_ > 0) {
    return;
  }

  if (layer_tree_host_) {
    layer_tree_host_->SetDeferCommits(false);
  }
}

void Compositor::WillBeginMainFrame(int frame_id) {}
void Compositor::DidBeginMainFrame() {}
void Compositor::Animate(base::TimeTicks frame_begin_time) {}
void Compositor::Layout() {}
void Compositor::ApplyScrollAndScale(const gfx::Vector2d& scroll_delta,
                                     float page_scale) {}

scoped_ptr<cc::OutputSurface> Compositor::CreateOutputSurface(bool fallback) {
  DCHECK(CalledOnValidThread());

  // Don't use the provided fallback path, we need the browser side and
  // renderer sides to be in sync, so this would probably result in no
  // output anyway
  if (fallback) {
    return scoped_ptr<cc::OutputSurface>();
  }

  uint32 output_surface_id = next_output_surface_id_++;

  if (!use_software_) {
    scoped_refptr<cc::ContextProvider> context_provider =
        CompositorUtils::GetInstance()->GetContextProvider();
    if (!context_provider) {
      return scoped_ptr<cc::OutputSurface>();
    }
    scoped_ptr<CompositorOutputSurfaceGL> output(
        new CompositorOutputSurfaceGL(output_surface_id,
                                      context_provider,
                                      proxy_));
    return output.PassAs<cc::OutputSurface>();
  }

  scoped_ptr<CompositorSoftwareOutputDevice> output_device(
      new CompositorSoftwareOutputDevice());
  scoped_ptr<CompositorOutputSurfaceSoftware> output(
      new CompositorOutputSurfaceSoftware(
        output_surface_id,
        output_device.PassAs<cc::SoftwareOutputDevice>(),
        proxy_));
  return output.PassAs<cc::OutputSurface>();
}

void Compositor::DidInitializeOutputSurface() {}
void Compositor::WillCommit() {}

void Compositor::DidCommit() {
  client_->CompositorDidCommit();
}

void Compositor::DidCommitAndDrawFrame() {}
void Compositor::DidCompleteSwapBuffers() {}

// static
scoped_ptr<Compositor> Compositor::Create(CompositorClient* client,
                                          bool software) {
  if (!client) {
    return scoped_ptr<Compositor>();
  }

  return make_scoped_ptr(new Compositor(client, software));
}

Compositor::~Compositor() {
  CHECK_EQ(lock_count_, 0);
  proxy_->CompositorDestroyed();
}

bool Compositor::IsActive() const {
  return layer_tree_host_.get() != NULL;
}

void Compositor::SetVisibility(bool visible) {
  DCHECK(CalledOnValidThread());
  if (!visible) {
    layer_tree_host_.reset();
  } else if (!layer_tree_host_) {
    cc::LayerTreeSettings settings;
    settings.allow_antialiasing = false;
    settings.begin_frame_scheduling_enabled = false;
    settings.throttle_frame_production = false;
    settings.using_synchronous_renderer_compositor = true;

    layer_tree_host_ = cc::LayerTreeHost::CreateThreaded(
        this, content::HostSharedBitmapManager::current(), settings,
        CompositorUtils::GetInstance()->GetMessageLoopProxy());

    layer_tree_host_->SetRootLayer(root_layer_);
    layer_tree_host_->SetVisible(true);
    layer_tree_host_->SetViewportSize(size_);
    layer_tree_host_->SetDeviceScaleFactor(device_scale_factor_);

    if (lock_count_ > 0) {
      layer_tree_host_->SetDeferCommits(true);
    }

    layer_tree_host_->SetLayerTreeHostClientReady();
  }
}

void Compositor::SetDeviceScaleFactor(float scale) {
  DCHECK(CalledOnValidThread());
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
  if (layer) {
    root_layer_->AddChild(layer);
  }
}

void Compositor::DidSwapCompositorFrame(uint32 surface_id,
                                        FrameHandleVector& returned_frames) {
  DCHECK(CalledOnValidThread());
  proxy_->DidSwapCompositorFrame(surface_id, returned_frames);
}

} // namespace oxide
