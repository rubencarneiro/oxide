// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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
#include "base/memory/ref_counted_memory.h"
#include "cc/resources/shared_bitmap.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "ui/gfx/skia_util.h"

#include "oxide_compositor_frame_data.h"

namespace oxide {

class RefCountedPixelMemory : public base::RefCountedMemory {
 public:
  static scoped_refptr<RefCountedPixelMemory> Create(const gfx::Size& size);

  const unsigned char* front() const override;
  size_t size() const override;

 private:
  RefCountedPixelMemory(size_t size);
  ~RefCountedPixelMemory();

  size_t size_;
  scoped_ptr<unsigned char[]> pixels_;
};

RefCountedPixelMemory::RefCountedPixelMemory(size_t size)
    : size_(size),
      pixels_(new unsigned char[size]) {}

RefCountedPixelMemory::~RefCountedPixelMemory() {}

// static
scoped_refptr<RefCountedPixelMemory>
RefCountedPixelMemory::Create(const gfx::Size& size) {
  if (size.IsEmpty()) {
    return nullptr;
  }

  base::CheckedNumeric<size_t> s = 4;
  s *= size.width();
  s *= size.height();
  if (!s.IsValid()) {
    return nullptr;
  }

  return make_scoped_refptr(new RefCountedPixelMemory(s.ValueOrDie()));
}

const unsigned char* RefCountedPixelMemory::front() const {
  return pixels_.get();
}

size_t RefCountedPixelMemory::size() const {
  return size_;
}

void CompositorSoftwareOutputDevice::Resize(const gfx::Size& pixel_size,
                                            float scale_factor) {
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
  DCHECK(!surface_);
  DCHECK(!damage_rect.IsEmpty());
  DCHECK(last_painted_buffer_id_ > 0 ||
         damage_rect == gfx::Rect(viewport_pixel_size_));

  EnsureBackbuffer();

  // Create a surface
  SkImageInfo info = SkImageInfo::MakeN32Premul(viewport_pixel_size_.width(),
                                                viewport_pixel_size_.height());
  surface_ = skia::AdoptRef(SkSurface::NewRasterDirect(
      info,
      const_cast<unsigned char*>(back_buffer_->pixels->front()),
      info.minRowBytes()));

  // Get the outdated region for this buffer so we can update it from the
  // last painted buffer
  SkRegion outdated_region = back_buffer_->outdated_region;
  back_buffer_->outdated_region = SkRegion();

  // Don't copy pixels inside the new damage rect, as those will be painted
  // anyway
  outdated_region.op(gfx::RectToSkIRect(damage_rect),
                     SkRegion::kDifference_Op);

  // Now copy pixels in the outdated region from the previous buffer to the
  // new buffer. This brings everything outside of the damage_rect in the new
  // buffer up-to-date
  if (!outdated_region.isEmpty()) {
    DCHECK_NE(last_painted_buffer_id_, 0U);
    DCHECK_NE(last_painted_buffer_id_, back_buffer_->id);

    const BufferData* last_painted_buffer = GetLastPaintedBuffer();

    SkImageInfo info =
        SkImageInfo::MakeN32Premul(viewport_pixel_size_.width(),
                                   viewport_pixel_size_.height());
    SkBitmap back_bitmap;
    back_bitmap.installPixels(
        info,
        const_cast<unsigned char*>(last_painted_buffer->pixels->front()),
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
  DCHECK(surface_);
  DCHECK(back_buffer_);

  last_painted_buffer_id_ = back_buffer_->id;

  // Add the damage rect to other buffers
  for (auto& buffer : buffers_) {
    if (buffer.id == back_buffer_->id || buffer.id == 0) {
      continue;
    }
    buffer.outdated_region.op(gfx::RectToSkIRect(damage_rect_),
                              SkRegion::kUnion_Op);                              
  }

  back_buffer_ = nullptr;
  surface_ = nullptr;
}

void CompositorSoftwareOutputDevice::DiscardBackbuffer() {
  if (is_backbuffer_discarded_) {
    return;
  }

  is_backbuffer_discarded_ = true;

  back_buffer_ = nullptr;
  last_painted_buffer_id_ = 0;

  for (auto& buffer : buffers_) {
    DiscardBuffer(&buffer);
  }
}

void CompositorSoftwareOutputDevice::EnsureBackbuffer() {
  is_backbuffer_discarded_ = false;

  if (!back_buffer_) {
    auto it = std::find_if(buffers_.begin(),
                           buffers_.end(),
                           [](const BufferData& buffer) {
      return buffer.id > 0 && buffer.available;
    });

    if (it != buffers_.end()) {
      back_buffer_ = &(*it);
      back_buffer_->available = false;
    }
  }

  if (!back_buffer_) {
    auto it = std::find_if(buffers_.begin(),
                           buffers_.end(),
                           [](const BufferData& buffer) {
      return buffer.id == 0;
    });
    DCHECK(it != buffers_.end());

    DCHECK(!it->pixels);
    it->id = GetNextId();
    it->available = false;
    it->pixels = RefCountedPixelMemory::Create(viewport_pixel_size_);
    it->size = viewport_pixel_size_;
    it->outdated_region.setRect(gfx::RectToSkIRect(gfx::Rect(viewport_pixel_size_)));

    back_buffer_ = &(*it);
  }

  DCHECK_NE(back_buffer_->id, 0U);
  DCHECK(back_buffer_->size == viewport_pixel_size_);
  DCHECK(back_buffer_->pixels);
  DCHECK(!back_buffer_->available);
}

CompositorSoftwareOutputDevice::BufferData*
CompositorSoftwareOutputDevice::GetBufferById(unsigned id) {
  if (id == 0) {
    return nullptr;
  }

  auto it = std::find_if(buffers_.begin(),
                         buffers_.end(),
                         [id](const BufferData& buffer) {
    return id == buffer.id;
  });

  if (it == buffers_.end()) {
    return nullptr;
  }

  return &(*it);
}

CompositorSoftwareOutputDevice::BufferData*
CompositorSoftwareOutputDevice::GetLastPaintedBuffer() {
  return GetBufferById(last_painted_buffer_id_);
}

unsigned CompositorSoftwareOutputDevice::GetNextId() {
  unsigned id = next_buffer_id_++;
  if (id == 0) {
    id = next_buffer_id_++;
  }

  return id;
}

void CompositorSoftwareOutputDevice::DiscardBuffer(BufferData* buffer) {
  if (buffer->id == 0) {
    return;
  }

  buffer->id = 0;
  buffer->available = true;
  buffer->pixels = nullptr;
  buffer->size = gfx::Size();
  buffer->outdated_region = SkRegion();
}

CompositorSoftwareOutputDevice::CompositorSoftwareOutputDevice()
    : next_buffer_id_(1),
      last_painted_buffer_id_(0),
      back_buffer_(nullptr),
      is_backbuffer_discarded_(true) {}

CompositorSoftwareOutputDevice::~CompositorSoftwareOutputDevice() {}

void CompositorSoftwareOutputDevice::PopulateFrameDataForSwap(
    CompositorFrameData* data) {
  DCHECK(!surface_);
  DCHECK(!back_buffer_);
  DCHECK_NE(last_painted_buffer_id_, 0U);

  const BufferData* buffer = GetLastPaintedBuffer();

  data->size_in_pixels = buffer->size;
  data->software_frame_data->id = buffer->id;
  data->software_frame_data->damage_rect = damage_rect_;
  data->software_frame_data->pixels = buffer->pixels;

  damage_rect_ = gfx::Rect();
}

void CompositorSoftwareOutputDevice::ReclaimResources(unsigned id) {
  if (id == 0) {
    return;
  }

  BufferData* buffer = GetBufferById(id);
  if (!buffer) {
    return;
  }

  DCHECK(!buffer->available);
  buffer->available = true;

  if (is_backbuffer_discarded_ || buffer->size != viewport_pixel_size_) {
    DiscardBuffer(buffer);
  }
}

} // namespace oxide
