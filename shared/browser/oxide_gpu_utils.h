// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_GPU_UTILS_H_
#define _OXIDE_SHARED_BROWSER_GPU_UTILS_H_

#include <queue>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

typedef unsigned int GLuint;

namespace content {
class WebGraphicsContext3DCommandBufferImpl;
}

namespace gpu {
namespace gles2 {
class TextureRef;
}
}

namespace oxide {

class AcceleratedFrameHandle;
class RenderWidgetHostView;

class GpuUtils FINAL : public base::RefCountedThreadSafe<GpuUtils> {
 public:
  static void Initialize();
  void Destroy();

  static scoped_refptr<GpuUtils> instance();

  gfx::GLSurfaceHandle GetSharedSurfaceHandle();

  scoped_refptr<AcceleratedFrameHandle> GetAcceleratedFrameHandle(
      RenderWidgetHostView* rwhv,
      uint32 surface_id,
      const gpu::Mailbox& mailbox,
      uint32 sync_point,
      const gfx::Size& size,
      float scale);

 private:
  typedef content::WebGraphicsContext3DCommandBufferImpl WGC3DCBI;
  friend class base::RefCountedThreadSafe<GpuUtils>;

  GpuUtils();
  ~GpuUtils();

  bool FetchTextureResources(AcceleratedFrameHandle* handle);
  void FetchTextureResourcesOnGpuThread();

  scoped_ptr<WGC3DCBI> offscreen_context_;

  base::Lock fetch_texture_resources_lock_;
  bool is_fetch_texture_resources_pending_;
  std::queue<scoped_refptr<AcceleratedFrameHandle> > fetch_texture_resources_queue_;
};

class AcceleratedFrameHandle :
    public base::RefCountedThreadSafe<AcceleratedFrameHandle> {
 public:

  GLuint GetTextureID();
  gfx::Size size_in_pixels() const { return size_in_pixels_; }
  float device_scale_factor() const { return device_scale_factor_; }
  gpu::Mailbox mailbox() const { return mailbox_; }

  void WasFreed();

 private:
  friend class base::RefCountedThreadSafe<AcceleratedFrameHandle>;
  friend class GpuUtils;

  AcceleratedFrameHandle(int32 client_id,
                         int32 route_id,
                         RenderWidgetHostView* rwhv,
                         uint32 surface_id,
                         const gpu::Mailbox& mailbox,
                         uint32 sync_point,
                         const gfx::Size& size,
                         float scale);
  virtual ~AcceleratedFrameHandle();

  void UpdateTextureResourcesOnGpuThread();
  void OnSyncPointRetired();

  void FreeTextureRef();

  int32 client_id_;
  int32 route_id_;

  base::WeakPtr<RenderWidgetHostView> rwhv_;
  uint32 surface_id_;
  gpu::Mailbox mailbox_;
  uint32 sync_point_;
  gfx::Size size_in_pixels_;
  float device_scale_factor_;

  base::Lock lock_;
  base::ConditionVariable resources_available_condition_;
  bool did_fetch_texture_resources_;

  gpu::gles2::TextureRef* ref_;
  GLuint service_id_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GPU_UTILS_H_
