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

#include "web_contents_client.h"

#include "shared/browser/context_menu/web_context_menu.h"
#include "shared/browser/touch_selection/touch_editing_menu.h"
#include "shared/browser/touch_selection/touch_editing_menu_controller.h"

#include "web_contents_helper.h"

namespace oxide {

WebContentsClient::~WebContentsClient() = default;

// static
WebContentsClient* WebContentsClient::FromWebContents(
    content::WebContents* contents) {
  WebContentsHelper* helper = WebContentsHelper::FromWebContents(contents);
  if (!helper) {
    return nullptr;
  }
  return helper->client();
}

bool WebContentsClient::ShouldHandleNavigation(const GURL& url,
                                               bool user_gesture) {
  return true;
}

bool WebContentsClient::CanCreateWindows() {
  return false;
}

bool WebContentsClient::ShouldCreateNewWebContents(
    const GURL& url,
    WindowOpenDisposition disposition,
    bool user_gesture) {
  return true;
}

bool WebContentsClient::AdoptNewWebContents(
    const gfx::Rect& initial_pos,
    WindowOpenDisposition disposition,
    WebContentsUniquePtr contents) {
  return false;
}

void WebContentsClient::DownloadRequested(const GURL& url,
                                          const std::string& mime_type,
                                          const bool should_prompt,
                                          const base::string16& suggested_filename,
                                          const std::string& cookies,
                                          const std::string& referrer,
                                          const std::string& user_agent) {}

void WebContentsClient::HttpAuthenticationRequested(
    ResourceDispatcherHostLoginDelegate* login_delegate) {}

std::unique_ptr<WebContextMenu> WebContentsClient::CreateContextMenu(
    const content::ContextMenuParams& params,
    const std::vector<content::MenuItem>& items,
    WebContextMenuClient* client) {
  return nullptr;
}

std::unique_ptr<TouchEditingMenuController>
WebContentsClient::CreateOverrideTouchEditingMenuController(
    TouchEditingMenuControllerClient* client) {
  return nullptr;
}

std::unique_ptr<TouchEditingMenu> WebContentsClient::CreateTouchEditingMenu(
      blink::WebContextMenuData::EditFlags edit_flags,
      TouchEditingMenuClient* client) {
  return nullptr;
}

} // namespace oxide
