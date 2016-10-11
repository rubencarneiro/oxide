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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_

#include <deque>
#include <queue>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "cc/output/software_output_device.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace base {
class RefCountedMemory;
}

namespace oxide {

class CompositorFrameData;

class CompositorSoftwareOutputDevice : public cc::SoftwareOutputDevice {
 public:
  CompositorSoftwareOutputDevice();
  ~CompositorSoftwareOutputDevice();

  void PopulateFrameDataForSwap(CompositorFrameData* data);
  void ReclaimResources(unsigned id);

 private:
  struct BufferData {
    BufferData();
    ~BufferData();

    unsigned id;
    bool available;
    scoped_refptr<base::RefCountedMemory> pixels;
    gfx::Size size;
    SkRegion outdated_region;
  };

  // cc::SoftwareOutputDevice implementation
  void Resize(const gfx::Size& pixel_size, float scale_factor) override;
  SkCanvas* BeginPaint(const gfx::Rect& damage_rect) override;
  void EndPaint() override;
  void DiscardBackbuffer() override;
  void EnsureBackbuffer() override;

  // =========
  BufferData* GetBufferById(unsigned id);
  BufferData* GetLastPaintedBuffer();
  unsigned GetNextId();
  void DiscardBuffer(BufferData* buffer);

  float device_scale_factor_;

  unsigned next_buffer_id_;
  unsigned last_painted_buffer_id_;

  BufferData* back_buffer_;
  std::array<BufferData, 2> buffers_;

  bool is_backbuffer_discarded_;
 
  DISALLOW_COPY_AND_ASSIGN(CompositorSoftwareOutputDevice);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_
