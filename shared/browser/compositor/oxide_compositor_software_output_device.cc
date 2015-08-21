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

#include "oxide_compositor_software_output_device.h"

#include "base/logging.h"
#include "cc/resources/shared_bitmap.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "ui/gfx/skia_util.h"

#include "oxide_compositor_frame_data.h"

namespace oxide {

void CompositorSoftwareOutputDevice::Resize(const gfx::Size& pixel_size,
                                            float scale_factor) {
  DCHECK(CalledOnValidThread());
  if (pixel_size == viewport_pixel_size_ && scale_factor == scale_factor_) {
    return;
  }

  viewport_pixel_size_ = pixel_size;
  scale_factor_ = scale_factor;

  DiscardBackbuffer();
  EnsureBackbuffer();
}

SkCanvas* CompositorSoftwareOutputDevice::BeginPaint(
    const gfx::Rect& damage_rect) {
  DCHECK(CalledOnValidThread());
  DCHECK(!in_paint_);
  DCHECK(!damage_rect.IsEmpty());

  in_paint_ = true;

  EnsureBackbuffer();
  DCHECK_NE(backing_frame_.id, 0U);

  // Create a surface
  SkImageInfo info = SkImageInfo::MakeN32Premul(viewport_pixel_size_.width(),
                                                viewport_pixel_size_.height());
  surface_ = skia::AdoptRef(SkSurface::NewRasterDirect(
      info,
      backing_frame_.bitmap->pixels(),
      info.minRowBytes()));

  DCHECK(previous_frame_.id != 0 ||
         damage_rect == gfx::Rect(viewport_pixel_size_)) <<
      "Expected the damage rect to be the whole viewport for the first frame";

  // See if this buffer has been used in the past
  while (!previous_damage_rects_.empty()) {
    unsigned id = previous_damage_rects_.front().id;
    previous_damage_rects_.pop_front();
    if (id == backing_frame_.id) {
      break;
    }
  }

  SkRegion outdated_region;

  if (previous_damage_rects_.empty() &&
      previous_frame_.id != 0 &&
      previous_frame_.id != backing_frame_.id) {
    // If this is a new buffer, then we need to copy everything from the
    // previous buffer. Note, this can also happen if we receive buffers
    // back from the embedder in the wrong order, as it will get popped
    // off previous_damage_rects_.
    outdated_region.setRect(
        gfx::RectToSkIRect(gfx::Rect(viewport_pixel_size_)));
  } else {
    // Add together the damage rects since this buffer was last used
    for (std::deque<DamageData>::iterator it = previous_damage_rects_.begin();
         it != previous_damage_rects_.end(); ++it) {
      outdated_region.op(gfx::RectToSkIRect(it->damage), SkRegion::kUnion_Op);
    }
  }

  // Don't copy pixels inside the new damage rect, as those will be painted
  // anyway
  outdated_region.op(gfx::RectToSkIRect(damage_rect),
                     SkRegion::kDifference_Op);

  // Now copy pixels in the outdated region from the previous buffer to the
  // new buffer. This brings everything outside of the damage_rect in the new
  // buffer up-to-date
  if (!outdated_region.isEmpty()) {
    DCHECK(previous_frame_.id != 0 && previous_frame_.id != backing_frame_.id);
    DCHECK(backing_frame_.size == previous_frame_.size);

    SkImageInfo info =
        SkImageInfo::MakeN32Premul(viewport_pixel_size_.width(),
                                   viewport_pixel_size_.height());
    SkBitmap back_bitmap;
    back_bitmap.installPixels(info, previous_frame_.bitmap->pixels(),
                              info.minRowBytes());

    for (SkRegion::Iterator it(outdated_region); !it.done(); it.next()) {
      SkRect rect = SkRect::Make(it.rect());
      surface_->getCanvas()->drawBitmapRect(back_bitmap, rect, rect, nullptr);
    }
  }

  damage_rect_ = damage_rect;

  return surface_->getCanvas();
}

void CompositorSoftwareOutputDevice::EndPaint() {
  DCHECK(CalledOnValidThread());
  DCHECK(in_paint_);
  DCHECK_NE(backing_frame_.id, 0U);
  DCHECK(backing_frame_.size == viewport_pixel_size_);

  in_paint_ = false;

  previous_frame_ = backing_frame_;
  pending_frames_.push_back(backing_frame_);
  previous_damage_rects_.push_back(DamageData(backing_frame_.id, damage_rect_));

  backing_frame_ = OutputFrameData();
  damage_rect_ = gfx::Rect();
}

void CompositorSoftwareOutputDevice::DiscardBackbuffer() {
  DCHECK(CalledOnValidThread());
  if (is_backbuffer_discarded_) {
    return;
  }

  is_backbuffer_discarded_ = true;

  backing_frame_ = OutputFrameData();
  previous_frame_ = OutputFrameData();

  while (!returned_frames_.empty()) {
    returned_frames_.pop();
  }

  previous_damage_rects_.clear();
}

void CompositorSoftwareOutputDevice::EnsureBackbuffer() {
  DCHECK(CalledOnValidThread());
  is_backbuffer_discarded_ = false;

  if (backing_frame_.id == 0 && !returned_frames_.empty()) {
    backing_frame_ = returned_frames_.front();
    returned_frames_.pop();
    DCHECK(backing_frame_.size == viewport_pixel_size_);
    DCHECK(backing_frame_.bitmap.get());
  }

  if (backing_frame_.id == 0) {
    backing_frame_.id = GetNextId();
    scoped_ptr<cc::SharedBitmap> shared_bitmap =
        content::HostSharedBitmapManager::current()->AllocateSharedBitmap(
          viewport_pixel_size_);
    DCHECK(shared_bitmap);
    backing_frame_.bitmap =
        linked_ptr<cc::SharedBitmap>(shared_bitmap.release());
    backing_frame_.size = viewport_pixel_size_;
  }
}

unsigned CompositorSoftwareOutputDevice::GetNextId() {
  unsigned id = next_frame_id_++;
  if (id == 0) {
    id = next_frame_id_++;
  }

  return id;
}

CompositorSoftwareOutputDevice::CompositorSoftwareOutputDevice()
    : next_frame_id_(1),
      in_paint_(false),
      is_backbuffer_discarded_(false) {
  DetachFromThread();
}

CompositorSoftwareOutputDevice::~CompositorSoftwareOutputDevice() {}

void CompositorSoftwareOutputDevice::PopulateFrameDataForSwap(
    CompositorFrameData* data) {
  DCHECK(CalledOnValidThread());
  DCHECK(!in_paint_);
  DCHECK_NE(previous_frame_.id, 0U);
  DCHECK_EQ(previous_frame_.id, previous_damage_rects_.back().id);

  data->size_in_pixels = previous_frame_.size;
  data->software_frame_data->id = previous_frame_.id;
  data->software_frame_data->damage_rect =
      previous_damage_rects_.back().damage;
  data->software_frame_data->bitmap_id = previous_frame_.bitmap->id();
}

void CompositorSoftwareOutputDevice::ReclaimResources(unsigned id) {
  DCHECK(CalledOnValidThread());
  if (id == 0) {
    return;
  }

  std::deque<OutputFrameData>::iterator it;
  for (it = pending_frames_.begin(); it != pending_frames_.end(); ++it) {
    if (it->id == id) {
      break;
    }
  }

  CHECK(it != pending_frames_.end());

  if (!is_backbuffer_discarded_ && it->size == viewport_pixel_size_) {
    returned_frames_.push(*it);
  } else if (it->id == previous_frame_.id) {
    previous_frame_ = OutputFrameData();
  }

  pending_frames_.erase(it);
}

} // namespace oxide
