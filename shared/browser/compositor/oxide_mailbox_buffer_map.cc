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
#include "cc/output/compositor_frame.h"
#include "cc/output/compositor_frame_metadata.h"
#include "cc/output/gl_frame_data.h"

#include "oxide_compositor_gpu_shims.h"

namespace oxide {

MailboxBufferMap::DelayedFrameSwap::DelayedFrameSwap(
    uint32_t surface_id,
    cc::CompositorFrame* frame)
    : surface_id(surface_id),
      size(frame->gl_frame_data->size),
      scale(frame->metadata.device_scale_factor),
      mailbox(frame->gl_frame_data->mailbox) {}

void MailboxBufferMap::AddMapping(const gpu::Mailbox& mailbox,
                                  const MailboxBufferData& data,
                                  DelayedFrameSwapQueue* ready_frame_swaps) {
  lock_.AssertAcquired();

  map_[mailbox] = data;

  while (!delayed_frame_swaps_.empty()) {
    const DelayedFrameSwap& swap = delayed_frame_swaps_.front();
    if (map_.find(swap.mailbox) == map_.end() &&
        swap.surface_id == surface_id_) {
      break;
    }

    ready_frame_swaps->push(swap);
    delayed_frame_swaps_.pop();
  }
}

MailboxBufferMap::MailboxBufferMap(CompositingMode mode)
    : mode_(mode),
      surface_id_(0) {}

MailboxBufferMap::~MailboxBufferMap() {}

void MailboxBufferMap::SetOutputSurfaceID(uint32_t surface_id) {
  base::AutoLock lock(lock_);

  surface_id_ = surface_id;

  for (auto it = map_.begin(); it != map_.end(); ) {
    if (it->second.surface_id == surface_id) {
      ++it;
      continue;
    }

    if (mode_ == COMPOSITING_MODE_TEXTURE) {
      auto e = it++;
      map_.erase(e);
    } else {
      DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
      it->second.data.image.live = false;
      if (it->second.data.image.ref_count == 0) {
        GpuUtils::GetTaskRunner()->PostTask(
            FROM_HERE,
            base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                       GpuUtils::GetHardwareEGLDisplay(),
                       it->second.data.image.egl_image));
        auto e = it++;
        map_.erase(e);
      } else {
        ++it;
      }
    }
  }

  while (!delayed_frame_swaps_.empty()) {
    DCHECK_NE(delayed_frame_swaps_.front().surface_id, surface_id);
    delayed_frame_swaps_.pop();
  }
}

bool MailboxBufferMap::AddTextureMapping(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    GLuint texture,
    DelayedFrameSwapQueue* ready_frame_swaps) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_TEXTURE);
  DCHECK_NE(texture, 0U);

  base::AutoLock lock(lock_);

  if (surface_id != surface_id_) {
    return false;
  }

  DCHECK(map_.find(mailbox) == map_.end());

  MailboxBufferData data;
  data.surface_id = surface_id;
  data.data.texture = texture;

  AddMapping(mailbox, data, ready_frame_swaps);

  return true;
}

bool MailboxBufferMap::AddEGLImageMapping(
    uint32_t surface_id,
    const gpu::Mailbox& mailbox,
    EGLImageKHR egl_image,
    DelayedFrameSwapQueue* ready_frame_swaps) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
  DCHECK_NE(egl_image, EGL_NO_IMAGE_KHR);

  base::AutoLock lock(lock_);

  if (surface_id != surface_id_) {
    return false;
  }

  DCHECK(map_.find(mailbox) == map_.end());

  MailboxBufferData data;
  data.surface_id = surface_id;
  data.data.image.live = true;
  data.data.image.ref_count = 0;
  data.data.image.egl_image = egl_image;

  AddMapping(mailbox, data, ready_frame_swaps);

  return true;
}

void MailboxBufferMap::MailboxBufferDestroyed(const gpu::Mailbox& mailbox) {
  base::AutoLock lock(lock_);

  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    map_.erase(mailbox);
  } else {
    DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);
    auto it = map_.find(mailbox);
    if (it == map_.end()) {
      return;
    }
    it->second.data.image.live = false;
    if (it->second.data.image.ref_count == 0) {
      GpuUtils::GetTaskRunner()->PostTask(
          FROM_HERE,
          base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                     GpuUtils::GetHardwareEGLDisplay(),
                     it->second.data.image.egl_image));
      map_.erase(it);
    }
  }
}

GLuint MailboxBufferMap::ConsumeTextureFromMailbox(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_TEXTURE);

  base::AutoLock lock(lock_);
  auto it = map_.find(mailbox);
  if (it == map_.end()) {
    return 0;
  }

  return it->second.data.texture;
}

EGLImageKHR MailboxBufferMap::ConsumeEGLImageFromMailbox(
    const gpu::Mailbox& mailbox) {
  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);

  base::AutoLock lock(lock_);
  auto it = map_.find(mailbox);
  if (it == map_.end() || !it->second.data.image.live) {
    return EGL_NO_IMAGE_KHR;
  }

  ++it->second.data.image.ref_count;
  return it->second.data.image.egl_image;
}

void MailboxBufferMap::ReclaimMailboxBufferResources(
    const gpu::Mailbox& mailbox) {
  if (mode_ == COMPOSITING_MODE_TEXTURE) {
    return;
  }

  DCHECK_EQ(mode_, COMPOSITING_MODE_EGLIMAGE);

  base::AutoLock lock(lock_);

  auto it = map_.find(mailbox);
  DCHECK(it != map_.end());
  DCHECK_GT(it->second.data.image.ref_count, 0);

  if (--it->second.data.image.ref_count == 0 &&
      !it->second.data.image.live) {
    GpuUtils::GetTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(base::IgnoreResult(&EGL::DestroyImageKHR),
                   GpuUtils::GetHardwareEGLDisplay(),
                   it->second.data.image.egl_image));
    map_.erase(it);
  }
}

bool MailboxBufferMap::CanBeginFrameSwap(uint32_t surface_id,
                                         cc::CompositorFrame* frame) {
  DCHECK(frame->gl_frame_data.get());

  base::AutoLock lock(lock_);

  if (!delayed_frame_swaps_.empty()) {
    DelayedFrameSwap swap(surface_id, frame);
    delayed_frame_swaps_.push(swap);
    return false;
  }

  if (surface_id != surface_id_) {
    return true;
  }

  if (map_.find(frame->gl_frame_data->mailbox) == map_.end()) {
    DelayedFrameSwap swap(surface_id, frame);
    delayed_frame_swaps_.push(swap);
    return false;
  }

  return true;
}

} // namespace oxide
