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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SINGLE_THREAD_PROXY_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SINGLE_THREAD_PROXY_H_

#include "base/macros.h"
#include "base/threading/non_thread_safe.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"
#include "shared/browser/compositor/oxide_compositor_proxy.h"
#include "shared/browser/compositor/oxide_mailbox_buffer_map.h"

namespace oxide {

class CompositorSingleThreadProxy : public CompositorProxy,
                                    public base::NonThreadSafe {
 public:
  CompositorSingleThreadProxy(CompositorProxyClient* client);
  ~CompositorSingleThreadProxy() override;

 private:
  // Response from CompositorUtils::GetTextureFromMailbox
  void GetTextureFromMailboxResponse(uint32_t surface_id,
                                     const gpu::Mailbox& mailbox,
                                     GLuint texture);

  // Response from CompositorUtils::CreateEGLImageFromMailbox
  void CreateEGLImageFromMailboxResponse(uint32_t surface_id,
                                         const gpu::Mailbox& mailbox,
                                         EGLImageKHR egl_image);

  void DidCompleteGLFrame(scoped_ptr<CompositorFrameData> frame);
  void ContinueSwapGLFrame(scoped_ptr<CompositorFrameData> frame);

  // CompositorProxy implementation
  void SetOutputSurface(CompositorOutputSurface* output_surface) override;
  void MailboxBufferCreated(const gpu::Mailbox& mailbox,
                            uint64_t sync_point) override;
  void MailboxBufferDestroyed(const gpu::Mailbox& mailbox) override;
  void SwapCompositorFrame(scoped_ptr<CompositorFrameData> frame) override;
  void DidSwapCompositorFrame(uint32_t surface_id,
                              FrameHandleVector returned_frames) override;
  void ReclaimResourcesForFrame(CompositorFrameData* frame) override;

  const CompositingMode mode_;

  CompositorOutputSurface* output_surface_;

  MailboxBufferMap mailbox_buffer_map_;

  int frames_waiting_for_completion_;
  int mailbox_resource_fetches_in_progress_;

  DISALLOW_COPY_AND_ASSIGN(CompositorSingleThreadProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_SINGLE_THREAD_PROXY_H_
