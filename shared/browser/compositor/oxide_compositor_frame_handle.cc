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

#include "base/bind.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"

#include "oxide_compositor_frame_collector.h"
#include "oxide_compositor_frame_data.h"

namespace oxide {

namespace {

void ReclaimResourcesForFrameThunk(
    base::WeakPtr<CompositorFrameCollector> collector,
    uint32_t surface_id,
    std::unique_ptr<CompositorFrameData> frame) {
  if (!collector) {
    return;
  }

  collector->ReclaimResourcesForFrame(surface_id, frame.get());
}

}

CompositorFrameHandle::CompositorFrameHandle(
    uint32_t surface_id,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    base::WeakPtr<CompositorFrameCollector> collector,
    std::unique_ptr<CompositorFrameData> data)
    : surface_id_(surface_id),
      task_runner_(task_runner),
      collector_(collector),
      data_(std::move(data)) {}

CompositorFrameHandle::~CompositorFrameHandle() {
  if (!data_) {
    return;
  }

  if (task_runner_->BelongsToCurrentThread()) {
    if (collector_) {
      collector_->ReclaimResourcesForFrame(surface_id_, data_.get());
    }
  } else {
    // CompositorFrameHandle may be deleted on another thread where the
    // application UI is composited on a dedicated render thread
    task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&ReclaimResourcesForFrameThunk,
                   collector_, surface_id_, base::Passed(&data_)));
  }
}

} // namespace oxide
