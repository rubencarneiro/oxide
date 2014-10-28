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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

typedef unsigned int GLuint;

namespace oxide {

class CompositorThreadProxy;

class GLFrameData {
 public:
  GLFrameData(const gpu::Mailbox& mailbox,
              GLuint texture_id);
  virtual ~GLFrameData();

  const gpu::Mailbox& mailbox() const { return mailbox_; }
  GLuint texture_id() const { return texture_id_; }

 private:
  gpu::Mailbox mailbox_;
  GLuint texture_id_;
};

class SoftwareFrameData {
 public:
  SoftwareFrameData(unsigned id,
                    const gfx::Rect& damage_rect,
                    uint8* pixels);
  ~SoftwareFrameData();

  unsigned id() const { return id_; }
  const gfx::Rect& damage_rect() const { return damage_rect_; }
  uint8* pixels() const { return pixels_; }

 private:
  unsigned id_;
  gfx::Rect damage_rect_;
  uint8* pixels_;
};

class CompositorFrameHandle final :
    public base::RefCounted<CompositorFrameHandle> {
 public:
  CompositorFrameHandle(uint32 surface_id,
                        scoped_refptr<CompositorThreadProxy> proxy,
                        const gfx::Size& size,
                        float scale);

  const gfx::Size& size_in_pixels() const { return size_in_pixels_; }
  float device_scale() const { return device_scale_; }

  GLFrameData* gl_frame_data() { return gl_frame_data_.get(); }
  SoftwareFrameData* software_frame_data() { return software_frame_data_.get(); }

 private:
  friend class CompositorThreadProxy;
  friend class base::RefCounted<CompositorFrameHandle>;

  ~CompositorFrameHandle();

  uint32 surface_id_;
  scoped_refptr<CompositorThreadProxy> proxy_;

  gfx::Size size_in_pixels_;
  float device_scale_;

  scoped_ptr<GLFrameData> gl_frame_data_;
  scoped_ptr<SoftwareFrameData> software_frame_data_;

  DISALLOW_COPY_AND_ASSIGN(CompositorFrameHandle);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_
