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

#include "oxide_compositor_output_surface_gl.h"

#include "base/logging.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_ack.h"
#include "cc/output/context_provider.h"
#include "cc/output/gl_frame_data.h"
#include "cc/output/output_surface_client.h"
#include "cc/resources/resource_format.h"
#include "cc/resources/resource_provider.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/common/sync_token.h"

#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_thread_proxy.h"

namespace oxide {

void CompositorOutputSurfaceGL::DetachFromClient() {
  DCHECK(CalledOnValidThread());

  DiscardBackbuffer();
  while (!pending_buffers_.empty()) {
    BufferData& buffer = pending_buffers_.front();
    DiscardBuffer(&buffer);
    pending_buffers_.pop_front();
  }

  CompositorOutputSurface::DetachFromClient();
}

void CompositorOutputSurfaceGL::EnsureBackbuffer() {
  DCHECK(CalledOnValidThread());
  is_backbuffer_discarded_ = false;

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (!back_buffer_.texture_id && !returned_buffers_.empty()) {
    back_buffer_ = returned_buffers_.front();
    returned_buffers_.pop();
    DCHECK(back_buffer_.size == surface_size_);
  }

  if (!back_buffer_.texture_id) {
    gl->GenTextures(1, &back_buffer_.texture_id);
    back_buffer_.size = surface_size_;
    gl->BindTexture(GL_TEXTURE_2D, back_buffer_.texture_id);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    cc::ResourceFormat format = cc::RGBA_8888;
    gl->TexImage2D(GL_TEXTURE_2D,
                   0,
                   cc::GLInternalFormat(format),
                   surface_size_.width(),
                   surface_size_.height(),
                   0,
                   cc::GLDataFormat(format),
                   cc::GLDataType(format),
                   nullptr);
    gl->GenMailboxCHROMIUM(back_buffer_.mailbox.name);
    gl->ProduceTextureCHROMIUM(GL_TEXTURE_2D, back_buffer_.mailbox.name);

    GLuint64 sync_point = gl->InsertFenceSyncCHROMIUM();
    gl->ShallowFlushCHROMIUM();

    // We don't use the SyncToken. However, generating it ensures that the
    // sync point has been flushed to the GPU thread before continuing
    gpu::SyncToken token;
    gl->GenSyncTokenCHROMIUM(sync_point, token.GetData());

    proxy_->MailboxBufferCreated(back_buffer_.mailbox, sync_point);
  }
}

void CompositorOutputSurfaceGL::DiscardBackbuffer() {
  DCHECK(CalledOnValidThread());
  if (is_backbuffer_discarded_) {
    return;
  }

  is_backbuffer_discarded_ = true;

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (back_buffer_.texture_id) {
    DiscardBuffer(&back_buffer_);
    back_buffer_ = BufferData();
  }

  while (!returned_buffers_.empty()) {
    BufferData& buffer = returned_buffers_.front();
    DiscardBuffer(&buffer);
    returned_buffers_.pop();
  }

  if (fbo_) {
    gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl->DeleteFramebuffers(1, &fbo_);
    fbo_ = 0;
  }
}

void CompositorOutputSurfaceGL::Reshape(const gfx::Size& size,
                                        float scale_factor,
                                        bool has_alpha) {
  if (surface_size_ == size && device_scale_factor_ == scale_factor) {
    return;
  }

  surface_size_ = size;
  device_scale_factor_ = scale_factor;

  DiscardBackbuffer();
  EnsureBackbuffer();
}

void CompositorOutputSurfaceGL::BindFramebuffer() {
  DCHECK(CalledOnValidThread());

  EnsureBackbuffer();
  DCHECK(back_buffer_.texture_id);
  DCHECK(back_buffer_.size == surface_size_);

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (!fbo_) {
    gl->GenFramebuffers(1, &fbo_);
  }
  gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
  gl->FramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           back_buffer_.texture_id,
                           0);
}

void CompositorOutputSurfaceGL::SwapBuffers(cc::CompositorFrame* frame) {
  DCHECK(CalledOnValidThread());
  DCHECK(frame->gl_frame_data);
  DCHECK(!back_buffer_.mailbox.IsZero());
  DCHECK(surface_size_ == back_buffer_.size);
  DCHECK(frame->gl_frame_data->size == back_buffer_.size);
  DCHECK(!back_buffer_.size.IsEmpty());

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();
  gl->Flush();

  CompositorFrameData data;
  data.size_in_pixels = back_buffer_.size;
  data.device_scale = frame->metadata.device_scale_factor;
  data.gl_frame_data = make_scoped_ptr(new GLFrameData());
  data.gl_frame_data->mailbox = back_buffer_.mailbox;

  DoSwapBuffers(&data);

  pending_buffers_.push_back(back_buffer_);
  back_buffer_ = BufferData();
}

void CompositorOutputSurfaceGL::ReclaimResources(
    const CompositorFrameAck& ack) {
  DCHECK(CalledOnValidThread());
  DCHECK_EQ(ack.software_frame_id, 0U);
  DCHECK(!ack.gl_frame_mailbox.IsZero());

  std::deque<BufferData>::iterator it;
  for (it = pending_buffers_.begin(); it != pending_buffers_.end(); ++it) {
    DCHECK(!it->mailbox.IsZero());
    if (memcmp(it->mailbox.name,
               ack.gl_frame_mailbox.name,
               sizeof(it->mailbox.name)) == 0) {
      break;
    }
  }

  CHECK(it != pending_buffers_.end());

  it->sync_point = 0;

  if (!is_backbuffer_discarded_ && it->size == surface_size_) {
    returned_buffers_.push(*it);
  } else {
    DiscardBuffer(&(*it));
  }

  pending_buffers_.erase(it);

  CompositorOutputSurface::ReclaimResources(ack);
}

void CompositorOutputSurfaceGL::DiscardBuffer(BufferData* buffer) {
  context_provider_->ContextGL()->DeleteTextures(1, &buffer->texture_id);
  proxy_->MailboxBufferDestroyed(buffer->mailbox);
}

CompositorOutputSurfaceGL::CompositorOutputSurfaceGL(
    uint32_t surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    scoped_refptr<CompositorThreadProxy> proxy)
    : CompositorOutputSurface(surface_id, context_provider, proxy),
      is_backbuffer_discarded_(false),
      fbo_(0) {
  capabilities_.uses_default_gl_framebuffer = false;
}

CompositorOutputSurfaceGL::~CompositorOutputSurfaceGL() {}

} // namespace oxide
