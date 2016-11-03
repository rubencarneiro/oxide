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

#include "qt/core/glue/edit_capability_flags.h"
#include "qt/core/glue/web_context_menu.h"
#include "shared/browser/web_context_menu_client.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

MediaType ToMediaType(blink::WebContextMenuData::MediaType type) {
  switch (type) {
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

MediaStatusFlags ToMediaStatusFlags(int flags) {
  MediaStatusFlags rv;
  if (flags & blink::WebContextMenuData::MediaInError) {
    rv |= MEDIA_STATUS_IN_ERROR;
  }
  if (flags & blink::WebContextMenuData::MediaPaused) {
    rv |= MEDIA_STATUS_PAUSED;
  }
  if (flags & blink::WebContextMenuData::MediaMuted) {
    rv |= MEDIA_STATUS_MUTED;
  }
  if (flags & blink::WebContextMenuData::MediaLoop) {
    rv |= MEDIA_STATUS_LOOP;
  }
  if (flags & blink::WebContextMenuData::MediaCanSave) {
    rv |= MEDIA_STATUS_CAN_SAVE;
  }
  if (flags & blink::WebContextMenuData::MediaHasAudio) {
    rv |= MEDIA_STATUS_HAS_AUDIO;
  }
  if (flags & blink::WebContextMenuData::MediaCanToggleControls) {
    rv |= MEDIA_STATUS_CAN_TOGGLE_CONTROLS;
  }
  if (flags & blink::WebContextMenuData::MediaControls) {
    rv |= MEDIA_STATUS_CONTROLS;
  }
  if (flags & blink::WebContextMenuData::MediaCanPrint) {
    rv |= MEDIA_STATUS_CAN_PRINT;
  }
  if (flags & blink::WebContextMenuData::MediaCanRotate) {
    rv |= MEDIA_STATUS_CAN_ROTATE;
  }
  return rv;
}

EditCapabilityFlags ToEditCapabilityFlags(int flags) {
  EditCapabilityFlags rv;
  if (flags & blink::WebContextMenuData::CanUndo) {
    rv |= EDIT_CAPABILITY_UNDO;
  }
  if (flags & blink::WebContextMenuData::CanRedo) {
    rv |= EDIT_CAPABILITY_REDO;
  }
  if (flags & blink::WebContextMenuData::CanCut) {
    rv |= EDIT_CAPABILITY_CUT;
  }
  if (flags & blink::WebContextMenuData::CanCopy) {
    rv |= EDIT_CAPABILITY_COPY;
  }
  if (flags & blink::WebContextMenuData::CanPaste) {
    rv |= EDIT_CAPABILITY_PASTE;
  }
  if (flags & blink::WebContextMenuData::CanDelete) {
    rv |= EDIT_CAPABILITY_ERASE;
  }
  if (flags & blink::WebContextMenuData::CanSelectAll) {
    rv |= EDIT_CAPABILITY_SELECT_ALL;
  }
  return rv;
}

}

void WebContextMenuImpl::Show() {
  menu_->Show();
}

void WebContextMenuImpl::Hide() {
  menu_->Hide();
}

void WebContextMenuImpl::close() {
  client_->Close();
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

WebContextMenuParams WebContextMenuImpl::GetParams() const {
  WebContextMenuParams rv;

  rv.page_url = QUrl(QString::fromStdString(params_.page_url.spec()));
  rv.frame_url = QUrl(QString::fromStdString(params_.frame_url.spec()));

  ContentsView* contents_view =
      ContentsView::FromWebContents(client_->GetWebContents());
  gfx::Point position =
      DpiUtils::ConvertChromiumPixelsToQt(gfx::Point(params_.x, params_.y),
                                          contents_view->GetScreen());
  rv.position = ToQt(position);

  rv.link_url = QUrl(QString::fromStdString(params_.link_url.spec()));
  rv.unfiltered_link_url =
      QUrl(QString::fromStdString(params_.unfiltered_link_url.spec()));
  rv.link_text = QString::fromStdString(base::UTF16ToUTF8(params_.link_text));

  rv.media_type = ToMediaType(params_.media_type);
  rv.has_image_contents = params_.has_image_contents;
  rv.src_url = QUrl(QString::fromStdString(params_.src_url.spec()));

  rv.media_flags = ToMediaStatusFlags(params_.media_flags);

  rv.selection_text =
      QString::fromStdString(base::UTF16ToUTF8(params_.selection_text));
  rv.is_editable = params_.is_editable;
  rv.edit_flags = ToEditCapabilityFlags(params_.edit_flags);

  return std::move(rv);
}

} // namespace qt
} // namespace oxide
