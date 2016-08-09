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

#include "qt_screen.h"

#include <QCursor>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformscreen.h>

#include "base/logging.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/rect.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_type_conversions.h"

namespace oxide {
namespace qt {

namespace {
int64_t g_next_id = 0;
}

void Screen::OnScreenAdded(QScreen* screen) {
  DCHECK(displays_.find(screen) == displays_.end());

  if (displays_.size() > 0) {
    QScreen* existing_screen = displays_.begin()->first;
    QList<QScreen*> virtual_siblings = screen->virtualSiblings();
    auto it = std::find(virtual_siblings.begin(), virtual_siblings.end(),
                        existing_screen);
    if (it == virtual_siblings.end()) {
      LOG(WARNING) <<
          "More than one virtual screen detected - this is not " <<
          "supported in Oxide";
      return;
    }
  }

  screen->setOrientationUpdateMask(Qt::LandscapeOrientation |
                                   Qt::PortraitOrientation |
                                   Qt::InvertedLandscapeOrientation |
                                   Qt::InvertedPortraitOrientation);
  connect(screen, &QScreen::availableGeometryChanged,
          this, [=] (const QRect&) {
    UpdateDisplayForScreen(screen, true);
  });
  connect(screen, &QScreen::geometryChanged,
          this, [=] (const QRect&) {
    UpdateDisplayForScreen(screen, true);
  });
  connect(screen, &QScreen::orientationChanged,
          this, [=] (Qt::ScreenOrientation) {
    UpdateDisplayForScreen(screen, true);
  });
  connect(screen, &QScreen::physicalDotsPerInchChanged,
          this, [=] (qreal) {
    UpdateDisplayForScreen(screen, true);
  });

  display::Display display;
  display.set_id(g_next_id++);
  display.set_touch_support(display::Display::TOUCH_SUPPORT_UNKNOWN);

  displays_[screen] = std::move(display);

  UpdateDisplayForScreen(screen, false);
  NotifyDisplayAdded(displays_[screen]);
}

void Screen::OnScreenRemoved(QScreen* screen) {
  DCHECK(displays_.find(screen) != displays_.end());

  display::Display display = displays_[screen];
  displays_.erase(screen);

  NotifyDisplayRemoved(display);
}

void Screen::OnPrimaryScreenChanged(QScreen* screen) {
  DCHECK_EQ(screen, QGuiApplication::primaryScreen());

  primary_display_ = nullptr;

  if (screen) {
    DCHECK(displays_.find(screen) != displays_.end());
    primary_display_ = &displays_[screen];
  }

  NotifyPrimaryDisplayChanged();
}

void Screen::OnPlatformScreenPropertyChanged(QPlatformScreen* screen,
                                             const QString& property_name) {
  if (property_name == QStringLiteral("scale")) {
    UpdateDisplayForScreen(screen->screen(), true);
  }
}

void Screen::UpdateDisplayForScreen(QScreen* screen,
                                    bool notify) {
  DCHECK(displays_.find(screen) != displays_.end());

  display::Display& display = displays_[screen];

  display.set_device_scale_factor(DpiUtils::GetScaleFactorForScreen(screen));

  // In Qt, at least with the xcb and windows backend, the display size is in
  // device-independent pixels but the display origin is in native pixels (which
  // are physical pixels with most platforms). Implementations of
  // display::Screen in Chromium seem to scale the display origin according to
  // the display's scale.

  gfx::Rect qt_bounds = ToChromium(screen->geometry());
  display.set_bounds(
      DpiUtils::ConvertQtPixelsToChromium(
          gfx::Rect(gfx::ScaleToFlooredPoint(qt_bounds.origin(),
                                             1 / screen->devicePixelRatio()),
                    qt_bounds.size()),
          screen));

  gfx::Rect qt_work_area = ToChromium(screen->availableGeometry());
  gfx::Point work_area_offset =
      DpiUtils::ConvertQtPixelsToChromium(
          qt_work_area.origin() - qt_bounds.origin().OffsetFromOrigin(),
          screen);
  display.set_work_area(
      gfx::Rect(display.bounds().origin() + work_area_offset.OffsetFromOrigin(),
                DpiUtils::ConvertQtPixelsToChromium(qt_work_area.size(),
                                                    screen)));

  display.SetRotationAsDegree(
      screen->angleBetween(screen->nativeOrientation(),
                           screen->orientation()));

  if (!notify) {
    return;
  }

  NotifyDisplayPropertiesChanged(display);
}

Screen::Screen()
    : primary_display_(nullptr) {
  connect(QGuiApplication::instance(), SIGNAL(screenAdded(QScreen*)),
          SLOT(OnScreenAdded(QScreen*)));
  connect(QGuiApplication::instance(), SIGNAL(screenRemoved(QScreen*)),
          SLOT(OnScreenRemoved(QScreen*)));
  connect(QGuiApplication::instance(), SIGNAL(primaryScreenChanged(QScreen*)),
          SLOT(OnPrimaryScreenChanged(QScreen*)));

  QString platform = QGuiApplication::platformName();
  if (platform.startsWith("ubuntu") || platform == "mirserver") {
    connect(QGuiApplication::platformNativeInterface(),
            SIGNAL(screenPropertyChanged(QPlatformScreen*, const QString&)),
            SLOT(OnPlatformScreenPropertyChanged(QPlatformScreen*,
                                                 const QString&)));
  }

  for (auto screen : QGuiApplication::screens()) {
    OnScreenAdded(screen);
  }
  OnPrimaryScreenChanged(QGuiApplication::primaryScreen());
}

Screen::~Screen() = default;

// static
Screen* Screen::GetInstance() {
  return static_cast<Screen*>(oxide::Screen::GetInstance());
}

display::Display Screen::DisplayFromQScreen(QScreen* screen) {
  if (!screen) {
    return display::Display();
  }

  DCHECK(displays_.find(screen) != displays_.end());
  return displays_[screen];
}

display::Display Screen::GetPrimaryDisplay() {
  if (!primary_display_) {
    return display::Display();
  }

  return *primary_display_;
}

std::vector<display::Display> Screen::GetAllDisplays() {
  std::vector<display::Display> rv;
  for (const auto& value : displays_) {
    rv.push_back(value.second);
  }

  if (rv.empty()) {
    rv.push_back(display::Display());
  }

  return rv;
}

gfx::Point Screen::GetCursorScreenPoint() {
  QPoint point = QCursor::pos();
  return gfx::Point(point.x(), point.y());
}

} // namespace qt
} // namespace oxide
