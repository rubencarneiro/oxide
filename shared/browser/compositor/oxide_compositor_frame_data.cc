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

#include "oxide_compositor_frame_data.h"

namespace oxide {

GLFrameData::GLFrameData()
    : type(Type::INVALID) {
  resource.egl_image = nullptr;
}

GLFrameData::~GLFrameData() {}

SoftwareFrameData::SoftwareFrameData()
    : id(0),
      pixels(nullptr) {}

SoftwareFrameData::~SoftwareFrameData() {}

CompositorFrameData::CompositorFrameData()
    : surface_id(0),
      device_scale(0.f) {}

CompositorFrameData::~CompositorFrameData() {}

CompositorFrameData::CompositorFrameData(CompositorFrameData&& other)
    : CompositorFrameData() {
  std::swap(surface_id, other.surface_id);
  std::swap(size_in_pixels, other.size_in_pixels);
  std::swap(device_scale, other.device_scale);
  std::swap(gl_frame_data, other.gl_frame_data);
  std::swap(software_frame_data, other.software_frame_data);
}

// static
scoped_ptr<CompositorFrameData> CompositorFrameData::AllocFrom(
    CompositorFrameData* other) {
  return make_scoped_ptr(new CompositorFrameData(std::move(*other)));
}

} // namespace oxide
