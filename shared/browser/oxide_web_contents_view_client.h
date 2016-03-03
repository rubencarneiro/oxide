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

#include "base/macros.h"
#include "third_party/WebKit/public/platform/WebScreenInfo.h"

namespace content {
struct ContextMenuParams;
class NativeWebKeyboardEvent;
class RenderFrameHost;
class WebCursor;
}

namespace gfx {
class RectF;
}

namespace ui {
class TouchHandleDrawable;
}

namespace oxide {

class InputMethodContext;
class WebContentsView;
class WebContextMenu;
class WebPopupMenu;

class WebContentsViewClient {
 public:
  virtual ~WebContentsViewClient();

  // XXX(chrisccoulson) This should return a gfx::Display handle
  virtual blink::WebScreenInfo GetScreenInfo() const = 0;

  virtual bool IsVisible() const = 0;

  virtual bool HasFocus() const = 0;

  virtual gfx::RectF GetBounds() const = 0;

  virtual void SwapCompositorFrame() = 0;
  virtual void EvictCurrentFrame() = 0;

  virtual void UpdateCursor(const content::WebCursor& cursor);

  virtual WebContextMenu* CreateContextMenu(
      content::RenderFrameHost* rfh,
      const content::ContextMenuParams& params);

  virtual WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh);

  virtual ui::TouchHandleDrawable* CreateTouchHandleDrawable() const;
  virtual void TouchSelectionChanged(bool active,
                                     const gfx::RectF& bounds,
                                     bool handle_drag_in_progress) const;

  virtual InputMethodContext* GetInputMethodContext() const;

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
