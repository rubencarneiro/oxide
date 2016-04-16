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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_LISTENER_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_LISTENER_H_

#include "base/memory/scoped_ptr.h"

namespace gpu {
class Mailbox;
}

namespace oxide {

class CompositorFrameData;
class CompositorOutputSurface;

class CompositorOutputSurfaceListener {
 public:
  virtual ~CompositorOutputSurfaceListener() {}

  // Notification that |output_surface| was bound
  virtual void OutputSurfaceBound(CompositorOutputSurface* output_surface) = 0;

  // Notification that |output_surface| was destroyed
  virtual void OutputSurfaceDestroyed(
      CompositorOutputSurface* output_surface) = 0;

  // Notification that a buffer was created with the specified mailbox name
  virtual void MailboxBufferCreated(const gpu::Mailbox& mailbox,
                                    uint64_t sync_point) = 0;

  // Notification that the buffer with the specified mailbox name was destroyed
  virtual void MailboxBufferDestroyed(const gpu::Mailbox& mailbox) = 0;

  // Notification that the client should swap
  virtual void SwapCompositorFrame(scoped_ptr<CompositorFrameData> frame) = 0;

  // Notification that the client has returned all frames
  virtual void AllFramesReturnedFromClient() = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_OUTPUT_SURFACE_LISTENER_H_
