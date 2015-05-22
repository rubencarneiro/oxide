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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/context_menu_params.h"

class GURL;

namespace base {
template <typename T> class DeleteHelper;
}

namespace content {
class ContextMenuParams;
class RenderFrameHost;
class RenderFrameHostImpl;
}

namespace gfx {
class Rect;
}

namespace oxide {

class WebContextMenu : public content::WebContentsObserver {
 public:
  virtual void Show() = 0;
  void Close();

  base::WeakPtr<WebContextMenu> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 protected:
  friend class base::DeleteHelper<WebContextMenu>;

  WebContextMenu(content::RenderFrameHost* rfh,
                 const content::ContextMenuParams& params);
  virtual ~WebContextMenu();

  content::ContextMenuParams params_;

  void Undo() const;
  void Redo() const;
  void Cut() const;
  void Copy() const;
  void Paste() const;
  void Erase() const;
  void SelectAll() const;
  void SaveLink() const;
  void SaveImage() const;

 private:
  void RenderFrameDeleted(content::RenderFrameHost* rfh) final;

  virtual void Hide();

  content::RenderFrameHostImpl* render_frame_host_;

  base::WeakPtrFactory<WebContextMenu> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebContextMenu);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_H_
