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

#include "oxide_compositor_thread_proxy.h"

namespace oxide {

void CompositorOutputSurfaceGL::EnsureBackbuffer() {
  is_backbuffer_discarded_ = false;

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (!backing_texture_.texture_id && !returned_textures_.empty()) {
    backing_texture_ = returned_textures_.front();
    returned_textures_.pop();
    DCHECK(backing_texture_.size == surface_size_);
  }

  if (!backing_texture_.texture_id) {
    gl->GenTextures(1, &backing_texture_.texture_id);
    backing_texture_.size = surface_size_;
    gl->BindTexture(GL_TEXTURE_2D, backing_texture_.texture_id);
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
                   NULL);
    gl->GenMailboxCHROMIUM(backing_texture_.mailbox.name);
    gl->ProduceTextureCHROMIUM(GL_TEXTURE_2D, backing_texture_.mailbox.name);
  }
}

void CompositorOutputSurfaceGL::DiscardBackbuffer() {
  if (is_backbuffer_discarded_) {
    return;
  }

  is_backbuffer_discarded_ = true;

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (backing_texture_.texture_id) {
    gl->DeleteTextures(1, &backing_texture_.texture_id);
    backing_texture_ = OutputFrameData();
  }

  while (!returned_textures_.empty()) {
    OutputFrameData& texture = returned_textures_.front();
    returned_textures_.pop();
    gl->DeleteTextures(1, &texture.texture_id);
  }

  if (fbo_) {
    gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl->DeleteFramebuffers(1, &fbo_);
    fbo_ = 0;
  }
}

void CompositorOutputSurfaceGL::Reshape(const gfx::Size& size,
                                        float scale_factor) {
  if (surface_size_ == size && device_scale_factor_ == scale_factor) {
    return;
  }

  surface_size_ = size;
  device_scale_factor_ = scale_factor;

  DiscardBackbuffer();
  EnsureBackbuffer();
}

void CompositorOutputSurfaceGL::BindFramebuffer() {
  EnsureBackbuffer();
  DCHECK(backing_texture_.texture_id);
  DCHECK(backing_texture_.size == surface_size_);

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (!fbo_) {
    gl->GenFramebuffers(1, &fbo_);
  }
  gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
  gl->FramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           backing_texture_.texture_id,
                           0);
}

void CompositorOutputSurfaceGL::SwapBuffers(cc::CompositorFrame* frame) {
  DCHECK(frame->gl_frame_data);
  DCHECK(!backing_texture_.mailbox.IsZero());
  DCHECK(surface_size_ == backing_texture_.size);
  DCHECK(frame->gl_frame_data->size == backing_texture_.size);
  DCHECK(!backing_texture_.size.IsEmpty());

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();
  gl->Flush();

  frame->gl_frame_data->mailbox = backing_texture_.mailbox;
  frame->gl_frame_data->sync_point = gl->InsertSyncPointCHROMIUM();

  CompositorOutputSurface::SwapBuffers(frame);

  pending_textures_.push_back(backing_texture_);
  backing_texture_ = OutputFrameData();
}

void CompositorOutputSurfaceGL::ReclaimResources(
    const cc::CompositorFrameAck& ack) {
  DCHECK(ack.gl_frame_data);
  DCHECK_EQ(ack.last_software_frame_id, 0);
  DCHECK(!ack.gl_frame_data->mailbox.IsZero());
  DCHECK(!ack.gl_frame_data->size.IsEmpty());

  std::deque<OutputFrameData>::iterator it;
  for (it = pending_textures_.begin(); it != pending_textures_.end(); ++it) {
    DCHECK(!it->mailbox.IsZero());
    if (memcmp(it->mailbox.name,
               ack.gl_frame_data->mailbox.name,
               sizeof(it->mailbox.name)) == 0) {
      DCHECK(it->size == ack.gl_frame_data->size);
      break;
    }
  }

  CHECK(it != pending_textures_.end());

  it->sync_point = 0;

  if (!is_backbuffer_discarded_ && it->size == surface_size_) {
    returned_textures_.push(*it);
  } else {
    context_provider_->ContextGL()->DeleteTextures(1, &it->texture_id);
  }

  pending_textures_.erase(it);

  CompositorOutputSurface::ReclaimResources(ack);
}

CompositorOutputSurfaceGL::CompositorOutputSurfaceGL(
    uint32 surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    scoped_refptr<CompositorThreadProxy> proxy)
    : CompositorOutputSurface(surface_id, context_provider, proxy),
      is_backbuffer_discarded_(false),
      fbo_(0) {
  capabilities_.uses_default_gl_framebuffer = false;
}

CompositorOutputSurfaceGL::~CompositorOutputSurfaceGL() {
  DiscardBackbuffer();
  while (!pending_textures_.empty()) {
    OutputFrameData& texture = pending_textures_.front();
    pending_textures_.pop_front();
    context_provider_->ContextGL()->DeleteTextures(1, &texture.texture_id);
  }
}

} // namespace oxide
