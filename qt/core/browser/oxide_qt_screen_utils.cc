// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include "oxide_qt_screen_utils.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <QString>

#include "third_party/WebKit/public/platform/modules/screen_orientation/WebScreenOrientationType.h"
#include "third_party/WebKit/public/platform/WebRect.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

blink::WebScreenOrientationType GetOrientationTypeFromScreenOrientation(
    Qt::ScreenOrientation orientation) {
  switch (orientation) {
    case Qt::PortraitOrientation:
      return blink::kWebScreenOrientationPortraitPrimary;
    case Qt::LandscapeOrientation:
      return blink::kWebScreenOrientationLandscapePrimary;
    case Qt::InvertedPortraitOrientation:
      return blink::kWebScreenOrientationPortraitSecondary;
    case Qt::InvertedLandscapeOrientation:
      return blink::kWebScreenOrientationLandscapeSecondary;
    default:
      NOTREACHED();
      return blink::kWebScreenOrientationUndefined;
  }
}

}

blink::WebScreenInfo GetWebScreenInfoFromQScreen(QScreen* screen) {
  blink::WebScreenInfo result;

  result.depth = 24;
  result.depth_per_component = 8; // XXX: Copied the GTK impl here
  result.is_monochrome = result.depth == 1;
  result.device_scale_factor = DpiUtils::GetScaleFactorForScreen(screen);

  gfx::Rect rect =
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(screen->geometry()),
                                          screen);
  result.rect = blink::WebRect(rect.x(),
                               rect.y(),
                               rect.width(),
                               rect.height());

  gfx::Rect available_rect =
      DpiUtils::ConvertQtPixelsToChromium(
        ToChromium(screen->availableGeometry()),
        screen);
  result.available_rect = blink::WebRect(available_rect.x(),
                                         available_rect.y(),
                                         available_rect.width(),
                                         available_rect.height());

  result.orientation_type =
      GetOrientationTypeFromScreenOrientation(screen->orientation());

  result.orientation_angle =
      screen->angleBetween(screen->orientation(),
                           screen->nativeOrientation());

  return result;
}

} // namespace qt
} // namespace oxide
