// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_THREAD_PROXY_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_THREAD_PROXY_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include <map>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/threading/thread_checker.h"
#include "cc/resources/shared_bitmap.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"
#include "shared/browser/compositor/oxide_mailbox_buffer_map.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace cc {
class CompositorFrameAck;
}

namespace gfx {
class Rect;
class Size;
}

namespace oxide {

class Compositor;
class CompositorFrameData;
class CompositorFrameHandle;
class CompositorOutputSurface;
class GLFrameData;
class SoftwareFrameData;

// This class bridges Compositor (which lives on the owner thread) and its
// client, and CompositorOutputSurface (which lives on the impl thread)
class CompositorThreadProxy 
    : public base::RefCountedThreadSafe<CompositorThreadProxy> {
 public:
  typedef std::vector<scoped_refptr<CompositorFrameHandle> > FrameHandleVector;

  CompositorThreadProxy(Compositor* compositor);

  // Notification that the owning Compositor instance has been destroyed,
  // called on the owner thread
  void CompositorDestroyed();

  // Set the current output surface, and called on the impl thread
  void SetOutputSurface(CompositorOutputSurface* output_surface);

  // Notification from the compositor that a buffer was created with the
  // specified mailbox name, called on the impl thread
  void MailboxBufferCreated(const gpu::Mailbox& mailbox, uint64_t sync_point);

  // Notification from the compositor that the buffer with the specified
  // mailbox name was destroyed, called on the impl thread
  void MailboxBufferDestroyed(const gpu::Mailbox& mailbox);

  // Called from the compositor to tell the client to swap, called on
  // the impl thread
  void SwapCompositorFrame(CompositorFrameData* frame);

  // Called from the client to tell the compositor that a frame swap
  // completed. |returned_frames| contains the buffers that the client
  // no longer needs. This function is called on the owner thread or
  // another thread whilst the owner thread is frozen
  void DidSwapCompositorFrame(uint32_t surface_id,
                              FrameHandleVector returned_frames);

  // Called when CompositorFrameHandle is deleted, so that associated
  // resources can be reclaimed. Called on the owner thread or another
  // thread whilst the owner thread is frozen
  void ReclaimResourcesForFrame(CompositorFrameData* frame);

 private:
  friend class base::RefCountedThreadSafe<CompositorThreadProxy>;

  ~CompositorThreadProxy();

  // Response from CompositorUtils::GetTextureFromMailbox
  void GetTextureFromMailboxResponseOnOwnerThread(uint32_t surface_id,
                                                  const gpu::Mailbox& mailbox,
                                                  GLuint texture);

  // Response from CompositorUtils::CreateEGLImageFromMailbox
  void CreateEGLImageFromMailboxResponseOnOwnerThread(
      uint32_t surface_id,
      const gpu::Mailbox& mailbox,
      EGLImageKHR egl_image);

  // Owner thread handler for a new software frame
  void SendSwapSoftwareFrameOnOwnerThread(
      scoped_ptr<CompositorFrameData> frame);

  // Called when the fence for a new GL frame is passed on the GPU thread
  void DidCompleteGLFrameOnImplThread(scoped_ptr<CompositorFrameData> frame);

  // Owner thread handler for a new GL frame
  void SendSwapGLFrameOnOwnerThread(scoped_ptr<CompositorFrameData> frame);

  // Called when the client is not notified of a new frame, eg, if it has
  // disappeared. Note that |frame| will be modified.
  // This is called on the owner thread.
  void DidSkipSwapCompositorFrame(scoped_ptr<CompositorFrameData> frame);

  // Impl thread handler for DidSwapCompositorFrame
  void SendDidSwapBuffersToOutputSurfaceOnImplThread(
      uint32_t surface_id,
      FrameHandleVector returned_frames);

  // Impl thread handler for ReclaimResourcesForFrame
  void SendReclaimResourcesToOutputSurfaceOnImplThread(
      scoped_ptr<CompositorFrameData> frame);

  struct OwnerData {
    OwnerData() : compositor(nullptr) {}

    Compositor* compositor;
  };

  struct ImplData {
    ImplData() : output_surface(nullptr) {}

    CompositorOutputSurface* output_surface;
  };

  OwnerData& owner();
  ImplData& impl();

  const CompositingMode mode_;

  scoped_refptr<base::SingleThreadTaskRunner> owner_message_loop_;
  scoped_refptr<base::SingleThreadTaskRunner> impl_message_loop_;

  base::ThreadChecker owner_thread_checker_;
  base::ThreadChecker impl_thread_checker_;

  OwnerData owner_unsafe_access_;
  ImplData impl_unsafe_access_;

  MailboxBufferMap mailbox_buffer_map_;

  DISALLOW_COPY_AND_ASSIGN(CompositorThreadProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_THREAD_PROXY_H_
