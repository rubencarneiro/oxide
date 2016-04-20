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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace oxide {

class CompositorFrameCollector;
class CompositorFrameData;

class OXIDE_SHARED_EXPORT CompositorFrameHandle
    : public base::RefCountedThreadSafe<CompositorFrameHandle> {
 public:
  CompositorFrameHandle(uint32_t surface_id,
                        scoped_refptr<base::SingleThreadTaskRunner> task_runner,
                        base::WeakPtr<CompositorFrameCollector> collector,
                        std::unique_ptr<CompositorFrameData> data);

  CompositorFrameData* data() const { return data_.get(); }

 private:
  friend class CompositorFrameCollector;
  friend class base::RefCountedThreadSafe<CompositorFrameHandle>;

  ~CompositorFrameHandle();

  uint32_t surface_id_;

  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::WeakPtr<CompositorFrameCollector> collector_;
  
  std::unique_ptr<CompositorFrameData> data_;

  DISALLOW_COPY_AND_ASSIGN(CompositorFrameHandle);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_
