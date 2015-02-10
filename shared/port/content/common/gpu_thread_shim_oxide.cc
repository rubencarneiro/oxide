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

#include "gpu_thread_shim_oxide.h"

#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/single_thread_task_runner.h"
#include "content/common/gpu/client/command_buffer_proxy_impl.h"
#include "content/common/gpu/client/context_provider_command_buffer.h"
#include "content/common/gpu/gpu_channel.h"
#include "content/common/gpu/gpu_channel_manager.h"
#include "content/common/gpu/gpu_command_buffer_stub.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/context_group.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/command_buffer/service/mailbox_manager.h"
#include "gpu/command_buffer/service/texture_manager.h"

namespace content {
namespace oxide_gpu_shim {

namespace {

gfx::GLShareGroup* g_gl_share_group;

content::GpuCommandBufferStub* LookupCommandBuffer(int32_t client_id,
                                                   int32_t route_id) {
  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::GetChannelManager();
  DCHECK(gpu_channel_manager);
  content::GpuChannel* channel =
      gpu_channel_manager->LookupChannel(client_id);
  if (!channel) {
    return nullptr;
  }

  return channel->LookupCommandBuffer(route_id);
}

bool IsCurrentlyOnGpuThread() {
  return content::GpuChildThread::GetTaskRunner()->BelongsToCurrentThread();
}

}

TextureRefHolder::TextureRefHolder() {}

TextureRefHolder::TextureRefHolder(
    content::GpuCommandBufferStub* command_buffer,
    gpu::gles2::TextureRef* ref)
    : command_buffer_(command_buffer->AsWeakPtr()),
      ref_(ref) {
  DCHECK(IsCurrentlyOnGpuThread());
}

TextureRefHolder::~TextureRefHolder() {
  DCHECK(IsCurrentlyOnGpuThread());

  if (IsValid()) {
    DCHECK(command_buffer_);
    if (ref_->HasOneRef()) {
      command_buffer_->decoder()->MakeCurrent();
    }
  }
}

bool TextureRefHolder::IsValid() const {
  DCHECK(IsCurrentlyOnGpuThread());
  return !!ref_.get();
}

GLuint TextureRefHolder::GetServiceID() const {
  DCHECK(IsCurrentlyOnGpuThread());
  DCHECK(IsValid());
  return ref_->service_id();
}

TextureRefHolder CreateTextureRef(int32_t client_id,
                                  int32_t route_id,
                                  const gpu::Mailbox& mailbox) {
  DCHECK(IsCurrentlyOnGpuThread());

  content::GpuCommandBufferStub* command_buffer =
      LookupCommandBuffer(client_id, route_id);
  if (!command_buffer) {
    return TextureRefHolder();
  }

  gpu::gles2::ContextGroup* group =
      command_buffer->decoder()->GetContextGroup();
  gpu::gles2::Texture* texture =
      group->mailbox_manager()->ConsumeTexture(mailbox);
  if (!texture) {
    return TextureRefHolder();
  }

  scoped_refptr<gpu::gles2::TextureRef> ref =
      new gpu::gles2::TextureRef(group->texture_manager(),
                                 0, texture);
  return TextureRefHolder(command_buffer, ref.get());
}

int32_t GetContextProviderRouteID(
    content::ContextProviderCommandBuffer* provider) {
  return provider->GetCommandBufferProxy()->GetRouteID();
}

gfx::GLShareGroup* GetGLShareGroup() {
  return g_gl_share_group;
}

void SetGLShareGroup(gfx::GLShareGroup* share_group) {
  g_gl_share_group = share_group;
}

} // namespace oxide_gpu_shim
} // namespace content
