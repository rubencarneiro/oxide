// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_CONTAINER_H_
#define _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_CONTAINER_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "content/public/common/menu_item.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace cc {
class Layer;
}

namespace gfx {
class Rect;
}

namespace ui {
class TouchHandleDrawable;
}

namespace oxide {

class Compositor;
class RenderWidgetHostView;

class RenderWidgetHostViewContainer {
 public:
  virtual ~RenderWidgetHostViewContainer() {}

  virtual Compositor* GetCompositor() const = 0;

  virtual void AttachLayer(scoped_refptr<cc::Layer> layer) = 0;

  virtual void DetachLayer(scoped_refptr<cc::Layer> layer) = 0;

  virtual void CursorChanged(RenderWidgetHostView* view) = 0;

  virtual gfx::Size GetViewSizeInPixels() const = 0;

  virtual gfx::Rect GetViewBounds() const = 0;

  virtual bool HasFocus() const = 0;

  virtual bool IsFullscreen() const = 0;

  virtual float GetTopControlsHeight() = 0;

  virtual ui::TouchHandleDrawable* CreateTouchHandleDrawable() const = 0;

  virtual void TouchSelectionChanged(RenderWidgetHostView* view,
                                     bool handle_drag_in_progress,
                                     bool insertion_handle_tapped) = 0;

  virtual void EditingCapabilitiesChanged(RenderWidgetHostView* view) = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_CONTAINER_H_
