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
#include <QString>
#include <QUrl>

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/nullable_string16.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/drop_data.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/vector2d.h"
#include "url/gurl.h"

#include "shared/browser/oxide_drag_source_client.h"

#include "oxide_qt_contents_native_view_data.h"
#include "oxide_qt_skutils.h"

namespace oxide {
namespace qt {

namespace {

void PrepareMimeData(QMimeData* mime_data,
                     const content::DropData& drop_data) {
  mime_data->setData("oxide/x-renderer-taint", QByteArray());

  if (!drop_data.text.string().empty()) {
    QString text =
        QString::fromStdString(base::UTF16ToUTF8(drop_data.text.string()));
    mime_data->setText(text);
  }
  if (drop_data.url.is_valid()) {
    QUrl url = QUrl(QString::fromStdString(drop_data.url.spec()));
    mime_data->setUrls(QList<QUrl>() << url);
  }
  if (!drop_data.html.string().empty()) {
    mime_data->setHtml(
        QString::fromStdString(base::UTF16ToUTF8(drop_data.html.string())));
  }
}

Qt::DropActions ToDropActions(blink::WebDragOperationsMask mask) {
  Qt::DropActions actions = Qt::IgnoreAction;

  if (mask & blink::WebDragOperationCopy) {
    actions |= Qt::CopyAction;
  }
  if (mask & blink::WebDragOperationLink) {
    actions |= Qt::LinkAction;
  }
  if (mask & blink::WebDragOperationMove) {
    actions |= Qt::MoveAction;
  }

  return actions;
}

blink::WebDragOperation ToWebDragOperation(Qt::DropAction action) {
  switch (action) {
    case Qt::CopyAction:
      return blink::WebDragOperationCopy;
    case Qt::MoveAction:
    case Qt::TargetMoveAction:
      return blink::WebDragOperationMove;
    case Qt::LinkAction:
      return blink::WebDragOperationLink;
    case Qt::IgnoreAction:
      return blink::WebDragOperationNone;
    default:
      NOTREACHED();
      return blink::WebDragOperationNone;
  }
}

}

void DragSource::StartDragging(content::WebContents* contents,
                               const content::DropData& drop_data,
                               blink::WebDragOperationsMask allowed_ops,
                               const SkBitmap& bitmap,
                               const gfx::Vector2d& image_offset_pix) {
  ContentsNativeViewData* cnvd =
      ContentsNativeViewData::FromWebContents(contents);
  if (!cnvd) {
    client()->EndDrag(blink::WebDragOperationNone);
    return;
  }

  DCHECK(cnvd->GetNativeView());

  QPointer<QDrag> drag = new QDrag(cnvd->GetNativeView());
  drag->setMimeData(new QMimeData());

  drag->setHotSpot(QPoint(image_offset_pix.x(), image_offset_pix.y()));

  QPixmap pixmap;
  if (!bitmap.drawsNothing()) {
    QImage image = QImageFromSkBitmap(bitmap);
    if (!image.isNull()) {
      pixmap = QPixmap::fromImage(image);
    }
  }
  drag->setPixmap(pixmap);

  PrepareMimeData(drag->mimeData(), drop_data);

  base::WeakPtr<DragSource> self = weak_ptr_factory_.GetWeakPtr();

  Qt::DropAction action = Qt::IgnoreAction;
  {
    base::MessageLoop::ScopedNestableTaskAllower a(
        base::MessageLoop::current());
    action = drag->exec(ToDropActions(allowed_ops));
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
