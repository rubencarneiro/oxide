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

#include "oxide_mailbox_buffer_map.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"

#include "oxide_compositor_gpu_shims.h"

namespace oxide {

void MailboxBufferMap::UnrefMailboxBufferDataForEGLImage(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);

  base::AutoLock lock(lock_);
  auto it = map_.find(mailbox);
  DCHECK(it != map_.end());

  DCHECK_GT(it->second.egl_image.ref_count, 0);
  if (--it->second.egl_image.ref_count == 0) {
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                   GpuUtils::GetHardwareEGLDisplay(),
                   it->second.egl_image.image));
    map_.erase(it);
  }
}

MailboxBufferMap::MailboxBufferMap(CompositingMode mode,
                                   CompositorThreadProxy* proxy)
    : mode_(mode),
      proxy_(proxy),
      surface_id_(0) {}

MailboxBufferMap::~MailboxBufferMap() {}

void MailboxBufferMap::SetOutputSurfaceID(uint32_t surface_id) {
  base::AutoLock lock(lock_);
  surface_id_ = surface_id;
}

bool MailboxBufferMap::AddTextureMapping(uint32_t surface_id,
                                         const gpu::Mailbox& mailbox,
                                         GLuint texture) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_TEXTURE);
  DCHECK_NE(texture, 0U);

  base::AutoLock lock(lock_);
  if (surface_id != surface_id_) {
    return false;
  }

  DCHECK(map_.find(mailbox) == map_.end());

  MailboxBufferData data;
  data.texture = texture;

  map_[mailbox] = data;

  return true;
}

bool MailboxBufferMap::AddEGLImageMapping(uint32_t surface_id,
                                          const gpu::Mailbox& mailbox,
                                          EGLImageKHR egl_image) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
  DCHECK_NE(egl_image, EGL_NO_IMAGE_KHR);

  base::AutoLock lock(lock_);
  if (surface_id != surface_id_) {
    return false;
  }

  DCHECK(map_.find(mailbox) == map_.end());

  MailboxBufferData data;
  data.egl_image.ref_count = 1;
  data.egl_image.image = egl_image;

  map_[mailbox] = data;

  return true;
}

void MailboxBufferMap::MailboxBufferDestroyed(const gpu::Mailbox& mailbox) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    base::AutoLock lock(lock_);
    size_t rv = map_.erase(mailbox);
    DCHECK_GT(rv, 0U);
    return;
  }

  UnrefMailboxBufferDataForEGLImage(mailbox);
}

GLuint MailboxBufferMap::ConsumeTextureFromMailbox(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_TEXTURE);

  base::AutoLock lock(lock_);
  auto it = map_.find(mailbox);
  if (it == map_.end()) {
    return 0;
  }

  return it->second.texture;
}

EGLImageKHR MailboxBufferMap::ConsumeEGLImageFromMailbox(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);

  base::AutoLock lock(lock_);
  auto it = map_.find(mailbox);
  if (it == map_.end()) {
    return EGL_NO_IMAGE_KHR;
  }

  ++it->second.egl_image.ref_count;
  return it->second.egl_image.image;
}

void MailboxBufferMap::ReclaimMailboxBufferResources(
    const gpu::Mailbox& mailbox) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    return;
  }

  UnrefMailboxBufferDataForEGLImage(mailbox);
}

} // namespace oxide
