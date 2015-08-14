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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_

#include <deque>
#include <queue>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/linked_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "cc/output/software_output_device.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace cc {
class SharedBitmap;
}

namespace oxide {

class CompositorFrameData;

class CompositorSoftwareOutputDevice : public cc::SoftwareOutputDevice,
                                       public base::NonThreadSafe {
 public:
  CompositorSoftwareOutputDevice();
  ~CompositorSoftwareOutputDevice();

  void PopulateFrameDataForSwap(CompositorFrameData* data);
  void ReclaimResources(unsigned id);

 private:
  // cc::SoftwareOutputDevice implementation
  void Resize(const gfx::Size& pixel_size, float scale_factor) override;
  SkCanvas* BeginPaint(const gfx::Rect& damage_rect) override;
  void EndPaint(cc::SoftwareFrameData* software_frame_data) override;
  void DiscardBackbuffer() override;
  void EnsureBackbuffer() override;

  // =========
  unsigned GetNextId();

  struct OutputFrameData {
    OutputFrameData() : id(0) {}

    unsigned id;
    linked_ptr<cc::SharedBitmap> bitmap;
    gfx::Size size;
  };

  unsigned next_frame_id_;

  OutputFrameData backing_frame_;
  OutputFrameData previous_frame_;
  std::deque<OutputFrameData> pending_frames_;
  std::queue<OutputFrameData> returned_frames_;

  struct DamageData {
    DamageData() : id(0) {}
    DamageData(unsigned id, const gfx::Rect& damage)
        : id(id), damage(damage) {}

    unsigned id;
    gfx::Rect damage;
  };

  // We keep track of damage rects, so that when we recycle an old buffer
  // we can calculate the outdated region to copy from the previous buffer
  std::deque<DamageData> previous_damage_rects_;

  bool in_paint_;
  bool is_backbuffer_discarded_;
 
  DISALLOW_COPY_AND_ASSIGN(CompositorSoftwareOutputDevice);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SOFTWARE_OUTPUT_DEVICE_H_
