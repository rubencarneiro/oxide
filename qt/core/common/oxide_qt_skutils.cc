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

#include "third_party/skia/include/core/SkImageInfo.h"

namespace oxide {
namespace qt {

QImage::Format QImageFormatFromSkImageInfo(const SkImageInfo& info) {
  SkAlphaType alpha = info.alphaType();
  if (alpha == kUnknown_SkAlphaType || alpha == kOpaque_SkAlphaType) {
    return QImage::Format_Invalid;
  }

  bool premultiplied = alpha == kPremul_SkAlphaType;

  switch (info.colorType()) {
    case kRGB_565_SkColorType:
      return QImage::Format_RGB16;
    case kARGB_4444_SkColorType:
      return premultiplied ?
          QImage::Format_ARGB4444_Premultiplied : QImage::Format_Invalid;
    case kRGBA_8888_SkColorType:
      return premultiplied ?
          QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBA8888;
    default:
      return QImage::Format_Invalid;
  }
}

} // namespace qt
} // namespace oxide
