// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qt_backing_store.h"

#include <QImage>
#include <QPainter>
#include <vector>

#include "content/public/browser/render_process_host.h"
#include "skia/ext/platform_canvas.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/vector2d_conversions.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/rect_conversions.h"
#include "ui/gfx/rect_f.h"
#include "ui/surface/transport_dib.h"

static const int kMaxVideoLayerSize = 23170;

namespace oxide {
namespace qt {

BackingStore::BackingStore(
    content::RenderWidgetHost* widget, const gfx::Size& size, float scale) :
    content::BackingStore(widget, size),
    pixmap_(size.width() * scale, size.height() * scale),
    device_scale_factor_(scale) {}

void BackingStore::PaintToBackingStore(
    content::RenderProcessHost* process,
    TransportDIB::Id bitmap,
    const gfx::Rect& bitmap_rect,
    const std::vector<gfx::Rect>& copy_rects,
    float scale_factor,
    const base::Closure& completion_callback,
    bool* scheduled_completion_callback) {
  *scheduled_completion_callback = false;

  if (bitmap_rect.IsEmpty()) {
    return;
  }

  gfx::Rect pixel_bitmap_rect =
      gfx::ScaleToEnclosingRect(bitmap_rect, scale_factor);
  const int width = pixel_bitmap_rect.width();
  const int height = pixel_bitmap_rect.height();

  if (width <= 0 || width > kMaxVideoLayerSize ||
      height <= 0 || height > kMaxVideoLayerSize) {
    return;
  }

  TransportDIB* dib = process->GetTransportDIB(bitmap);
  if (!dib) {
    return;
  }

  QImage img(static_cast<uchar*>(dib->memory()),
             width, height, QImage::Format_ARGB32);

  QPainter painter(&pixmap_);
  for (size_t i = 0; i < copy_rects.size(); ++i) {
    gfx::Rect copy_rect = copy_rects[i];
    gfx::Rect pixel_copy_src_rect =
        gfx::ScaleToEnclosingRect(copy_rect, scale_factor);
    gfx::Rect pixel_copy_dst_rect =
        gfx::ScaleToEnclosingRect(copy_rect, device_scale_factor_);

    QRect src_rect(pixel_copy_src_rect.x() - pixel_bitmap_rect.x(),
                   pixel_copy_src_rect.y() - pixel_bitmap_rect.y(),
                   pixel_copy_src_rect.width(),
                   pixel_copy_src_rect.height());
    QRect dst_rect(pixel_copy_dst_rect.x(),
                   pixel_copy_dst_rect.y(),
                   pixel_copy_dst_rect.width(),
                   pixel_copy_dst_rect.height());

    painter.drawImage(dst_rect, img, src_rect);
  }
}

bool BackingStore::CopyFromBackingStore(const gfx::Rect& rect,
                                        skia::PlatformBitmap* output) {
  const int width = std::min(size().width(), rect.width());
  const int height = std::min(size().height(), rect.height());

  if (!output->Allocate(width, height, true)) {
    return false;
  }

  const SkBitmap& bitmap = output->GetBitmap();
  SkAutoLockPixels alp(bitmap);

  if (bitmap.config() != SkBitmap::kARGB_8888_Config) {
    return false;
  }

  QImage img(pixmap_.toImage());

  int bytes_per_line = img.bytesPerLine();
  const uchar* data = img.bits();

  for (int y = 0; y < height; ++y) {
    const uint32* src_row =
        reinterpret_cast<const uint32*>(data + (bytes_per_line * y));
    uint32* dest_row = bitmap.getAddr32(0, y);
    for (int x = 0; x < width; ++x, ++dest_row) {
      *dest_row = src_row[x] | 0xff000000;
    }
  }

  return true;
}

void BackingStore::ScrollBackingStore(const gfx::Vector2d& delta,
                                      const gfx::Rect& clip_rect,
                                      const gfx::Size& view_size) {
  DCHECK(delta.x() == 0 || delta.y() == 0);

  gfx::Rect pixel_rect = gfx::ToEnclosingRect(
      gfx::ScaleRect(clip_rect, device_scale_factor_));
  gfx::Vector2d pixel_delta = gfx::ToFlooredVector2d(
      gfx::ScaleVector2d(delta, device_scale_factor_));

  pixmap_.scroll(pixel_delta.x(),
                 pixel_delta.y(),
                 pixel_rect.x(),
                 pixel_rect.y(),
                 pixel_rect.width(),
                 pixel_rect.height());
}

} // namespace qt
} // namespace oxide
