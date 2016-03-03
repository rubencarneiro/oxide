// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_qt_skutils.h"

#include <QImage>

#include "base/logging.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkImageInfo.h"

namespace oxide {
namespace qt {

namespace {

QImage::Format QImageFormatFromSkColorType(const SkColorType& type,
                                           bool premultiplied_alpha) {
  switch (type) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case kAlpha_8_SkColorType:
      return QImage::Format_Alpha8;
    case kGray_8_SkColorType:
      return QImage::Format_Grayscale8;
#endif
    case kRGB_565_SkColorType:
      return QImage::Format_RGB16;
    case kARGB_4444_SkColorType:
      return premultiplied_alpha ?
          QImage::Format_ARGB4444_Premultiplied : QImage::Format_Invalid;
    case kRGBA_8888_SkColorType:
      return premultiplied_alpha ?
          QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBA8888;
    default:
      return QImage::Format_Invalid;
  }
}

QImage QImageFromSkBitmap_BGRA_8888(const SkBitmap& bitmap) {
  QImage image(QSize(bitmap.width(), bitmap.height()),
               bitmap.alphaType() == kPremul_SkAlphaType ?
                   QImage::Format_RGBA8888_Premultiplied :
                   QImage::Format_RGBA8888);
  if (image.isNull()) {
    LOG(WARNING) << "Failed to allocate image";
    return QImage();
  }

  if (bitmap.getSize() != static_cast<size_t>(image.byteCount())) {
    LOG(WARNING) << "Mismatched image sizes";
    return QImage();
  }

  {
    SkAutoLockPixels lock(bitmap);
    memcpy(image.bits(), bitmap.getPixels(), image.byteCount());
  }

  static const int kPixelByteCount = 4;

  for (int y = 0; y < image.height(); ++y) {
    uchar* line = image.scanLine(y);
    for (int x = 0; x < image.width(); ++x) {
      uchar b = line[x * kPixelByteCount];
      uchar r = line[(x * kPixelByteCount) + 2];

      line[x * kPixelByteCount] = r;
      line[(x * kPixelByteCount) + 2] = b;
    }
  }

  return image;
}

}

QImage QImageFromSkBitmap(const SkBitmap& bitmap) {
  SkImageInfo info = bitmap.info();
  SkAlphaType alpha = info.alphaType();
  SkColorType color = info.colorType();

  if (alpha == kUnknown_SkAlphaType || alpha == kOpaque_SkAlphaType) {
    LOG(WARNING) << "Unsupported alpha type: " << alpha;
    return QImage();
  }

  QImage::Format format =
      QImageFormatFromSkColorType(color, alpha == kPremul_SkAlphaType);
  if (format != QImage::Format_Invalid) {
    SkAutoLockPixels lock(bitmap);
    return QImage(reinterpret_cast<const uchar*>(bitmap.getPixels()),
                  bitmap.width(), bitmap.height(), format);
  }

  if (color == kBGRA_8888_SkColorType) {
    return QImageFromSkBitmap_BGRA_8888(bitmap);
  }

  LOG(WARNING) <<
      "Unsupported image type, alpha: " << alpha << ", color: " << color;
  return QImage();
}

} // namespace qt
} // namespace oxide
