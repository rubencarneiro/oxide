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

#ifndef _OXIDE_SHARED_PORT_CONTENT_COMMON_GPU_THREAD_SHIM_H_
#define _OXIDE_SHARED_PORT_CONTENT_COMMON_GPU_THREAD_SHIM_H_

#include <cstdint>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/common/content_export.h"

typedef unsigned int GLuint;
typedef void* EGLImageKHR;

namespace gfx {
class GLImageEGL;
class GLShareGroup;
}

namespace gpu {
class Mailbox;
namespace gles2 {
class TextureManager;
class TextureRef;
}
}

namespace content {

class ContextProviderCommandBuffer;
class GpuCommandBufferStub;

namespace oxide_gpu_shim {

// Wrapper class for TextureRef. This is not thread-safe, and should only
// be used on the GPU thread
class CONTENT_EXPORT Texture {
 public:
  Texture(content::GpuCommandBufferStub* command_buffer,
          gpu::gles2::TextureRef* ref);
  ~Texture();

  // Return the real texture ID
  GLuint GetServiceID() const;

  // Returns the TextureManager associated with the share group that the
  // TextureRef belongs to
  gpu::gles2::TextureManager* GetTextureManager() const;

  // Destroy the underlying TextureRef. This can fail, eg, if the context
  // is lost. On success, |this| can be deleted. On failure, |this| must not
  // be deleted until its TextureManager is deing destroyed
  // Returns: true on success, false on failure
  bool Destroy();  

 private:
  // We use a WeakPtr, but it's a bug if this class outlives command_buffer_
  base::WeakPtr<content::GpuCommandBufferStub> command_buffer_;

  scoped_refptr<gpu::gles2::TextureRef> ref_;

  DISALLOW_COPY_AND_ASSIGN(Texture);
};

// Create and return a Texture instance for the corresponding client process,
// command buffer and mailbox. The caller takes ownership of Texture
CONTENT_EXPORT Texture* ConsumeTextureFromMailbox(
    int32_t client_id,
    int32_t route_id,
    const gpu::Mailbox& mailbox);

// Create and return an EGLImage for the specified texture
CONTENT_EXPORT EGLImageKHR CreateImageFromTexture(
    int32_t client_id,
    int32_t route_id,
    GLuint texture);

CONTENT_EXPORT int32_t GetContextProviderRouteID(
    content::ContextProviderCommandBuffer* provider);

gfx::GLShareGroup* GetGLShareGroup();
CONTENT_EXPORT void SetGLShareGroup(gfx::GLShareGroup* share_group);

} // oxide_gpu_shim
} // content

#endif // _OXIDE_SHARED_PORT_CONTENT_COMMON_GPU_THREAD_SHIM_H_
