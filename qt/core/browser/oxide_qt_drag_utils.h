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

#ifndef _OXIDE_QT_CORE_BROWSER_DRAG_UTILS_H_
#define _OXIDE_QT_CORE_BROWSER_DRAG_UTILS_H_

#include <Qt>
#include <QtGlobal>

#include "third_party/WebKit/public/platform/WebDragOperation.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
class QMimeData;
class QScreen;
QT_END_NAMESPACE

namespace content {
class DropData;
}

namespace gfx {
class Point;
}

namespace oxide {
namespace qt {

void ToQMimeData(const content::DropData& drop_data, QMimeData* mime_data);

Qt::DropActions ToQtDropActions(blink::WebDragOperationsMask mask);

blink::WebDragOperation ToWebDragOperation(Qt::DropAction action);

void GetDragEnterEventParams(QDragEnterEvent* event,
                             QScreen* screen,
                             float location_bar_content_offset,
                             content::DropData* drop_data,
                             gfx::Point* location,
                             blink::WebDragOperationsMask* allowed_ops,
                             int* key_modifiers);

void GetDropEventParams(QDropEvent* event,
                        QScreen* screen,
                        float location_bar_content_offset,
                        gfx::Point* location,
                        int* key_modifiers);

Qt::DropAction ToQtDropAction(blink::WebDragOperation op);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_DRAG_UTILS_H_
