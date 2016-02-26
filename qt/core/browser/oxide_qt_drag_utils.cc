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

#include "oxide_qt_drag_utils.h"

#include <QByteArray>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QString>
#include <QUrl>

#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/pickle.h"
#include "base/strings/nullable_string16.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/drop_data.h"
#include "url/gurl.h"
#include "ui/events/event_constants.h"
#include "ui/gfx/geometry/point.h"

#include "shared/common/oxide_enum_flags.h"

namespace oxide {
namespace qt {

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebDragOperationsMask)

const char kRendererTaint[] = "oxide/x-renderer-taint";
const char kCustomRendererData[] = "oxide/x-custom-renderer-data";

void UnpickleCustomRendererData(
    const QByteArray& custom_data,
    std::map<base::string16, base::string16>* out) {
  if (custom_data.isEmpty()) {
    return;
  }

  base::Pickle pickle(custom_data.constData(), custom_data.size());
  base::PickleIterator iter(pickle);

  uint64_t size;
  if (!iter.ReadUInt64(&size)) {
    return;
  }

  std::map<base::string16, base::string16> custom_map;

  for (uint64_t i = 0; i < size; ++i) {
    base::string16 type;
    base::string16 data;

    if (!iter.ReadString16(&type)) {
      return;
    }
    if (!iter.ReadString16(&data)) {
      return;
    }

    custom_map[type] = data;
  }

  std::swap(*out, custom_map);
}

void ToDropData(const QMimeData* mime_data, content::DropData* drop_data) {
  if (!mime_data->data(kRendererTaint).isEmpty()) {
    drop_data->did_originate_from_renderer = true;
  }

  if (mime_data->hasText()) {
    drop_data->text =
        base::NullableString16(
          base::UTF8ToUTF16(mime_data->text().toStdString()), false);
  }
  if (mime_data->hasUrls()) {
    drop_data->url = GURL(mime_data->urls()[0].toString().toStdString());
  }
  if (mime_data->hasHtml()) {
    drop_data->html =
        base::NullableString16(
          base::UTF8ToUTF16(mime_data->html().toStdString()), false);
  }

  if (drop_data->did_originate_from_renderer) {
    UnpickleCustomRendererData(mime_data->data(kCustomRendererData),
                               &drop_data->custom_data);
  }
}

blink::WebDragOperationsMask ToWebDragOperations(Qt::DropActions actions) {
  blink::WebDragOperationsMask ops = blink::WebDragOperationNone;
  if (actions | Qt::CopyAction) {
    ops |= blink::WebDragOperationCopy;
  }
  if ((actions | Qt::MoveAction) || (actions | Qt::TargetMoveAction)) {
    ops |= blink::WebDragOperationMove;
  }
  if (actions | Qt::LinkAction) {
    ops |= blink::WebDragOperationLink;
  }

  return ops;
}

}

void ToQMimeData(const content::DropData& drop_data, QMimeData* mime_data) {
  mime_data->setData(kRendererTaint, QByteArray(1, 'a'));

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

  if (drop_data.custom_data.size() > 0) {
    base::Pickle pickle;
    pickle.WriteUInt64(
        base::checked_cast<uint64_t>(drop_data.custom_data.size()));
    for (const auto& custom_data : drop_data.custom_data) {
      pickle.WriteString16(custom_data.first);
      pickle.WriteString16(custom_data.second);
    }
    if (pickle.size() < std::numeric_limits<int>::max()) {
      mime_data->setData(kCustomRendererData,
                         QByteArray(static_cast<const char*>(pickle.data()),
                                    base::checked_cast<int>(pickle.size())));
    }
  }
}

Qt::DropActions ToQtDropActions(blink::WebDragOperationsMask mask) {
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

void GetDragEnterEventParams(QDragEnterEvent* event,
                             float device_scale,
                             content::DropData* drop_data,
                             gfx::Point* location,
                             blink::WebDragOperationsMask* allowed_ops,
                             int* key_modifiers) {
  ToDropData(event->mimeData(), drop_data);
  *allowed_ops = ToWebDragOperations(event->possibleActions());

  GetDropEventParams(event, device_scale, location, key_modifiers);
}

void GetDropEventParams(QDropEvent* event,
                        float device_scale,
                        gfx::Point* location,
                        int* key_modifiers) {
  *location = gfx::ScaleToRoundedPoint(gfx::Point(event->pos().x(),
                                                  event->pos().y()),
                                       1 / device_scale);

  Qt::KeyboardModifiers modifiers = event->keyboardModifiers();
  if (modifiers & Qt::ShiftModifier) {
    *key_modifiers |= ui::EF_SHIFT_DOWN;
  }
  if (modifiers & Qt::ControlModifier) {
    *key_modifiers |= ui::EF_CONTROL_DOWN;
  }
  if (modifiers & Qt::AltModifier) {
    *key_modifiers |= ui::EF_ALT_DOWN;
  }
  if (modifiers & Qt::MetaModifier) {
    *key_modifiers |= ui::EF_COMMAND_DOWN;
  }
}

Qt::DropAction ToQtDropAction(blink::WebDragOperation op) {
  switch (op) {
    case blink::WebDragOperationCopy:
      return Qt::CopyAction;
    case blink::WebDragOperationMove:
      return Qt::MoveAction;
    case blink::WebDragOperationLink:
      return Qt::LinkAction;
    case blink::WebDragOperationNone:
      return Qt::IgnoreAction;
    default:
      return Qt::IgnoreAction;
  }
}

} // namespace qt
} // namespace oxide
