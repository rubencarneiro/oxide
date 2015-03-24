// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_MAILBOX_BUFFER_MAP_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_MAILBOX_BUFFER_MAP_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include <map>

#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "gpu/command_buffer/common/mailbox.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"

namespace oxide {

class CompositorThreadProxy;

class MailboxBufferMap {
 public:
  MailboxBufferMap(CompositingMode mode,
                   CompositorThreadProxy* proxy);
  ~MailboxBufferMap();

  void SetOutputSurfaceID(uint32_t surface_id);

  bool AddTextureMapping(uint32_t surface_id,
                         const gpu::Mailbox& mailbox,
                         GLuint texture);
  bool AddEGLImageMapping(uint32_t surface_id,
                          const gpu::Mailbox& mailbox,
                          EGLImageKHR egl_image);

  void MailboxBufferDestroyed(const gpu::Mailbox& mailbox);

  GLuint ConsumeTextureFromMailbox(const gpu::Mailbox& mailbox);
  EGLImageKHR ConsumeEGLImageFromMailbox(const gpu::Mailbox& mailbox);

  void ReclaimMailboxBufferResources(const gpu::Mailbox& mailbox);

 private:
  void UnrefMailboxBufferDataForEGLImage(const gpu::Mailbox& mailbox);

  CompositingMode mode_;
  CompositorThreadProxy* proxy_;

  base::Lock lock_;

  uint32_t surface_id_;

  union MailboxBufferData {
    GLuint texture;
    struct {
      int ref_count;
      EGLImageKHR image;
    } egl_image;
  };

  std::map<gpu::Mailbox, MailboxBufferData> map_;

  DISALLOW_COPY_AND_ASSIGN(MailboxBufferMap);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_MAILBOX_BUFFER_MAP_H_
