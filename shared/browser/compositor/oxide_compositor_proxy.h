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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_PROXY_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_PROXY_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace gpu {
class Mailbox;
}

namespace oxide {

class CompositorFrameData;
class CompositorFrameHandle;
class CompositorOutputSurface;

class CompositorProxyClient {
 public:
  virtual ~CompositorProxyClient() {}

  virtual void SwapCompositorFrameFromProxy(
      scoped_ptr<CompositorFrameData> frame) = 0;
};

class CompositorProxy : public base::RefCounted<CompositorProxy> {
 public:
  virtual ~CompositorProxy() {}

  // Notification that the owning Compositor instance has been destroyed
  void ClientDestroyed() { client_ = nullptr; }

  // Set the current output surface
  virtual void SetOutputSurface(CompositorOutputSurface* output_surface) = 0;

  // Notification from the compositor that a buffer was created with the
  // specified mailbox name
  virtual void MailboxBufferCreated(const gpu::Mailbox& mailbox,
                                    uint64_t sync_point) = 0;

  // Notification from the compositor that the buffer with the specified
  // mailbox name was destroyed
  virtual void MailboxBufferDestroyed(const gpu::Mailbox& mailbox) = 0;

  // Called from the OutputSurface to tell the client to swap
  virtual void SwapCompositorFrame(scoped_ptr<CompositorFrameData> frame) = 0;

  // Called from the client to tell the compositor that a frame swap
  // completed. |returned_frames| contains the buffers that the client
  // no longer needs
  using FrameHandleVector = std::vector<scoped_refptr<CompositorFrameHandle>>;
  virtual void DidSwapCompositorFrame(uint32_t surface_id,
                                      FrameHandleVector returned_frames) = 0;

  // Called when CompositorFrameHandle is deleted, so that associated
  // resources can be reclaimed
  virtual void ReclaimResourcesForFrame(CompositorFrameData* frame) = 0;

 protected:
  CompositorProxy(CompositorProxyClient* client) : client_(client) {}

  CompositorProxyClient* client() const { return client_; }

 private:
  CompositorProxyClient* client_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_PROXY_H_
