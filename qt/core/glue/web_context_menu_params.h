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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PARAMS_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PARAMS_H_

#include <QFlags>
#include <QPoint>
#include <QString>
#include <QUrl>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/glue/edit_capability_flags.h"

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

namespace content {
struct ContextMenuParams;
}

namespace oxide {
namespace qt {

enum MediaType {
  MEDIA_TYPE_NONE,
  MEDIA_TYPE_IMAGE,
  MEDIA_TYPE_VIDEO,
  MEDIA_TYPE_AUDIO,
  MEDIA_TYPE_CANVAS,
  MEDIA_TYPE_PLUGIN
};

enum MediaStatusFlag {
  MEDIA_STATUS_NONE = 0,
  MEDIA_STATUS_IN_ERROR = 1 << 0,
  MEDIA_STATUS_PAUSED = 1 << 1,
  MEDIA_STATUS_MUTED = 1 << 2,
  MEDIA_STATUS_LOOP = 1 << 3,
  MEDIA_STATUS_CAN_SAVE = 1 << 4,
  MEDIA_STATUS_HAS_AUDIO = 1 << 5,
  MEDIA_STATUS_CAN_TOGGLE_CONTROLS = 1 << 6,
  MEDIA_STATUS_CONTROLS = 1 << 7,
  MEDIA_STATUS_CAN_PRINT = 1 << 8,
  MEDIA_STATUS_CAN_ROTATE = 1 << 9
};

Q_DECLARE_FLAGS(MediaStatusFlags, MediaStatusFlag);

struct OXIDE_QTCORE_EXPORT WebContextMenuParams {
  enum class SourceType {
    Mouse,
    Keyboard,
    Touch,
    LongPress,
    LongTap
  };

  QUrl page_url;
  QUrl frame_url;

  QPoint position;

  QUrl link_url;
  QUrl unfiltered_link_url;
  QString link_text;

  QString title_text;

  MediaType media_type = MEDIA_TYPE_NONE;
  bool has_image_contents = false;
  QUrl src_url;

  MediaStatusFlags media_flags;

  QString selection_text;
  bool is_editable = false;
  EditCapabilityFlags edit_flags;

  SourceType source_type;

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
  static WebContextMenuParams From(const content::ContextMenuParams& params,
                                   QScreen* screen);
#endif
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PARAMS_H_
