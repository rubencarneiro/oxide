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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PROXY_CLIENT_H_

#include <QPoint>
#include <QString>
#include <QtGlobal>
#include <QUrl>

namespace oxide {
namespace qt {

enum EditFlags {
  EDIT_CAN_DO_NONE = 0,
  EDIT_CAN_UNDO = 1 << 0,
  EDIT_CAN_REDO = 1 << 1,
  EDIT_CAN_CUT = 1 << 2,
  EDIT_CAN_COPY = 1 << 3,
  EDIT_CAN_PASTE = 1 << 4,
  EDIT_CAN_ERASE = 1 << 5,
  EDIT_CAN_SELECT_ALL = 1 << 6
};

enum MediaType {
  MEDIA_TYPE_NONE,
  MEDIA_TYPE_IMAGE,
  MEDIA_TYPE_VIDEO,
  MEDIA_TYPE_AUDIO,
  MEDIA_TYPE_CANVAS,
  MEDIA_TYPE_PLUGIN
};

class WebContextMenuProxyClient {
 public:
  virtual ~WebContextMenuProxyClient() {}

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

  virtual void cancel() = 0;

  virtual int editFlags() const = 0;
  virtual void undo() const = 0;
  virtual void redo() const = 0;
  virtual void cut() const = 0;
  virtual void copy() const = 0;
  virtual void paste() const = 0;
  virtual void erase() const = 0;
  virtual void selectAll() const = 0;

  virtual void saveLink() const = 0;
  virtual void saveImage() const = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_CONTEXT_MENU_PROXY_CLIENT_H_
