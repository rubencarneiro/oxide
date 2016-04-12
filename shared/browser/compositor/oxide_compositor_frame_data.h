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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_DATA_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_DATA_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "cc/resources/shared_bitmap.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace base {
class RefCountedMemory;
}

namespace oxide {

class GLFrameData {
 public:
  GLFrameData();
  ~GLFrameData();

  gpu::Mailbox mailbox;

  enum Type : uint8_t {
    INVALID,
    TEXTURE,
    EGLIMAGE
  };
  Type type;

  union {
    GLuint texture;
    EGLImageKHR egl_image;
  } resource;
};

class SoftwareFrameData {
 public:
  SoftwareFrameData();
  ~SoftwareFrameData();

  unsigned id;
  gfx::Rect damage_rect;
  scoped_refptr<base::RefCountedMemory> pixels;
};

class CompositorFrameData {
 public:
  CompositorFrameData();
  ~CompositorFrameData();

  CompositorFrameData(CompositorFrameData&& other);

  static scoped_ptr<CompositorFrameData> AllocFrom(
      CompositorFrameData* other);

  gfx::Size size_in_pixels;
  float device_scale;

  scoped_ptr<GLFrameData> gl_frame_data;
  scoped_ptr<SoftwareFrameData> software_frame_data;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_DATA_H_
