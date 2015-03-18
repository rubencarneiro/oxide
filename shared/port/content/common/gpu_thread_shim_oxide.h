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
class GLShareGroup;
}

namespace gpu {
class Mailbox;
}

namespace content {

class ContextProviderCommandBuffer;

namespace oxide_gpu_shim {

// Return the texture name for the specified client process, command buffer
// and mailbox
CONTENT_EXPORT GLuint GetTextureFromMailbox(
    int32_t client_id,
    int32_t route_id,
    const gpu::Mailbox& mailbox);

// Create and return an EGLImage for the specified texture
CONTENT_EXPORT EGLImageKHR CreateEGLImageFromTexture(
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
