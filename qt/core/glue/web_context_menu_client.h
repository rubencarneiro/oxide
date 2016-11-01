// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_CLIENT_H_

#include <QPoint>
#include <QString>
#include <QtGlobal>
#include <QUrl>

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

enum MediaStatusFlags {
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

class WebContextMenuClient {
 public:
  virtual ~WebContextMenuClient() {}

  virtual MediaType mediaType() const = 0;
  virtual QPoint position() const = 0;
  virtual QUrl linkUrl() const = 0;
  virtual QString linkText() const = 0;
  virtual QUrl unfilteredLinkUrl() const = 0;
  virtual QUrl srcUrl() const = 0;
  virtual bool hasImageContents() const = 0;
  virtual QUrl pageUrl() const = 0;
  virtual QUrl frameUrl() const = 0;
  virtual QString selectionText() const = 0;
  virtual bool isEditable() const = 0;

  virtual void close() = 0;

  virtual int editFlags() const = 0;

  virtual int mediaFlags() const = 0;

  virtual void copyImage() const = 0;
  virtual void saveLink() const = 0;
  virtual void saveMedia() const = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_CLIENT_H_
