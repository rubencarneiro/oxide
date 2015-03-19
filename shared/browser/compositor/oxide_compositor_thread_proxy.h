// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread_checker.h"
#include "cc/resources/shared_bitmap.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"

namespace base {
class MessageLoopProxy;
}

namespace cc {
class CompositorFrame;
class CompositorFrameAck;
}

namespace gfx {
class Rect;
class Size;
}

namespace oxide {

class Compositor;
class CompositorFrameHandle;
class CompositorOutputSurface;
class GLFrameData;
class ImageFrameData;
class SoftwareFrameData;

class CompositorThreadProxy final
    : public base::RefCountedThreadSafe<CompositorThreadProxy> {
 public:
  typedef std::vector<scoped_refptr<CompositorFrameHandle> > FrameHandleVector;

  CompositorThreadProxy(Compositor* compositor);

  void CompositorDestroyed();
  void SetOutputSurface(CompositorOutputSurface* output);

  void SwapCompositorFrame(cc::CompositorFrame* frame);
  void DidSwapCompositorFrame(
      uint32 surface_id,
      FrameHandleVector* returned_frames);
  void ReclaimResourcesForFrame(CompositorFrameHandle* frame);

 private:
  friend class base::RefCountedThreadSafe<CompositorThreadProxy>;

  ~CompositorThreadProxy();

  void DidSkipSwapCompositorFrame(
      uint32 surface_id,
      scoped_refptr<CompositorFrameHandle>* frame);

  void SendSwapTextureFrameOnOwnerThread(uint32_t surface_id,
                                         const gfx::Size& size,
                                         float scale,
                                         const gpu::Mailbox& mailbox,
                                         GLuint texture);
  void SendSwapEGLImageFrameOnOwnerThread(uint32_t surface_id,
                                          const gfx::Size& size,
                                          float scale,
                                          const gpu::Mailbox& mailbox,
                                          EGLImageKHR egl_image);
  void SendSwapSoftwareFrameOnOwnerThread(uint32_t surface_id,
                                          const gfx::Size& size,
                                          float scale,
                                          unsigned id,
                                          const gfx::Rect& damage_rect,
                                          const cc::SharedBitmapId& bitmap_id);
  void SendDidSwapBuffersToOutputSurfaceOnImplThread(
      uint32 surface_id,
      FrameHandleVector returned_frames);
  void SendReclaimResourcesToOutputSurfaceOnImplThread(
      uint32 surface_id,
      const gfx::Size& size_in_pixels,
      scoped_ptr<GLFrameData> gl_frame_data,
      scoped_ptr<SoftwareFrameData> software_frame_data,
      scoped_ptr<ImageFrameData> image_frame_data);

  struct OwnerData {
    OwnerData() : compositor(nullptr) {}

    Compositor* compositor;
  };

  struct ImplData {
    ImplData() : output(nullptr) {}

    CompositorOutputSurface* output;
  };

  OwnerData& owner();
  ImplData& impl();

  CompositingMode mode_;

  scoped_refptr<base::MessageLoopProxy> owner_message_loop_;
  scoped_refptr<base::MessageLoopProxy> impl_message_loop_;

  base::ThreadChecker owner_thread_checker_;
  base::ThreadChecker impl_thread_checker_;

  OwnerData owner_unsafe_access_;
  ImplData impl_unsafe_access_;

  DISALLOW_COPY_AND_ASSIGN(CompositorThreadProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITOR_THREAD_PROXY_H_
