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

#include "oxide_compositor_output_surface_gl.h"

#include <memory>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "cc/output/context_provider.h"
#include "cc/output/output_surface_client.h"
#include "cc/output/output_surface_frame.h"
#include "cc/resources/resource_format.h"
#include "cc/resources/resource_provider.h"
#include "content/common/gpu/client/context_provider_command_buffer.h" // nogncheck
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/common/sync_token.h"

#include "oxide_compositor_frame_ack.h"
#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_output_surface_listener.h"

namespace oxide {

CompositorOutputSurfaceGL::CompositorOutputSurfaceGL(
    uint32_t surface_id,
    scoped_refptr<cc::ContextProvider> context_provider,
    CompositorOutputSurfaceListener* listener)
    : CompositorOutputSurface(surface_id, context_provider, listener),
      device_scale_factor_(1.f),
      back_buffer_(nullptr),
      is_backbuffer_discarded_(false),
      fbo_(0) {
  capabilities_.uses_default_gl_framebuffer = false;
}

CompositorOutputSurfaceGL::~CompositorOutputSurfaceGL() {
  for (auto& buffer : buffers_) {
    buffer.available = true;
  }
  DiscardBackbuffer();
}

void CompositorOutputSurfaceGL::EnsureBackbuffer() {
  is_backbuffer_discarded_ = false;

  if (!back_buffer_) {
    auto it = std::find_if(buffers_.begin(),
                           buffers_.end(),
                           [](const BufferData& buffer) {
      return buffer.texture_id > 0 && buffer.available;
    });

    if (it != buffers_.end()) {
      back_buffer_ = &(*it);
      back_buffer_->available = false;
    }
  }

  if (!back_buffer_) {
    gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

    auto it = std::find_if(buffers_.begin(),
                           buffers_.end(),
                           [](const BufferData& buffer) {
      return buffer.texture_id == 0U;
    });
    DCHECK(it != buffers_.end());

    back_buffer_ = &(*it);
    back_buffer_->available = false;
    back_buffer_->size = surface_size_;

    gl->GenTextures(1, &back_buffer_->texture_id);
    gl->BindTexture(GL_TEXTURE_2D, back_buffer_->texture_id);
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
    gl->GenMailboxCHROMIUM(back_buffer_->mailbox.name);
    gl->ProduceTextureCHROMIUM(GL_TEXTURE_2D, back_buffer_->mailbox.name);

    GLuint64 sync_point = gl->InsertFenceSyncCHROMIUM();
    gl->ShallowFlushCHROMIUM();

    // We don't use the SyncToken. However, generating it ensures that the
    // sync point has been flushed to the GPU thread before continuing
    gpu::SyncToken token;
    gl->GenSyncTokenCHROMIUM(sync_point, token.GetData());

    listener()->MailboxBufferCreated(back_buffer_->mailbox, sync_point);
  }

  DCHECK_NE(back_buffer_->texture_id, 0U);
  DCHECK(back_buffer_->size == surface_size_);
  DCHECK(!back_buffer_->available);
}

void CompositorOutputSurfaceGL::DiscardBackbuffer() {
  if (is_backbuffer_discarded_) {
    return;
  }

  is_backbuffer_discarded_ = true;

  if (back_buffer_) {
    back_buffer_->available = true;
    back_buffer_ = nullptr;
  }

  for (auto& buffer : buffers_) {
    DiscardBufferIfPossible(&buffer);
  }

  if (fbo_) {
    gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();
    gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl->DeleteFramebuffers(1, &fbo_);
    fbo_ = 0;
  }
}

void CompositorOutputSurfaceGL::Reshape(const gfx::Size& size,
                                        float scale_factor,
                                        const gfx::ColorSpace& color_space,
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
  EnsureBackbuffer();

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();

  if (!fbo_) {
    gl->GenFramebuffers(1, &fbo_);
  }
  gl->BindFramebuffer(GL_FRAMEBUFFER, fbo_);
  gl->FramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           back_buffer_->texture_id,
                           0);
}

uint32_t CompositorOutputSurfaceGL::GetFramebufferCopyTextureFormat() {
  content::ContextProviderCommandBuffer* gl =
      static_cast<content::ContextProviderCommandBuffer*>(context_provider());
  return gl->GetCopyTextureInternalFormat();
}

void CompositorOutputSurfaceGL::SwapBuffers(cc::OutputSurfaceFrame frame) {
  DCHECK(back_buffer_);
  DCHECK(!back_buffer_->mailbox.IsZero());
  DCHECK(surface_size_ == back_buffer_->size);
  DCHECK(frame.size == back_buffer_->size);
  DCHECK(!back_buffer_->size.IsEmpty());

  gpu::gles2::GLES2Interface* gl = context_provider_->ContextGL();
  gl->Flush();

  std::unique_ptr<CompositorFrameData> data(new CompositorFrameData());
  data->size_in_pixels = back_buffer_->size;
  data->device_scale = device_scale_factor_;
  data->gl_frame_data = base::WrapUnique(new GLFrameData());
  data->gl_frame_data->mailbox = back_buffer_->mailbox;

  DoSwapBuffers(std::move(data));

  back_buffer_ = nullptr;
}

void CompositorOutputSurfaceGL::ReclaimResources(
    const CompositorFrameAck& ack) {
  DCHECK_EQ(ack.software_frame_id, 0U);
  DCHECK(!ack.gl_frame_mailbox.IsZero());

  BufferData& buffer = GetBufferDataForMailbox(ack.gl_frame_mailbox);
  DCHECK(!buffer.available);
  buffer.available = true;

  if (is_backbuffer_discarded_ || buffer.size != surface_size_) {
    DiscardBufferIfPossible(&buffer);
  }

  int unavailable_count = std::count_if(buffers_.begin(),
                                        buffers_.end(),
                                        [](const BufferData& buffer) {
    return !buffer.available;
  });
  if (unavailable_count == 0) {
    listener()->AllFramesReturnedFromClient();
  }
}

CompositorOutputSurfaceGL::BufferData&
CompositorOutputSurfaceGL::GetBufferDataForMailbox(
    const gpu::Mailbox& mailbox) {
  auto it = std::find_if(buffers_.begin(),
                         buffers_.end(),
                         [&mailbox](const BufferData& buffer) {
    return memcmp(buffer.mailbox.name,
                  mailbox.name,
                  sizeof(mailbox.name)) == 0;
  });
  DCHECK(it != buffers_.end());

  return *it;
}

void CompositorOutputSurfaceGL::DiscardBufferIfPossible(BufferData* buffer) {
  if (!buffer->available) {
    return;
  }

  if (buffer->texture_id == 0) {
    return;
  }

  DCHECK(!buffer->mailbox.IsZero());

  context_provider_->ContextGL()->DeleteTextures(1, &buffer->texture_id);
  listener()->MailboxBufferDestroyed(buffer->mailbox);

  buffer->texture_id = 0;
  buffer->mailbox.SetZero();
  buffer->size = gfx::Size();
}

} // namespace oxide
