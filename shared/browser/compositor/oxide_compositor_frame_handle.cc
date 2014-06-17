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

#include "oxide_compositor_frame_handle.h"

#include "oxide_compositor_thread_proxy.h"

namespace oxide {

GLFrameData::GLFrameData(const gpu::Mailbox& mailbox,
                         const gfx::Size& size,
                         float scale,
                         GLuint texture_id)
    : mailbox_(mailbox),
      size_in_pixels_(size),
      device_scale_(scale),
      texture_id_(texture_id) {}

GLFrameData::~GLFrameData() {}

CompositorFrameHandle::CompositorFrameHandle()
    : surface_id(0) {}

CompositorFrameHandle::~CompositorFrameHandle() {
  if (proxy) {
    proxy->ReclaimResourcesForFrame(this);
  }
}

} // namespace oxide
