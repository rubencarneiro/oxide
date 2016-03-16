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

#ifndef _OXIDE_QT_CORE_BROWSER_DPI_UTILS_H_
#define _OXIDE_QT_CORE_BROWSER_DPI_UTILS_H_

#include <QtGlobal>

#include "base/macros.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/size_f.h"

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

// Interaction with most of Qt is done in device-independent pixels, with (in
// 5.6) Qt performing conversions between device-independent pixels and native
// pixels (which is what the underlying windowing system uses). The conversion
// is based on the pixel ratio of the screen, which is provided by the
// platform's QPA plugin. On X11, native pixels are equivalent to physical
// pixels, whereas on OSX they are scaled already by the OS.
// On Ubuntu / Mir, no scaling is done in Qt - it's performed in the UI toolkit,
// which sits above Oxide and we interact with Qt in physical pixels, which
// means we have to do scaling manually. That's what this class is for - it
// also allows us to apply an override scale for testing.
//
// Some terms to consider when using this class:
// - Physical pixels - shouldn't really need explaining
// - Chromium pixels - These are fully-scaled device-independent pixels used
//   in the interface between qt/ and shared/
// - Qt pixels - These are what we interface to Qt with - they may be device
//   indepdendent or physical pixels, depending on the platform. And they don't
//   have any additional scale applied to them
class DpiUtils {
 public:
  // Return the total scale factor for |screen|. This is a combination of that
  // specified by the underlying QPA plugin, toolkit/QPA specific overrides and
  // environment overrides
  static float GetScaleFactorForScreen(QScreen* screen);

  static gfx::Rect ConvertQtPixelsToChromium(const gfx::Rect& rect,
                                             QScreen* screen);
  static gfx::RectF ConvertQtPixelsToChromium(const gfx::RectF& rect,
                                              QScreen* screen);
  static gfx::Point ConvertQtPixelsToChromium(const gfx::Point& point,
                                              QScreen* screen);
  static gfx::PointF ConvertQtPixelsToChromium(const gfx::PointF& point,
                                               QScreen* screen);
  static gfx::Size ConvertQtPixelsToChromium(const gfx::Size& size,
                                             QScreen* screen);
  static gfx::SizeF ConvertQtPixelsToChromium(const gfx::SizeF& size,
                                              QScreen* screen);
  static float ConvertQtPixelsToChromium(float value, QScreen* screen);


  static gfx::Rect ConvertChromiumPixelsToQt(const gfx::Rect& rect,
                                             QScreen* screen);
  static gfx::RectF ConvertChromiumPixelsToQt(const gfx::RectF& rect,
                                              QScreen* screen);
  static gfx::Point ConvertChromiumPixelsToQt(const gfx::Point& point,
                                              QScreen* screen);
  static gfx::PointF ConvertChromiumPixelsToQt(const gfx::PointF& point,
                                               QScreen* screen);
  static gfx::Size ConvertChromiumPixelsToQt(const gfx::Size& size,
                                             QScreen* screen);
  static float ConvertChromiumPixelsToQt(float value, QScreen* screen);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(DpiUtils);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_DPI_UTILS_H_
