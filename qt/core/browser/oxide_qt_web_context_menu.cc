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
#include "shared/browser/oxide_web_view.h"

namespace oxide {
namespace qt {

WebContextMenu::~WebContextMenu() {}

void WebContextMenu::Show() {
  proxy_->Show();
}

void WebContextMenu::Hide() {
  proxy_->Hide();
}

MediaType WebContextMenu::mediaType() const {
  switch (params_.media_type) {
    case blink::WebContextMenuData::MediaTypeNone:
    case blink::WebContextMenuData::MediaTypeFile:
      return MEDIA_TYPE_NONE;
    case blink::WebContextMenuData::MediaTypeImage:
      return MEDIA_TYPE_IMAGE;
    case blink::WebContextMenuData::MediaTypeVideo:
      return MEDIA_TYPE_VIDEO;
    case blink::WebContextMenuData::MediaTypeAudio:
      return MEDIA_TYPE_AUDIO;
    case blink::WebContextMenuData::MediaTypeCanvas:
      return MEDIA_TYPE_CANVAS;
    case blink::WebContextMenuData::MediaTypePlugin:
      return MEDIA_TYPE_PLUGIN;
    default:
      Q_UNREACHABLE();
  }
}

QPoint WebContextMenu::position() const {
  // The position is expressed in device-independent pixels (DIP), it needs to
  // be multiplied by the device pixel ratio (DPR) to obtain physical pixels.
  oxide::WebView* webview = oxide::WebView::FromWebContents(web_contents());
  const float dpr = webview->GetScreenInfo().deviceScaleFactor;
  return QPoint(params_.x * dpr, params_.y * dpr);
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

int WebContextMenu::editFlags() const {
  int flags = NO_CAPABILITY;
  if (params_.edit_flags & blink::WebContextMenuData::CanUndo) {
    flags |= UNDO_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanRedo) {
    flags |= REDO_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanCut) {
    flags |= CUT_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanCopy) {
    flags |= COPY_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanPaste) {
    flags |= PASTE_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanDelete) {
    flags |= ERASE_CAPABILITY;
  }
  if (params_.edit_flags & blink::WebContextMenuData::CanSelectAll) {
    flags |= SELECT_ALL_CAPABILITY;
  }
  return flags;
}

int WebContextMenu::mediaFlags() const {
  int flags = MEDIA_STATUS_NONE;
  if (params_.media_flags & blink::WebContextMenuData::MediaInError) {
    flags |= MEDIA_STATUS_IN_ERROR;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaPaused) {
    flags |= MEDIA_STATUS_PAUSED;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaMuted) {
    flags |= MEDIA_STATUS_MUTED;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaLoop) {
    flags |= MEDIA_STATUS_LOOP;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaCanSave) {
    flags |= MEDIA_STATUS_CAN_SAVE;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaHasAudio) {
    flags |= MEDIA_STATUS_HAS_AUDIO;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaCanToggleControls) {
    flags |= MEDIA_STATUS_CAN_TOGGLE_CONTROLS;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaControls) {
    flags |= MEDIA_STATUS_CONTROLS;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaCanPrint) {
    flags |= MEDIA_STATUS_CAN_PRINT;
  }
  if (params_.media_flags & blink::WebContextMenuData::MediaCanRotate) {
    flags |= MEDIA_STATUS_CAN_ROTATE;
  }
  return flags;
}

void WebContextMenu::saveLink() const {
  SaveLink();
}

void WebContextMenu::saveMedia() const {
  SaveMedia();
}

WebContextMenu::WebContextMenu(content::RenderFrameHost* rfh,
                               const content::ContextMenuParams& params)
    : oxide::WebContextMenu(rfh, params) {}

void WebContextMenu::SetProxy(WebContextMenuProxy* proxy) {
  proxy_.reset(proxy);
}

} // namespace qt
} // namespace oxide
