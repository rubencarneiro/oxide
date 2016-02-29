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

#include "oxide_qt_screen_client.h"

#include <QCursor>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "ui/gfx/display.h"
#include "ui/gfx/geometry/rect.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

void ScreenClient::OnScreenGeometryChanged(const QRect& geometry) {
  UpdatePrimaryDisplay();
}

void ScreenClient::OnScreenOrientationChanged(
    Qt::ScreenOrientation orientation) {
  UpdatePrimaryDisplay();
}

void ScreenClient::UpdatePrimaryDisplay() {
  base::AutoLock lock(primary_display_lock_);

  QScreen* screen = QGuiApplication::primaryScreen();

  primary_display_.set_id(0);
  primary_display_.set_touch_support(gfx::Display::TOUCH_SUPPORT_UNKNOWN);
  primary_display_.set_device_scale_factor(
      DpiUtils::GetScaleFactorForScreen(screen));

  primary_display_.set_bounds(
      DpiUtils::ConvertQtPixelsToChromium(ToChromium(screen->geometry()),
                                          screen));
  primary_display_.set_work_area(
      DpiUtils::ConvertQtPixelsToChromium(
        ToChromium(screen->availableGeometry()),
        screen));

  primary_display_.SetRotationAsDegree(
      screen->angleBetween(screen->nativeOrientation(),
                           screen->orientation()));
}

gfx::Display ScreenClient::GetPrimaryDisplay() {
  base::AutoLock lock(primary_display_lock_);
  return primary_display_;
}

gfx::Point ScreenClient::GetCursorScreenPoint() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  QPoint point = QCursor::pos();
  return gfx::Point(point.x(), point.y());
}

ScreenClient::ScreenClient() {
  QScreen* primary_screen = QGuiApplication::primaryScreen();
  primary_screen->setOrientationUpdateMask(Qt::LandscapeOrientation |
                                           Qt::PortraitOrientation |
                                           Qt::InvertedLandscapeOrientation |
                                           Qt::InvertedPortraitOrientation);
  connect(primary_screen, SIGNAL(virtualGeometryChanged(const QRect&)),
          SLOT(OnScreenGeometryChanged(const QRect&)));
  connect(primary_screen, SIGNAL(geometryChanged(const QRect&)),
          SLOT(OnScreenGeometryChanged(const QRect&)));
  connect(primary_screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)),
          SLOT(OnScreenOrientationChanged(Qt::ScreenOrientation)));
  connect(primary_screen,
          SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)),
          SLOT(OnScreenOrientationChanged(Qt::ScreenOrientation)));

  UpdatePrimaryDisplay();
}

ScreenClient::~ScreenClient() {}

} // namespace qt
} // namespace oxide
