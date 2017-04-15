// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016-2017 Canonical Ltd.

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

#include "web_context_menu_params.h"

#include "base/strings/utf_string_conversions.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/geometry/point.h"

#include "qt/core/browser/oxide_qt_dpi_utils.h"
#include "qt/core/browser/oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

MediaType ToMediaType(blink::WebContextMenuData::MediaType type) {
  switch (type) {
    case blink::WebContextMenuData::kMediaTypeNone:
    case blink::WebContextMenuData::kMediaTypeFile:
      return MEDIA_TYPE_NONE;
    case blink::WebContextMenuData::kMediaTypeImage:
      return MEDIA_TYPE_IMAGE;
    case blink::WebContextMenuData::kMediaTypeVideo:
      return MEDIA_TYPE_VIDEO;
    case blink::WebContextMenuData::kMediaTypeAudio:
      return MEDIA_TYPE_AUDIO;
    case blink::WebContextMenuData::kMediaTypeCanvas:
      return MEDIA_TYPE_CANVAS;
    case blink::WebContextMenuData::kMediaTypePlugin:
      return MEDIA_TYPE_PLUGIN;
    default:
      Q_UNREACHABLE();
  }
}

MediaStatusFlags ToMediaStatusFlags(int flags) {
  MediaStatusFlags rv;
  if (flags & blink::WebContextMenuData::kMediaInError) {
    rv |= MEDIA_STATUS_IN_ERROR;
  }
  if (flags & blink::WebContextMenuData::kMediaPaused) {
    rv |= MEDIA_STATUS_PAUSED;
  }
  if (flags & blink::WebContextMenuData::kMediaMuted) {
    rv |= MEDIA_STATUS_MUTED;
  }
  if (flags & blink::WebContextMenuData::kMediaLoop) {
    rv |= MEDIA_STATUS_LOOP;
  }
  if (flags & blink::WebContextMenuData::kMediaCanSave) {
    rv |= MEDIA_STATUS_CAN_SAVE;
  }
  if (flags & blink::WebContextMenuData::kMediaHasAudio) {
    rv |= MEDIA_STATUS_HAS_AUDIO;
  }
  if (flags & blink::WebContextMenuData::kMediaCanToggleControls) {
    rv |= MEDIA_STATUS_CAN_TOGGLE_CONTROLS;
  }
  if (flags & blink::WebContextMenuData::kMediaControls) {
    rv |= MEDIA_STATUS_CONTROLS;
  }
  if (flags & blink::WebContextMenuData::kMediaCanPrint) {
    rv |= MEDIA_STATUS_CAN_PRINT;
  }
  if (flags & blink::WebContextMenuData::kMediaCanRotate) {
    rv |= MEDIA_STATUS_CAN_ROTATE;
  }
  return rv;
}

EditCapabilityFlags ToEditCapabilityFlags(int flags) {
  EditCapabilityFlags rv;
  if (flags & blink::WebContextMenuData::kCanUndo) {
    rv |= EDIT_CAPABILITY_UNDO;
  }
  if (flags & blink::WebContextMenuData::kCanRedo) {
    rv |= EDIT_CAPABILITY_REDO;
  }
  if (flags & blink::WebContextMenuData::kCanCut) {
    rv |= EDIT_CAPABILITY_CUT;
  }
  if (flags & blink::WebContextMenuData::kCanCopy) {
    rv |= EDIT_CAPABILITY_COPY;
  }
  if (flags & blink::WebContextMenuData::kCanPaste) {
    rv |= EDIT_CAPABILITY_PASTE;
  }
  if (flags & blink::WebContextMenuData::kCanDelete) {
    rv |= EDIT_CAPABILITY_ERASE;
  }
  if (flags & blink::WebContextMenuData::kCanSelectAll) {
    rv |= EDIT_CAPABILITY_SELECT_ALL;
  }
  return rv;
}

WebContextMenuParams::SourceType ToSourceType(ui::MenuSourceType type) {
  switch (type) {
    case ui::MENU_SOURCE_MOUSE:
      return WebContextMenuParams::SourceType::Mouse;
    case ui::MENU_SOURCE_KEYBOARD:
      return WebContextMenuParams::SourceType::Keyboard;
    case ui::MENU_SOURCE_TOUCH:
      return WebContextMenuParams::SourceType::Touch;
    case ui::MENU_SOURCE_LONG_PRESS:
      return WebContextMenuParams::SourceType::LongPress;
    case ui::MENU_SOURCE_LONG_TAP:
      return WebContextMenuParams::SourceType::LongTap;
    default:
      NOTREACHED();
      return WebContextMenuParams::SourceType::Mouse;
  }
}

}

// static
WebContextMenuParams WebContextMenuParams::From(
    const content::ContextMenuParams& params,
    QScreen* screen) {
  WebContextMenuParams rv;

  rv.page_url = QUrl(QString::fromStdString(params.page_url.spec()));
  rv.frame_url = QUrl(QString::fromStdString(params.frame_url.spec()));

  gfx::Point position =
      DpiUtils::ConvertChromiumPixelsToQt(gfx::Point(params.x, params.y),
                                          screen);
  rv.position = ToQt(position);

  rv.link_url = QUrl(QString::fromStdString(params.link_url.spec()));
  rv.unfiltered_link_url =
      QUrl(QString::fromStdString(params.unfiltered_link_url.spec()));
  rv.link_text = QString::fromStdString(base::UTF16ToUTF8(params.link_text));

  rv.title_text = QString::fromStdString(base::UTF16ToUTF8(params.title_text));

  rv.media_type = ToMediaType(params.media_type);
  rv.has_image_contents = params.has_image_contents;
  rv.src_url = QUrl(QString::fromStdString(params.src_url.spec()));

  rv.media_flags = ToMediaStatusFlags(params.media_flags);

  rv.selection_text =
      QString::fromStdString(base::UTF16ToUTF8(params.selection_text));
  rv.is_editable = params.is_editable;
  rv.edit_flags = ToEditCapabilityFlags(params.edit_flags);

  rv.source_type = ToSourceType(params.source_type);

  return std::move(rv);
}

} // namespace qt
} // namespace oxide
