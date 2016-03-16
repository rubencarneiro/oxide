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

#ifndef _OXIDE_QT_CORE_BROWSER_TYPE_CONVERSIONS_H_
#define _OXIDE_QT_CORE_BROWSER_TYPE_CONVERSIONS_H_

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>

#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/size_f.h"

namespace oxide {
namespace qt {

// gfx::Point <-> QPoint
inline gfx::Point ToChromium(const QPoint& point) {
  return gfx::Point(point.x(), point.y());
}

inline QPoint ToQt(const gfx::Point& point) {
  return QPoint(point.x(), point.y());
}

// gfx::PointF <-> QPointF
inline gfx::PointF ToChromium(const QPointF& point) {
  return gfx::PointF(point.x(), point.y());
}

inline QPointF ToQt(const gfx::PointF& point) {
  return QPointF(point.x(), point.y());
}

// gfx::Rect <-> QRect
inline gfx::Rect ToChromium(const QRect& rect) {
  return gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QRect ToQt(const gfx::Rect& rect) {
  return QRect(rect.x(), rect.y(), rect.width(), rect.height());
}

// gfx::RectF <-> QRectF
inline gfx::RectF ToChromium(const QRectF& rect) {
  return gfx::RectF(rect.x(), rect.y(), rect.width(), rect.height());
}

inline QRectF ToQt(const gfx::RectF& rect) {
  return QRectF(rect.x(), rect.y(), rect.width(), rect.height());
}

// gfx::Size <-> QSize
inline gfx::Size ToChromium(const QSize& size) {
  return gfx::Size(size.width(), size.height());
}

inline QSize ToQt(const gfx::Size& size) {
  return QSize(size.width(), size.height());
}

// gfx::SizeF <-> QSizeF
inline gfx::SizeF ToChromium(const QSizeF& size) {
  return gfx::SizeF(size.width(), size.height());
}

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_TYPE_CONVERSIONS_H_
