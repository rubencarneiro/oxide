// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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
#include <queue>

#include "base/macros.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "ui/gfx/geometry/size.h"

#include "shared/browser/compositor/oxide_compositing_mode.h"

namespace oxide {

class CompositorFrameData;

// This class maintains a map of mailbox names to actual in-process GPU buffers
class MailboxBufferMap {
 public:
  MailboxBufferMap(CompositingMode mode);
  ~MailboxBufferMap();

  // Tells us to drop all resource mappings
  void DropAllResources();

  // Add a mapping from |mailbox| to |texture|
  void AddTextureMapping(const gpu::Mailbox& mailbox, GLuint texture);

  // Add a mapping from |mailbox| to |egl_image|. MailboxBufferMap takes
  // ownership of |egl_image|
  void AddEGLImageMapping(const gpu::Mailbox& mailbox,
                          EGLImageKHR egl_image);

  // Notification that the GPU buffer for |mailbox| was destroyed by the
  // compositor
  void MailboxBufferDestroyed(const gpu::Mailbox& mailbox);

  // Returns the real texture for |mailbox| if a mapping exists
  GLuint ConsumeTextureFromMailbox(const gpu::Mailbox& mailbox);

  // Returns an EGLImageKHR for |mailbox| if a mapping exists. This also
  // increases the reference count by 1. You should call
  // ReclaimMailboxBufferResources when done
  EGLImageKHR ConsumeEGLImageFromMailbox(const gpu::Mailbox& mailbox);

  // Noop for COMPOSITING_MODE_TEXTURE, but decreases the reference count
  // for the resource identified by |mailbox| for COMPOSITING_MODE_EGLIMAGE
  void ReclaimMailboxBufferResources(const gpu::Mailbox& mailbox);

 private:
  struct MailboxBufferData;

  void AddMapping(const gpu::Mailbox& mailbox, const MailboxBufferData& data);

  CompositingMode mode_;

  struct MailboxBufferData {
    union {
      GLuint texture;
      struct {
        bool live;
        int ref_count;
        EGLImageKHR egl_image;
      } image;
    } data;
  };

  std::map<gpu::Mailbox, MailboxBufferData> map_;

  DISALLOW_COPY_AND_ASSIGN(MailboxBufferMap);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_MAILBOX_BUFFER_MAP_H_
