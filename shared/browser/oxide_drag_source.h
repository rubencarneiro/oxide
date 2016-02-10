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

#ifndef _OXIDE_SHARED_BROWSER_DRAG_SOURCE_H_
#define _OXIDE_SHARED_BROWSER_DRAG_SOURCE_H_

#include "third_party/WebKit/public/web/WebDragOperation.h"

class SkBitmap;

namespace content {
class DropData;
class WebContents;
}

namespace gfx {
class Vector2d;
}

namespace oxide {

class DragSourceClient;

class DragSource {
 public:
  virtual ~DragSource() {}

  virtual void StartDragging(content::WebContents* contents,
                             const content::DropData& drop_data,
                             blink::WebDragOperationsMask allowed_ops,
                             const SkBitmap& bitmap,
                             const gfx::Vector2d& image_offset_pix) = 0;

 protected:
  DragSource(DragSourceClient* client) : client_(client) {}

  DragSourceClient* client() const { return client_; }

 private:
  DragSourceClient* client_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_DRAG_SOURCE_H_
