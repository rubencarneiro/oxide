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

#include "qt_web_context_menu.h"

#include <QString>
#include <QUrl>

#include "base/strings/utf_string_conversions.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/gfx/geometry/point.h"
#include "url/gurl.h"

#include "qt/core/glue/web_context_menu.h"
#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "shared/browser/web_context_menu_client.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

void WebContextMenuImpl::Show() {
  menu_->Show();
}

void WebContextMenuImpl::Hide() {
  menu_->Hide();
}

MediaType WebContextMenuImpl::mediaType() const {
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

QPoint WebContextMenuImpl::position() const {
  ContentsView* contents_view =
      ContentsView::FromWebContents(client_->GetWebContents());
  gfx::Point position =
      DpiUtils::ConvertChromiumPixelsToQt(gfx::Point(params_.x, params_.y),
                                          contents_view->GetScreen());
  return ToQt(position);
}

QUrl WebContextMenuImpl::linkUrl() const {
  return QUrl(QString::fromStdString(params_.link_url.spec()));
}

QString WebContextMenuImpl::linkText() const {
  return QString::fromStdString(base::UTF16ToUTF8(params_.link_text));
}

QUrl WebContextMenuImpl::unfilteredLinkUrl() const {
  return QUrl(QString::fromStdString(params_.unfiltered_link_url.spec()));
}

QUrl WebContextMenuImpl::srcUrl() const {
  return QUrl(QString::fromStdString(params_.src_url.spec()));
}

bool WebContextMenuImpl::hasImageContents() const {
  return params_.has_image_contents;
}

QUrl WebContextMenuImpl::pageUrl() const {
  return QUrl(QString::fromStdString(params_.page_url.spec()));
}

QUrl WebContextMenuImpl::frameUrl() const {
  return QUrl(QString::fromStdString(params_.frame_url.spec()));
}

QString WebContextMenuImpl::selectionText() const {
  return QString::fromStdString(base::UTF16ToUTF8(params_.selection_text));
}

bool WebContextMenuImpl::isEditable() const {
  return params_.is_editable;
}

void WebContextMenuImpl::close() {
  client_->Close();
}

int WebContextMenuImpl::editFlags() const {
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

int WebContextMenuImpl::mediaFlags() const {
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

void WebContextMenuImpl::copyImage() const {
  client_->CopyImage();
}

void WebContextMenuImpl::saveLink() const {
  client_->SaveLink();
}

void WebContextMenuImpl::saveMedia() const {
  client_->SaveMedia();
}

WebContextMenuImpl::WebContextMenuImpl(const content::ContextMenuParams& params,
                                       oxide::WebContextMenuClient* client)
    : params_(params),
      client_(client) {}

WebContextMenuImpl::~WebContextMenuImpl() = default;

void WebContextMenuImpl::Init(std::unique_ptr<qt::WebContextMenu> menu) {
  menu_ = std::move(menu);
}

} // namespace qt
} // namespace oxide
