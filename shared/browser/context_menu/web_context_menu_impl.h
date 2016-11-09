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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_IMPL_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_IMPL_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/context_menu_params.h"
#include "content/public/common/menu_item.h"
#include "content/public/common/referrer.h"
#include "ui/base/window_open_disposition.h"

#include "shared/browser/context_menu/web_context_menu.h"
#include "shared/browser/context_menu/web_context_menu_client.h"

class GURL;

namespace oxide {

enum class WebContextMenuAction : unsigned;
enum class WebContextMenuActionGroup;

class WebContextMenuImpl : public WebContextMenu,
                           public WebContextMenuClient,
                           public content::WebContentsObserver {
 public:
  WebContextMenuImpl(content::RenderFrameHost* render_frame_host,
                     const content::ContextMenuParams& params);
  ~WebContextMenuImpl() override;

  // WebContextMenu implementation
  void Show() override;
  void Hide() override;

 private:
  class MenuBuilder;

  content::Referrer GetReferrer(const GURL& url) const;

  void OpenURL(const GURL& url,
               WindowOpenDisposition disposition);
  void SaveLink();
  void SaveImage();
  void CopyImage();
  void SaveMedia();

  void AppendLinkItems(std::vector<content::MenuItem>* items);
  void AppendImageItems(std::vector<content::MenuItem>* items);
  void AppendCanvasItems(std::vector<content::MenuItem>* items);
  void AppendAudioItems(std::vector<content::MenuItem>* items);
  void AppendVideoItems(std::vector<content::MenuItem>* items);
  void AppendEditableItems(std::vector<content::MenuItem>* items);
  void AppendCopyItems(std::vector<content::MenuItem>* items);

  bool IsCommandEnabled(WebContextMenuAction action) const;
  bool SupportsActionGroup(WebContextMenuActionGroup group) const;
  bool SupportsMediaActionGroup() const;

  std::vector<content::MenuItem> BuildItems();

  // WebContextMenuClient implementation
  content::WebContents* GetWebContents() const override;
  void Close() override;
  void ExecuteCommand(WebContextMenuAction action) override;

  // content::WebContentsObserver implementation
  void RenderFrameDeleted(content::RenderFrameHost* rfh) override;

  content::RenderFrameHost* render_frame_host_;

  content::ContextMenuParams params_;

  std::unique_ptr<WebContextMenu> menu_;

  DISALLOW_COPY_AND_ASSIGN(WebContextMenuImpl);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTEXT_MENU_IMPL_H_
