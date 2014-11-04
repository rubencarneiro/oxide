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
#include "content/common/gpu/sync_point_manager.h"
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
  DCHECK(content::GpuChildThread::instance());
  content::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::instance()->gpu_channel_manager();
  content::GpuChannel* channel =
      gpu_channel_manager->LookupChannel(client_id);
  if (!channel) {
    return NULL;
  }

  return channel->LookupCommandBuffer(route_id);
}

}

bool IsCurrentlyOnGpuThread() {
  return GetGpuThreadTaskRunner()->BelongsToCurrentThread();
}

scoped_refptr<base::SingleThreadTaskRunner> GetGpuThreadTaskRunner() {
  return content::GpuChildThread::message_loop_proxy();
}

void AddGpuThreadTaskObserver(base::MessageLoop::TaskObserver* obs) {
  DCHECK(IsCurrentlyOnGpuThread());
  content::GpuChildThread::instance()->message_loop()->AddTaskObserver(obs);
}

gpu::gles2::TextureRef* CreateTextureRef(unsigned target,
                                         int32_t client_id,
                                         int32_t route_id,
                                         const gpu::Mailbox& mailbox) {
  DCHECK(IsCurrentlyOnGpuThread());

  content::GpuCommandBufferStub* command_buffer =
      LookupCommandBuffer(client_id, route_id);
  if (!command_buffer) {
    return NULL;
  }

  gpu::gles2::ContextGroup* group =
      command_buffer->decoder()->GetContextGroup();
  gpu::gles2::Texture* texture =
      group->mailbox_manager()->ConsumeTexture(GL_TEXTURE_2D, mailbox);
  if (!texture) {
    return NULL;
  }

  gpu::gles2::TextureRef* ref =
      new gpu::gles2::TextureRef(group->texture_manager(),
                                 client_id,
                                 texture);
  ref->AddRef();

  return ref;
}

void ReleaseTextureRef(int32_t client_id,
                       int32_t route_id,
                       gpu::gles2::TextureRef* ref) {
  DCHECK(IsCurrentlyOnGpuThread());

  if (content::GpuCommandBufferStub* command_buffer =
          LookupCommandBuffer(client_id, route_id)) {
    command_buffer->decoder()->MakeCurrent();
  }

  ref->Release();
}

bool IsSyncPointRetired(uint32_t sync_point) {
  DCHECK(IsCurrentlyOnGpuThread());

  return content::GpuChildThread::instance()->gpu_channel_manager()
      ->sync_point_manager()
      ->IsSyncPointRetired(sync_point);
}

void AddSyncPointCallback(uint32_t sync_point, const base::Closure& callback) {
  DCHECK(IsCurrentlyOnGpuThread());

  content::GpuChildThread::instance()->gpu_channel_manager()
      ->sync_point_manager()
      ->AddSyncPointCallback(sync_point, callback);
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
