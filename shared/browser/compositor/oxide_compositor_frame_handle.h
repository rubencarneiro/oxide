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

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class Compositor;
class CompositorFrameData;
class CompositorProxy;
class CompositorSingleThreadProxy;

class OXIDE_SHARED_EXPORT CompositorFrameHandle
    : public base::RefCountedThreadSafe<CompositorFrameHandle> {
 public:
  CompositorFrameHandle(scoped_refptr<CompositorProxy> proxy,
                        scoped_ptr<CompositorFrameData> data);

  CompositorFrameData* data() const { return data_.get(); }

 private:
  friend class Compositor;
  friend class CompositorSingleThreadProxy;
  friend class base::RefCountedThreadSafe<CompositorFrameHandle>;

  ~CompositorFrameHandle();

  scoped_refptr<CompositorProxy> proxy_;
  scoped_ptr<CompositorFrameData> data_;

  DISALLOW_COPY_AND_ASSIGN(CompositorFrameHandle);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_FRAME_HANDLE_H_
