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

#ifndef _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITING_MODE_H_
#define _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITING_MODE_H_

namespace oxide {

enum CompositingMode {
  // Compositor output will be bitmap
  COMPOSITING_MODE_SOFTWARE,

  // Compositor output will be a GL texture, shared between the compositor
  // GL context and the application GL context
  COMPOSITING_MODE_TEXTURE,

  // Compositor output will be an EGLImage, backed by a texture belonging
  // to the compositor GL context in the same process
  COMPOSITING_MODE_IMAGE

  // TODO: Extra modes for separate GPU process
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COMPOSITOR_COMPOSITING_MODE_H_
