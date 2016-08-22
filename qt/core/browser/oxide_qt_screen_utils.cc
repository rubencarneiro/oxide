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

#if defined(ENABLE_HYBRIS)
#include "shared/browser/oxide_hybris_utils.h"
#endif

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {

blink::WebScreenOrientationType GetOrientationTypeFromScreenOrientation(
    Qt::ScreenOrientation orientation) {
  switch (orientation) {
    case Qt::PortraitOrientation:
      return blink::WebScreenOrientationPortraitPrimary;
    case Qt::LandscapeOrientation:
      return blink::WebScreenOrientationLandscapePrimary;
    case Qt::InvertedPortraitOrientation:
      return blink::WebScreenOrientationPortraitSecondary;
    case Qt::InvertedLandscapeOrientation:
      return blink::WebScreenOrientationLandscapeSecondary;
    default:
      NOTREACHED();
      return blink::WebScreenOrientationUndefined;
  }
}

}

blink::WebScreenInfo GetWebScreenInfoFromQScreen(QScreen* screen) {
  blink::WebScreenInfo result;

  result.depth = 24;
  result.depthPerComponent = 8; // XXX: Copied the GTK impl here
  result.isMonochrome = result.depth == 1;
  result.deviceScaleFactor = DpiUtils::GetScaleFactorForScreen(screen);

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
  result.availableRect = blink::WebRect(available_rect.x(),
                                        available_rect.y(),
                                        available_rect.width(),
                                        available_rect.height());

  result.orientationType =
      GetOrientationTypeFromScreenOrientation(screen->orientation());

  result.orientationAngle =
      screen->angleBetween(screen->orientation(),
                           GetNativeOrientation(screen));

  return result;
}

Qt::ScreenOrientation GetNativeOrientation(QScreen* screen) {
  Qt::ScreenOrientation native_orientation = screen->nativeOrientation();

#if defined(ENABLE_HYBRIS)
  // FIXME : Remove below hack once #1612659 is fixed
  std::string device_name;
  if (oxide::HybrisUtils::HasDeviceProperties()) {
    device_name = oxide::HybrisUtils::GetDeviceProperties().device;
  }

  if ((device_name == "cooler" || device_name == "frieza") &&
      screen == QGuiApplication::primaryScreen()) {
    // The native orientation returned by qt for M10 devices
    // is portrait which is wrong. It should be landscape.
    // See https://launchpad.net/bugs/1601887
    native_orientation = Qt::LandscapeOrientation;
  }
#endif

 return native_orientation;
}

} // namespace qt
} // namespace oxide
