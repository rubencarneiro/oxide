// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "oxide_compositor_frame_collector.h"

#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_frame_handle.h"

namespace oxide {

// static
CompositorFrameCollector* CompositorFrameCollector::FromFrameHandle(
    CompositorFrameHandle* handle) {
  return handle->collector_.get();
}

// static
scoped_ptr<CompositorFrameData> CompositorFrameCollector::TakeFrameData(
    CompositorFrameHandle* handle) {
  return std::move(handle->data_);
}

// static
uint32_t CompositorFrameCollector::FrameHandleSurfaceId(
    CompositorFrameHandle* handle) {
  return handle->surface_id_;
}

CompositorFrameCollector::~CompositorFrameCollector() {}

} // namespace oxide
