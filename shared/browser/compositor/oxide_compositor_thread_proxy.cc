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

#include "oxide_compositor_thread_proxy.h"

#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/output/context_provider.h"
#include "cc/output/gl_frame_data.h"
#include "cc/output/software_frame_data.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "gpu/command_buffer/client/context_support.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/GLES2/gl2extchromium.h"

#include "oxide_compositor.h"
#include "oxide_compositor_frame_handle.h"
#include "oxide_compositor_gpu_shims.h"
#include "oxide_compositor_output_surface.h"
#include "oxide_compositor_utils.h"

namespace oxide {

CompositorThreadProxy::~CompositorThreadProxy() {}

void CompositorThreadProxy::GetTextureFromMailboxResponseOnOwnerThread(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    GLuint texture) {
  base::AutoLock lock(mb_data_map_lock_);
  if (surface_id != surface_id_for_mb_data_map_) {
    return;
  }

  DCHECK(mb_data_map_.find(mailbox) == mb_data_map_.end());

  MailboxBufferData data;
  data.texture = texture;

  mb_data_map_[mailbox] = data;
}

void CompositorThreadProxy::CreateEGLImageFromMailboxResponseOnOwnerThread(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    EGLImageKHR egl_image) {
  base::AutoLock lock(mb_data_map_lock_);
  if (surface_id != surface_id_for_mb_data_map_) {
    return;
  }

  DCHECK(mb_data_map_.find(mailbox) == mb_data_map_.end());

  MailboxBufferData data;
  data.egl_image.ref_count = 1;
  data.egl_image.image = egl_image;

  mb_data_map_[mailbox] = data;
}

void CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread(
    uint32 surface_id,
    const gfx::Size& size,
    float scale,
    unsigned id,
    const gfx::Rect& damage_rect,
    const cc::SharedBitmapId& bitmap_id) {
  scoped_ptr<cc::SharedBitmap> bitmap(
      content::HostSharedBitmapManager::current()->GetSharedBitmapFromId(
        size, bitmap_id));
  if (!bitmap) {
    // This occurs if the output surface was destroyed
    return;
  }

  scoped_refptr<CompositorFrameHandle> frame(
      new CompositorFrameHandle(surface_id, this, size, scale));
  frame->software_frame_data_.reset(
      new SoftwareFrameData(id, damage_rect, bitmap->pixels()));

  if (!owner().compositor) {
    DidSkipSwapCompositorFrame(surface_id, &frame);
    return;
  }

  owner().compositor->SendSwapCompositorFrameToClient(surface_id, frame.get());
}

void CompositorThreadProxy::DidCompleteGLFrameOnImplThread(
    scoped_ptr<cc::CompositorFrame> frame) {
  owner_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(&CompositorThreadProxy::SendSwapGLFrameOnOwnerThread,
                 this,
                 impl().output_surface->surface_id(),
                 frame->gl_frame_data->size,
                 frame->metadata.device_scale_factor,
                 frame->gl_frame_data->mailbox));
}

void CompositorThreadProxy::SendSwapGLFrameOnOwnerThread(
    uint32_t surface_id,
    const gfx::Size& size,
    float scale,
    const gpu::Mailbox& mailbox) {
  scoped_refptr<CompositorFrameHandle> frame(
      new CompositorFrameHandle(surface_id, this, size, scale));

  {
    base::AutoLock lock(mb_data_map_lock_);

    auto it = mb_data_map_.find(mailbox);
    if (it == mb_data_map_.end()) {
      // TODO(chrisccoulson): Handle case where data hasn't been received from
      //  the GPU thread yet
      frame->gl_frame_data_.reset(new GLFrameData(mailbox, 0));
      DidSkipSwapCompositorFrame(surface_id, &frame);
      return;
    }

    if (mode_ == COMPOSITING_MODE_TEXTURE) {
      frame->gl_frame_data_.reset(new GLFrameData(mailbox, it->second.texture));
    } else {
      DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
      frame->image_frame_data_.reset(
          new ImageFrameData(mailbox, it->second.egl_image.image));
      ++it->second.egl_image.ref_count;
    }

    if (!owner().compositor) {
      DidSkipSwapCompositorFrame(surface_id, &frame);
      return;
    }
  }

  owner().compositor->SendSwapCompositorFrameToClient(surface_id, frame.get());
}

void CompositorThreadProxy::DidSkipSwapCompositorFrame(
    uint32_t surface_id,
    scoped_refptr<CompositorFrameHandle>* frame) {
  FrameHandleVector frames;
  frames.push_back(*frame);
  *frame = nullptr;

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, frames));
}

void CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread(
    uint32 surface_id,
    FrameHandleVector returned_frames) {
  if (impl().output_surface &&
      surface_id == impl().output_surface->surface_id()) {
    impl().output_surface->DidSwapBuffers();
  }

  while (!returned_frames.empty()) {
    scoped_refptr<CompositorFrameHandle> frame(returned_frames.back());
    returned_frames.pop_back();

    if (!frame.get()) {
      continue;
    }

    cc::CompositorFrameAck ack;
    if (frame->gl_frame_data() || frame->image_frame_data()) {
      ack.gl_frame_data.reset(new cc::GLFrameData());
      ack.gl_frame_data->size = frame->size_in_pixels();
      if (frame->gl_frame_data()) {
        scoped_ptr<GLFrameData> gl_frame_data = frame->gl_frame_data_.Pass();
        ack.gl_frame_data->mailbox = gl_frame_data->mailbox();
      } else {
        scoped_ptr<ImageFrameData> image_frame_data =
            frame->image_frame_data_.Pass();
        ack.gl_frame_data->mailbox = image_frame_data->mailbox();
        UnrefMailboxBufferDataForEGLImage(image_frame_data->mailbox());
      }
    } else if (frame->software_frame_data()) {
      scoped_ptr<SoftwareFrameData> software_frame_data =
          frame->software_frame_data_.Pass();
      ack.last_software_frame_id = software_frame_data->id();
    } else {
      NOTREACHED();
    }

    if (impl().output_surface &&
        frame->surface_id_ == impl().output_surface->surface_id()) {
      impl().output_surface->ReclaimResources(ack);
    }
  }
}

void CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread(
    uint32 surface_id,
    const gfx::Size& size_in_pixels,
    scoped_ptr<GLFrameData> gl_frame_data,
    scoped_ptr<SoftwareFrameData> software_frame_data,
    scoped_ptr<ImageFrameData> image_frame_data) {
  cc::CompositorFrameAck ack;
  if (gl_frame_data.get() || image_frame_data.get()) {
    ack.gl_frame_data.reset(new cc::GLFrameData());
    ack.gl_frame_data->size = size_in_pixels;
    if (gl_frame_data.get()) {
      ack.gl_frame_data->mailbox = gl_frame_data->mailbox();
    } else {
      ack.gl_frame_data->mailbox = image_frame_data->mailbox();
      UnrefMailboxBufferDataForEGLImage(image_frame_data->mailbox());
    }
  } else if (software_frame_data.get()) {
    ack.last_software_frame_id = software_frame_data->id();
  } else {
    NOTREACHED();
  }

  if (!impl().output_surface ||
      surface_id != impl().output_surface->surface_id()) {
    return;
  }

  impl().output_surface->ReclaimResources(ack);
}

void CompositorThreadProxy::UnrefMailboxBufferDataForEGLImage(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);

  base::AutoLock lock(mb_data_map_lock_);
  auto it = mb_data_map_.find(mailbox);
  DCHECK(it != mb_data_map_.end());

  DCHECK_GT(it->second.egl_image.ref_count, 0);
  if (--it->second.egl_image.ref_count == 0) {
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                   GpuUtils::GetHardwareEGLDisplay(),
                   it->second.egl_image.image));
    mb_data_map_.erase(it);
  }
}

CompositorThreadProxy::OwnerData& CompositorThreadProxy::owner() {
  DCHECK(owner_thread_checker_.CalledOnValidThread());
  return owner_unsafe_access_;
}

CompositorThreadProxy::ImplData& CompositorThreadProxy::impl() {
  DCHECK(impl_thread_checker_.CalledOnValidThread());
  return impl_unsafe_access_;
}

CompositorThreadProxy::CompositorThreadProxy(Compositor* compositor)
    : mode_(CompositorUtils::GetInstance()->GetCompositingMode()),
      owner_message_loop_(base::MessageLoopProxy::current()),
      surface_id_for_mb_data_map_(0) {
  owner().compositor = compositor;
  impl_thread_checker_.DetachFromThread();
}

void CompositorThreadProxy::CompositorDestroyed() {
  owner().compositor = nullptr;
}

void CompositorThreadProxy::SetOutputSurface(
    CompositorOutputSurface* output_surface) {
  DCHECK(!output_surface || !impl().output_surface);
  DCHECK(!impl_message_loop_ ||
         impl_message_loop_ == base::MessageLoopProxy::current());

  impl().output_surface = output_surface;
  impl_message_loop_ = base::MessageLoopProxy::current();

  base::AutoLock lock(mb_data_map_lock_);
  surface_id_for_mb_data_map_ =
      output_surface ? output_surface->surface_id() : 0;
}

void CompositorThreadProxy::MailboxBufferCreated(const gpu::Mailbox& mailbox,
                                                 uint32_t sync_point) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    CompositorUtils::GetInstance()->GetTextureFromMailbox(
        impl().output_surface->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorThreadProxy::GetTextureFromMailboxResponseOnOwnerThread,
          this, impl().output_surface->surface_id(), mailbox),
        owner_message_loop_);
  } else {
    DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
    CompositorUtils::GetInstance()->CreateEGLImageFromMailbox(
        impl().output_surface->context_provider(),
        mailbox,
        sync_point,
        base::Bind(
          &CompositorThreadProxy::CreateEGLImageFromMailboxResponseOnOwnerThread,
          this, impl().output_surface->surface_id(), mailbox),
        owner_message_loop_);
  }
}

void CompositorThreadProxy::MailboxBufferDestroyed(
    const gpu::Mailbox& mailbox) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    base::AutoLock lock(mb_data_map_lock_);
    size_t rv = mb_data_map_.erase(mailbox);
    DCHECK_GT(rv, 0U);
  } else {
    UnrefMailboxBufferDataForEGLImage(mailbox);
  }
}

void CompositorThreadProxy::SwapCompositorFrame(cc::CompositorFrame* frame) {
  switch (mode_) {
    case COMPOSITING_MODE_TEXTURE:
    case COMPOSITING_MODE_EGLIMAGE: {
      DCHECK(frame->gl_frame_data);

      scoped_ptr<cc::CompositorFrame> f(new cc::CompositorFrame());
      f->gl_frame_data = frame->gl_frame_data.Pass();
      f->metadata = frame->metadata;

      cc::ContextProvider* context_provider =
          impl().output_surface->context_provider();
      gpu::gles2::GLES2Interface* gl = context_provider->ContextGL();

      uint32_t query_id;
      gl->GenQueriesEXT(1, &query_id);
      gl->BeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query_id);
      context_provider->ContextSupport()->SignalQuery(
          query_id,
          base::Bind(&CompositorThreadProxy::DidCompleteGLFrameOnImplThread,
                     this, base::Passed(&f)));
      gl->EndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);
      gl->Flush();
      gl->DeleteQueriesEXT(1, &query_id);
      break;
    }
    case COMPOSITING_MODE_SOFTWARE: {
      DCHECK(frame->software_frame_data);

      cc::SoftwareFrameData* software_frame_data = frame->software_frame_data.get();

      owner_message_loop_->PostTask(
          FROM_HERE,
          base::Bind(&CompositorThreadProxy::SendSwapSoftwareFrameOnOwnerThread,
                     this,
                     impl().output_surface->surface_id(),
                     software_frame_data->size,
                     frame->metadata.device_scale_factor,
                     software_frame_data->id,
                     software_frame_data->damage_rect,
                     software_frame_data->bitmap_id));
      break;
    }

    default:
      NOTREACHED();
  }
}

void CompositorThreadProxy::DidSwapCompositorFrame(
    uint32_t surface_id,
    FrameHandleVector returned_frames) {
  for (auto it = returned_frames.begin(); it != returned_frames.end(); ++it) {
    // This is a CHECK because CompositorFrameHandle isn't actually thread safe
    // and we're going to clear some of its members. There are potential
    // security implications of there being a reference outside of the
    // compositor now
    CHECK((*it)->HasOneRef())
        << "Returned a frame that's still referenced from outside of the "
        << "compositor";
    DCHECK((*it)->proxy_ == this) << "Frame returned to wrong compositor";
  }

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendDidSwapBuffersToOutputSurfaceOnImplThread,
        this, surface_id, std::move(returned_frames)));
}

void CompositorThreadProxy::ReclaimResourcesForFrame(
    CompositorFrameHandle* frame) {
  DCHECK(frame);
  DCHECK(frame->proxy_ == this);

  impl_message_loop_->PostTask(
      FROM_HERE,
      base::Bind(
        &CompositorThreadProxy::SendReclaimResourcesToOutputSurfaceOnImplThread,
        this, frame->surface_id_, frame->size_in_pixels(),
        base::Passed(&frame->gl_frame_data_),
        base::Passed(&frame->software_frame_data_),
        base::Passed(&frame->image_frame_data_)));
}

} // namespace oxide
