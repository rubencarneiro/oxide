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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_CLIENT_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"

#include "shared/common/oxide_shared_export.h"

namespace content {
struct ContextMenuParams;
struct MenuItem;
class NativeWebKeyboardEvent;
class RenderFrameHost;
class WebCursor;
}

namespace ui {
class TouchHandleDrawable;
}

namespace oxide {

class InputMethodContext;
class LegacyTouchEditingClient;
class WebContentsView;
class WebContextMenu;
class WebPopupMenu;
class WebPopupMenuClient;

class OXIDE_SHARED_EXPORT WebContentsViewClient {
 public:
  virtual ~WebContentsViewClient();

  virtual display::Display GetDisplay() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  // The view's bounds in screen coordinates
  virtual gfx::RectF GetBounds() const = 0;

  // The top-level window's bounds in screen coordinates
  virtual gfx::Rect GetTopLevelWindowBounds() const = 0;

  virtual void SwapCompositorFrame() = 0;
  virtual void EvictCurrentFrame() = 0;

  virtual void UpdateCursor(const content::WebCursor& cursor);

  virtual std::unique_ptr<WebPopupMenu> CreatePopupMenu(
      const std::vector<content::MenuItem>& items,
      int selected_item,
      bool allow_multiple_selection,
      const gfx::Rect& bounds,
      WebPopupMenuClient* client);

  virtual std::unique_ptr<ui::TouchHandleDrawable> 
  CreateTouchHandleDrawable();

  virtual InputMethodContext* GetInputMethodContext() const;

  virtual LegacyTouchEditingClient* GetLegacyTouchEditingClient() const;

  virtual void UnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event);

 protected:
  WebContentsViewClient();

  WebContentsView* view() const { return view_; }

 private:
  friend class WebContentsView;

  WebContentsView* view_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsViewClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_VIEW_CLIENT_H_
