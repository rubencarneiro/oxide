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

#include "web_context_menu_host.h"

#include "qt/core/glue/macros.h"
#include "qt/core/glue/web_context_menu.h"
#include "qt/core/glue/web_context_menu_actions.h"
#include "qt/core/glue/web_context_menu_sections.h"
#include "shared/browser/context_menu/web_context_menu_actions.h"
#include "shared/browser/context_menu/web_context_menu_client.h"
#include "shared/browser/context_menu/web_context_menu_sections.h"

namespace oxide {
namespace qt {

void WebContextMenuHost::Show() {
  menu_->Show();
}

void WebContextMenuHost::Hide() {
  menu_->Hide();
}

void WebContextMenuHost::close() {
  client_->Close();
}

void WebContextMenuHost::execCommand(WebContextMenuAction action,
                                     bool close) {
  client_->ExecuteCommand(static_cast<oxide::WebContextMenuAction>(action),
                          close);
}

WebContextMenuHost::WebContextMenuHost(oxide::WebContextMenuClient* client)
    : client_(client) {
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::OpenLinkInNewTab,
                              oxide::WebContextMenuAction::OpenLinkInNewTab)
  STATIC_ASSERT_MATCHING_ENUM(
      WebContextMenuAction::OpenLinkInNewBackgroundTab,
      oxide::WebContextMenuAction::OpenLinkInNewBackgroundTab)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::OpenLinkInNewWindow,
                              oxide::WebContextMenuAction::OpenLinkInNewWindow)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::CopyLinkLocation,
                              oxide::WebContextMenuAction::CopyLinkLocation)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::SaveLink,
                              oxide::WebContextMenuAction::SaveLink)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::OpenImageInNewTab,
                              oxide::WebContextMenuAction::OpenImageInNewTab)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::CopyImageLocation,
                              oxide::WebContextMenuAction::CopyImageLocation)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::SaveImage,
                              oxide::WebContextMenuAction::SaveImage)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::CopyImage,
                              oxide::WebContextMenuAction::CopyImage)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::OpenMediaInNewTab,
                              oxide::WebContextMenuAction::OpenMediaInNewTab)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::CopyMediaLocation,
                              oxide::WebContextMenuAction::CopyMediaLocation)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::SaveMedia,
                              oxide::WebContextMenuAction::SaveMedia)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Undo,
                              oxide::WebContextMenuAction::Undo)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Redo,
                              oxide::WebContextMenuAction::Redo)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Cut,
                              oxide::WebContextMenuAction::Cut)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Copy,
                              oxide::WebContextMenuAction::Copy)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Paste,
                              oxide::WebContextMenuAction::Paste)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::Erase,
                              oxide::WebContextMenuAction::Erase)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuAction::SelectAll,
                              oxide::WebContextMenuAction::SelectAll)

  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::OpenLink,
                              oxide::WebContextMenuSection::OpenLink)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::Link,
                              oxide::WebContextMenuSection::Link)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::Media,
                              oxide::WebContextMenuSection::Media)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::Undo,
                              oxide::WebContextMenuSection::Undo)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::Editing,
                              oxide::WebContextMenuSection::Editing)
  STATIC_ASSERT_MATCHING_ENUM(WebContextMenuSection::Copy,
                              oxide::WebContextMenuSection::Copy)
}

WebContextMenuHost::~WebContextMenuHost() = default;

void WebContextMenuHost::Init(std::unique_ptr<qt::WebContextMenu> menu) {
  menu_ = std::move(menu);
}

} // namespace qt
} // namespace oxide
