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

#include "oxide_compositor_frame_handle.h"

#include <utility>

#include "oxide_compositor_frame_data.h"
#include "oxide_compositor_proxy.h"

namespace oxide {

CompositorFrameHandle::CompositorFrameHandle(
    scoped_refptr<CompositorProxy> proxy,
    scoped_ptr<CompositorFrameData> data)
    : proxy_(proxy),
      data_(std::move(data)) {}

CompositorFrameHandle::~CompositorFrameHandle() {
  if (data_) {
    proxy_->ReclaimResourcesForFrame(data_.get());
  }
}

} // namespace oxide
