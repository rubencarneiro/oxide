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

#include "oxide_qt_drag_source.h"

#include <QDrag>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QPointer>
#include <QPoint>

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/vector2d.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"

#include "shared/browser/oxide_drag_source_client.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_drag_utils.h"
#include "oxide_qt_skutils.h"

namespace oxide {
namespace qt {

void DragSource::StartDragging(content::WebContents* contents,
                               const content::DropData& drop_data,
                               blink::WebDragOperationsMask allowed_ops,
                               const gfx::ImageSkia& image,
                               const gfx::Vector2d& image_offset) {
  ContentsView* view = ContentsView::FromWebContents(contents);
  if (!view) {
    client()->EndDrag(blink::WebDragOperationNone);
    return;
  }

  DCHECK(view->native_view());
  DCHECK(view->GetScreen());

  QPointer<QDrag> drag = new QDrag(view->native_view());
  drag->setMimeData(new QMimeData());

  drag->setHotSpot(QPoint(image_offset.x(), image_offset.y()));

  float dpr = DpiUtils::GetScaleFactorForScreen(view->GetScreen());

  gfx::ImageSkiaRep image_rep = image.GetRepresentation(dpr);
  const SkBitmap& bitmap = image_rep.sk_bitmap();

  QPixmap pixmap;
  if (!bitmap.drawsNothing()) {
    QImage image = QImageFromSkBitmap(bitmap);
    image.setDevicePixelRatio(dpr);
    if (!image.isNull()) {
      pixmap = QPixmap::fromImage(image);
    }
  }
  drag->setPixmap(pixmap);

  ToQMimeData(drop_data, drag->mimeData());

  base::WeakPtr<DragSource> self = weak_ptr_factory_.GetWeakPtr();

  Qt::DropAction action = Qt::IgnoreAction;
  {
    base::MessageLoop::ScopedNestableTaskAllower a(
        base::MessageLoop::current());
    action = drag->exec(ToQtDropActions(allowed_ops));
  }

  if (!self) {
    return;
  }

  client()->EndDrag(ToWebDragOperation(action));
}

DragSource::DragSource(oxide::DragSourceClient* client)
    : oxide::DragSource(client),
      weak_ptr_factory_(this) {}

DragSource::~DragSource() {}

} // namespace qt
} // namespace oxide
