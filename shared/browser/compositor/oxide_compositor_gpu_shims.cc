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

#include <memory>

#include "base/lazy_instance.h"
#include "base/single_thread_task_runner.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/context_state.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/sync_point_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "gpu/ipc/service/gpu_channel_manager.h"
#include "gpu/ipc/service/gpu_command_buffer_stub.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface_egl.h"

namespace oxide {

namespace {

gpu::gles2::GLES2Decoder* GetGLES2Decoder(
    gpu::CommandBufferId command_buffer_id) {
  int32_t client_id =
      static_cast<int32_t>(command_buffer_id.GetUnsafeValue() >> 32);
  int32_t route_id =
      static_cast<int32_t>(command_buffer_id.GetUnsafeValue() &
                           0x00000000FFFFFFFF);

  gpu::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::GetChannelManager();
  DCHECK(gpu_channel_manager);

  gpu::GpuChannel* channel = gpu_channel_manager->LookupChannel(client_id);
  if (!channel) {
    return nullptr;
  }

  gpu::GpuCommandBufferStub* command_buffer =
      channel->LookupCommandBuffer(route_id);
  if (!command_buffer) {
    return nullptr;
  }

  CHECK_EQ(command_buffer->command_buffer_id(), command_buffer_id);

  return command_buffer->decoder();
}

}

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
bool GpuUtils::IsSyncPointRetired(const gpu::SyncToken& sync_token) {
  return content::GpuChildThread::GetChannelManager()
      ->sync_point_manager()->IsSyncTokenReleased(sync_token);
}

// static
bool GpuUtils::WaitForSyncPoint(const gpu::SyncToken& sync_token,
                                const base::Closure& callback) {
  return content::GpuChildThread::GetChannelManager()
      ->sync_point_manager()->WaitOutOfOrder(sync_token, callback);
}

// static
bool GpuUtils::CanUseEGLImage() {
  return gl::g_driver_egl.ext.b_EGL_KHR_gl_texture_2D_image &&
         gl::g_driver_egl.ext.b_EGL_KHR_image &&
         gl::g_driver_egl.ext.b_EGL_KHR_image_base;
}

// static
EGLDisplay GpuUtils::GetHardwareEGLDisplay() {
  return gl::GLSurfaceEGL::GetHardwareDisplay();
}

// static
GLuint GpuUtils::GetTextureFromMailbox(gpu::CommandBufferId command_buffer,
                                       const gpu::Mailbox& mailbox) {
  gpu::gles2::GLES2Decoder* decoder = GetGLES2Decoder(command_buffer);
  if (!decoder) {
    return 0;
  }

  gpu::gles2::TextureBase* texture =
      decoder->GetContextGroup()->mailbox_manager()->ConsumeTexture(mailbox);
  if (!texture) {
    return 0;
  }

  return texture->service_id();
}

// static
EGLImageKHR GpuUtils::CreateEGLImageFromMailbox(
    gpu::CommandBufferId command_buffer,
    const gpu::Mailbox& mailbox) {
  gpu::gles2::GLES2Decoder* decoder = GetGLES2Decoder(command_buffer);
  if (!decoder) {
    return EGL_NO_IMAGE_KHR;
  }

  gpu::gles2::TextureBase* texture =
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
      gl::GLSurfaceEGL::GetHardwareDisplay(),
      gl::GLContext::GetCurrent()->GetHandle(),
      EGL_GL_TEXTURE_2D_KHR,
      reinterpret_cast<EGLClientBuffer>(texture->service_id()),
      attrib_list);
  if (egl_image == EGL_NO_IMAGE_KHR) {
    LOG(ERROR) << "Error creating EGLImage: " << ui::GetLastEGLErrorString();
    return EGL_NO_IMAGE_KHR;
  }

  // This is required at least on Arale, but is it needed everywhere?
  glFinish();

  return egl_image;
}

} // namespace oxide
