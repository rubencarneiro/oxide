// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_compositor_gpu_shims.h"

#include "base/single_thread_task_runner.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/context_state.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_egl.h"

#include "shared/port/content/common/gpu_service_shim_oxide.h"

namespace oxide {

class ScopedTextureBinder {
 public:
  ScopedTextureBinder(GLenum target,
                      GLuint texture,
                      const gpu::gles2::ContextState* state)
      : target_(target),
        state_(state) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, texture);
  }

  ~ScopedTextureBinder() {
    const gpu::gles2::TextureUnit& info = state_->texture_units[0];
    GLuint last_id;
    scoped_refptr<gpu::gles2::TextureRef> texture_ref;
    switch (target_) {
      case GL_TEXTURE_2D:
        texture_ref = info.bound_texture_2d;
        break;
      case GL_TEXTURE_CUBE_MAP:
        texture_ref = info.bound_texture_cube_map;
        break;
      case GL_TEXTURE_EXTERNAL_OES:
        texture_ref = info.bound_texture_external_oes;
        break;
      case GL_TEXTURE_RECTANGLE_ARB:
        texture_ref = info.bound_texture_rectangle_arb;
        break;
      default:
        NOTREACHED();
        break;
    }
    if (texture_ref.get()) {
      last_id = texture_ref->service_id();
    } else {
      last_id = 0;
    }

    glBindTexture(target_, last_id);
    glActiveTexture(GL_TEXTURE0 + state_->active_texture_unit);
  }

 private:
  GLenum target_;
  const gpu::gles2::ContextState* state_;

  DISALLOW_COPY_AND_ASSIGN(ScopedTextureBinder);
};

// static
EGLBoolean EGL::DestroyImageKHR(EGLDisplay dpy, EGLImageKHR image) {
  return eglDestroyImageKHR(dpy, image);
}

// static
scoped_refptr<base::SingleThreadTaskRunner> GpuUtils::GetTaskRunner() {
  return content::GpuChildThread::GetTaskRunner();
}

// static
bool GpuUtils::IsSyncPointRetired(uint32_t sync_point) {
  return content::GpuChildThread::GetChannelManager()
      ->sync_point_manager()
      ->IsSyncPointRetired(sync_point);
}

// static
void GpuUtils::AddSyncPointCallback(uint32_t sync_point,
                                    const base::Closure& callback) {
  content::GpuChildThread::GetChannelManager()
      ->sync_point_manager()
      ->AddSyncPointCallback(sync_point, callback);
}

// static
EGLDisplay GpuUtils::GetHardwareEGLDisplay() {
  return gfx::GLSurfaceEGL::GetHardwareDisplay();
}

// static
GLuint GpuUtils::GetTextureFromMailbox(const CommandBufferID& command_buffer,
                                       const gpu::Mailbox& mailbox) {
  gpu::gles2::GLES2Decoder* decoder =
      content::oxide_gpu_shim::GetGLES2Decoder(
        command_buffer.client_id, command_buffer.route_id);
  if (!decoder) {
    return 0;
  }

  gpu::gles2::Texture* texture =
      decoder->GetContextGroup()->mailbox_manager()->ConsumeTexture(mailbox);
  if (!texture) {
    return 0;
  }

  return texture->service_id();
}

// static
EGLImageKHR GpuUtils::CreateEGLImageFromMailbox(
    const CommandBufferID& command_buffer,
    const gpu::Mailbox& mailbox) {
  gpu::gles2::GLES2Decoder* decoder =
      content::oxide_gpu_shim::GetGLES2Decoder(
        command_buffer.client_id, command_buffer.route_id);
  if (!decoder) {
    return EGL_NO_IMAGE_KHR;
  }

  gpu::gles2::Texture* texture =
      decoder->GetContextGroup()->mailbox_manager()->ConsumeTexture(mailbox);
  if (!texture) {
    return EGL_NO_IMAGE_KHR;
  }

  if (!decoder->MakeCurrent()) {
    return EGL_NO_IMAGE_KHR;
  }

  ScopedTextureBinder binder(GL_TEXTURE_2D, texture->service_id(),
                             decoder->GetContextState());

  EGLint attrib_list[] = {
      EGL_GL_TEXTURE_LEVEL_KHR, 0,
      EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
      EGL_NONE
  };
  EGLImageKHR egl_image = eglCreateImageKHR(
      gfx::GLSurfaceEGL::GetHardwareDisplay(),
      gfx::GLContext::GetCurrent()->GetHandle(),
      EGL_GL_TEXTURE_2D_KHR,
      reinterpret_cast<EGLClientBuffer>(texture->service_id()),
      attrib_list);
  if (egl_image == EGL_NO_IMAGE_KHR) {
    EGLint error = eglGetError();
    LOG(ERROR) << "Error creating EGLImage: " << error;
    return EGL_NO_IMAGE_KHR;
  }

  // This is required at least on Arale, but is it needed everywhere?
  glFinish();

  return egl_image;
}

} // namespace oxide
