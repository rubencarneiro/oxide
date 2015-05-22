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

#include "oxide_qt_web_context_menu.h"

#include <QPoint>
#include <QString>
#include <QUrl>

#include "base/strings/utf_string_conversions.h"

#include "qt/core/glue/oxide_qt_web_context_menu_proxy.h"

namespace oxide {
namespace qt {

WebContextMenu::~WebContextMenu() {}

void WebContextMenu::Show() {
  proxy_->Show();
}

void WebContextMenu::Hide() {
  proxy_->Hide();
}

QPoint WebContextMenu::position() const {
  return QPoint(params_.x, params_.y);
}

QUrl WebContextMenu::linkUrl() const {
  return QUrl(QString::fromStdString(params_.link_url.spec()));
}

QString WebContextMenu::linkText() const {
  return QString::fromStdString(base::UTF16ToUTF8(params_.link_text));
}

QUrl WebContextMenu::unfilteredLinkUrl() const {
  return QUrl(QString::fromStdString(params_.unfiltered_link_url.spec()));
}

QUrl WebContextMenu::srcUrl() const {
  return QUrl(QString::fromStdString(params_.src_url.spec()));
}

bool WebContextMenu::hasImageContents() const {
  return params_.has_image_contents;
}

QUrl WebContextMenu::pageUrl() const {
  return QUrl(QString::fromStdString(params_.page_url.spec()));
}

QUrl WebContextMenu::frameUrl() const {
  return QUrl(QString::fromStdString(params_.frame_url.spec()));
}

QString WebContextMenu::selectionText() const {
  return QString::fromStdString(base::UTF16ToUTF8(params_.selection_text));
}

bool WebContextMenu::isEditable() const {
  return params_.is_editable;
}

void WebContextMenu::cancel() {
  Close();
}

bool WebContextMenu::canUndo() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanUndo);
}

void WebContextMenu::undo() const {
  Undo();
}

bool WebContextMenu::canRedo() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanRedo);
}

void WebContextMenu::redo() const {
  Redo();
}

bool WebContextMenu::canCut() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanCut);
}

void WebContextMenu::cut() const {
  Cut();
}

bool WebContextMenu::canCopy() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanCopy);
}

void WebContextMenu::copy() const {
  Copy();
}

bool WebContextMenu::canPaste() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanPaste);
}

void WebContextMenu::paste() const {
  Paste();
}

bool WebContextMenu::canErase() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanDelete);
}

void WebContextMenu::erase() const {
  Erase();
}

bool WebContextMenu::canSelectAll() const {
  return (params_.edit_flags & blink::WebContextMenuData::CanSelectAll);
}

void WebContextMenu::selectAll() const {
  SelectAll();
}

void WebContextMenu::saveLink() const {
  SaveLink();
}

void WebContextMenu::saveImage() const {
  SaveImage();
}

WebContextMenu::WebContextMenu(content::RenderFrameHost* rfh,
                               const content::ContextMenuParams& params)
    : oxide::WebContextMenu(rfh, params) {}

void WebContextMenu::SetProxy(WebContextMenuProxy* proxy) {
  proxy_.reset(proxy);
}

} // namespace qt
} // namespace oxide
