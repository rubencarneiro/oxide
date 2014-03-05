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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

template <typename Type> struct DefaultSingletonTraits;

typedef unsigned int GLuint;

namespace content {
class WebGraphicsContext3DCommandBufferImpl;
}

namespace gpu {
class Mailbox;
}

namespace oxide {

class TextureHandle;

class GpuUtils FINAL {
 public:
  static void Initialize();
  static void Terminate();

  static GpuUtils* instance();

  gfx::GLSurfaceHandle GetSharedSurfaceHandle();

  TextureHandle* CreateTextureHandle();

 private:
  typedef content::WebGraphicsContext3DCommandBufferImpl WGC3DCBI;

  GpuUtils();

  scoped_ptr<WGC3DCBI> offscreen_context_;
};

class TextureHandle : public base::RefCountedThreadSafe<TextureHandle> {
 public:
  virtual void Consume(const gpu::Mailbox& mailbox,
                       const gfx::Size& size) = 0;

  virtual gfx::Size GetSize() const = 0;
  virtual GLuint GetID() = 0;

 protected:
  friend class base::RefCountedThreadSafe<TextureHandle>;

  TextureHandle() {}
  virtual ~TextureHandle() {}
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_GPU_UTILS_H_
