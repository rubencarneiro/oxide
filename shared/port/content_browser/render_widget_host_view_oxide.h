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

#ifndef _OXIDE_SHARED_PORT_CONTENT_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
#define _OXIDE_SHARED_PORT_CONTENT_BROWSER_RENDER_WIDGET_HOST_VIEW_H_

#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/common/content_export.h"
#include "ui/base/ime/text_input_type.h"

namespace gfx {
class Rect;
}

namespace content {

class CONTENT_EXPORT RenderWidgetHostViewOxide
    : public RenderWidgetHostViewBase {
 public:
  virtual ~RenderWidgetHostViewOxide();

 private:
  // content::RenderWidgetHostViewBase implementation
  void TextInputStateChanged(
      const ViewHostMsg_TextInputState_Params& params) final;
  void SelectionBoundsChanged(
      const ViewHostMsg_SelectionBounds_Params& params) final;

  virtual void OnTextInputStateChanged(ui::TextInputType type,
                                       bool show_ime_if_needed) = 0;
  virtual void OnSelectionBoundsChanged(const gfx::Rect& anchor_rect,
                                        const gfx::Rect& focus_rect,
                                        bool is_anchor_first) = 0;
};

} // namespace content

#endif // _OXIDE_SHARED_PORT_CONTENT_BROWSER_RENDER_WIDGET_HOST_VIEW_H_
