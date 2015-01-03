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

#ifndef _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_
#define _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_

#include "third_party/WebKit/public/platform/WebScreenInfo.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace content {
class WebCursor;
}

namespace gfx {
class Rect;
}

namespace oxide {

class Compositor;
class WebView;

class RenderWidgetHostViewDelegate {
 public:
  virtual ~RenderWidgetHostViewDelegate() {}

  virtual void EvictCurrentFrame() = 0;

  virtual void ProcessAckedTouchEvent(bool consumed) = 0;

  virtual void UpdateCursor(const content::WebCursor& cursor) = 0;

  virtual void TextInputStateChanged(ui::TextInputType type,
                                     bool show_ime_if_needed) = 0;

  virtual void FocusedNodeChanged(bool is_editable_node) = 0;

  virtual void ImeCancelComposition() = 0;

  virtual void SelectionBoundsChanged(const gfx::Rect& caret_rect,
                                      size_t selection_cursor_position,
                                      size_t selection_anchor_position) = 0;

  virtual void SelectionChanged() = 0;

  virtual Compositor* GetCompositor() const = 0;

  virtual gfx::Size GetViewSizePix() const = 0;

  virtual gfx::Rect GetViewBoundsDip() const = 0;

  virtual blink::WebScreenInfo GetScreenInfo() const = 0;

  virtual bool HasFocus() const = 0;

  virtual bool IsVisible() const = 0;

  virtual float GetLocationBarHeightDip() const = 0;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_RENDER_WIDGET_HOST_VIEW_DELEGATE_H_
