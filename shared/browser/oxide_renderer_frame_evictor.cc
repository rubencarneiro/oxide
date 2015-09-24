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

#include "oxide_renderer_frame_evictor.h"

#include "base/logging.h"
#include "base/memory/shared_memory.h"
#include "base/memory/singleton.h"
#include "base/sys_info.h"
#include "content/common/host_shared_bitmap_manager.h"

#include "shared/common/oxide_form_factor.h"

#include "oxide_renderer_frame_evictor_client.h"

namespace oxide {

RendererFrameEvictorClient::~RendererFrameEvictorClient() {
  RendererFrameEvictor::GetInstance()->RemoveFrame(this);
}

RendererFrameEvictor::RendererFrameEvictor()
    : max_number_of_saved_frames_(1),
      max_number_of_handles_(base::SharedMemory::GetHandleLimit() / 8.0f) {
  if (GetFormFactorHint() == FORM_FACTOR_DESKTOP) {
    max_number_of_saved_frames_ =
        std::min(5, 2 + (base::SysInfo::AmountOfPhysicalMemoryMB() / 256));
  }
}

void RendererFrameEvictor::CullUnlockedFrames() {
  if (unlocked_frames_.size() == 0) {
    return;
  }

  content::HostSharedBitmapManager* bitmap_manager =
      content::HostSharedBitmapManager::current();

  while (!unlocked_frames_.empty() &&
         ((unlocked_frames_.size() + locked_frames_.size()) >
            max_number_of_saved_frames_ ||
          bitmap_manager->AllocatedBitmapCount() > max_number_of_handles_)) {
    unlocked_frames_.front()->EvictCurrentFrame();
    unlocked_frames_.pop_front();
  }
}

// static
RendererFrameEvictor* RendererFrameEvictor::GetInstance() {
  return base::Singleton<RendererFrameEvictor>::get();
}

RendererFrameEvictor::~RendererFrameEvictor() {}

void RendererFrameEvictor::AddFrame(RendererFrameEvictorClient* frame,
                                    bool locked) {
  RemoveFrame(frame);
  if (locked) {
    locked_frames_[frame] = 1;
  } else {
    unlocked_frames_.push_back(frame);
  }

  CullUnlockedFrames();
}

void RendererFrameEvictor::RemoveFrame(RendererFrameEvictorClient* frame) {
  std::map<RendererFrameEvictorClient *, size_t>::iterator it =
      locked_frames_.find(frame);
  if (it != locked_frames_.end()) {
    locked_frames_.erase(it);
  }
  unlocked_frames_.remove(frame);
}

void RendererFrameEvictor::LockFrame(RendererFrameEvictorClient* frame) {
  std::list<RendererFrameEvictorClient *>::iterator it =
      std::find(unlocked_frames_.begin(), unlocked_frames_.end(), frame);
  if (it != unlocked_frames_.end()) {
    DCHECK(locked_frames_.find(frame) == locked_frames_.end());
    unlocked_frames_.erase(it);
    locked_frames_[frame] = 1;
  } else {
    DCHECK(locked_frames_.find(frame) != locked_frames_.end());
    locked_frames_[frame]++;
  }
}

void RendererFrameEvictor::UnlockFrame(RendererFrameEvictorClient* frame) {
  DCHECK(locked_frames_.find(frame) != locked_frames_.end());
  size_t count = locked_frames_[frame];
  DCHECK(count > 0);
  if (count > 1) {
    locked_frames_[frame]--;
  } else {
    DCHECK(std::find(
        unlocked_frames_.begin(), unlocked_frames_.end(), frame) ==
        unlocked_frames_.end());
    std::map<RendererFrameEvictorClient *, size_t>::iterator it =
        locked_frames_.find(frame);
    locked_frames_.erase(it);
    unlocked_frames_.push_back(frame);

    CullUnlockedFrames();
  }
}

} // namespace oxide
