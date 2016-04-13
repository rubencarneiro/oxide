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

#include "gpu_service_shim_oxide.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/thread_task_runner_handle.h"
#include "content/gpu/gpu_child_thread.h"
#include "gpu/command_buffer/service/gles2_cmd_decoder.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "gpu/ipc/service/gpu_channel_manager.h"
#include "gpu/ipc/service/gpu_command_buffer_stub.h"

namespace gpu {
namespace oxide_shim {

namespace {

base::LazyInstance<base::Lock> g_gl_share_group_lock =
    LAZY_INSTANCE_INITIALIZER;
bool g_gl_share_group_used = false;
gfx::GLShareGroup* g_gl_share_group;

gpu::GpuCommandBufferStub* LookupCommandBuffer(int32_t client_id,
                                               int32_t route_id) {
  gpu::GpuChannelManager* gpu_channel_manager =
      content::GpuChildThread::GetChannelManager();
  DCHECK(gpu_channel_manager);
  gpu::GpuChannel* channel =
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

gpu::gles2::GLES2Decoder* GetGLES2Decoder(
    gpu::CommandBufferId command_buffer_id) {
  DCHECK(IsCurrentlyOnGpuThread());

  int32_t client_id = static_cast<int32_t>(command_buffer_id.GetUnsafeValue() >> 32);
  int32_t route_id = static_cast<int32_t>(command_buffer_id.GetUnsafeValue() & 0x00000000FFFFFFFF);

  gpu::GpuCommandBufferStub* command_buffer =
      LookupCommandBuffer(client_id, route_id);
  if (!command_buffer) {
    return nullptr;
  }

  return command_buffer->decoder();
}

gfx::GLShareGroup* GetGLShareGroup() {
  base::AutoLock lock(g_gl_share_group_lock.Get());
  g_gl_share_group_used = true;
  return g_gl_share_group;
}

void SetGLShareGroup(gfx::GLShareGroup* share_group) {
  base::AutoLock lock(g_gl_share_group_lock.Get());
  CHECK(!g_gl_share_group_used || !share_group);
  g_gl_share_group = share_group;
}

} // namespace oxide_shim
} // namespace gpu
