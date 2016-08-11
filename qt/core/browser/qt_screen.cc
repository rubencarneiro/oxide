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

#include "shared/browser/display_form_factor.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_type_conversions.h"

using oxide::DisplayFormFactor;

namespace oxide {
namespace qt {

namespace {
bool g_enable_qtubuntu_integration_for_testing = false;
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  connect(screen, &QScreen::availableGeometryChanged,
          this, [=] (const QRect&) {
    UpdateDisplayForScreen(screen, true);
  });
#else
  connect(screen, &QScreen::virtualGeometryChanged,
          this, [=] (const QRect&) {
    UpdateDisplayForScreen(screen, true);
  });
#endif
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
  if (property_name == QStringLiteral("scale") ||
      property_name == QStringLiteral("formfactor")) {
    UpdateDisplayForScreen(screen->screen(), true);
  }
}

QScreen* Screen::QScreenFromDisplay(const display::Display& display) const {
  auto it =
      std::find_if(
          displays_.begin(), displays_.end(),
          [&display](std::map<QScreen*, display::Display>::value_type v) {
        return display.id() == v.second.id();
      });
  DCHECK(it != displays_.end());

  return it->first;
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
  if (platform.startsWith("ubuntu") || platform == "mirserver" ||
      g_enable_qtubuntu_integration_for_testing) {
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

display::Display Screen::DisplayFromQScreen(QScreen* screen) const {
  if (!screen) {
    return display::Display();
  }

  DCHECK(displays_.find(screen) != displays_.end());
  return displays_.at(screen);
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

DisplayFormFactor Screen::GetDisplayFormFactor(
    const display::Display& display) {
  QString platform = QGuiApplication::platformName();
  if (!platform.startsWith("ubuntu") && platform != "mirserver" &&
      !g_enable_qtubuntu_integration_for_testing) {
    return oxide::Screen::GetDisplayFormFactor(display);
  }

  QScreen* q_screen = QScreenFromDisplay(display);

  QPlatformNativeInterface* interface =
      QGuiApplication::platformNativeInterface();
  void* data =
      interface->nativeResourceForScreen(QByteArray("formfactor"), q_screen);
  if (!data) {
    return oxide::Screen::GetDisplayFormFactor(display);
  }

  switch (*reinterpret_cast<int*>(data)) {
    case 1: // mir_form_factor_phone
    case 2: // mir_form_factor_tablet
      return DisplayFormFactor::Mobile;
    case 3: // mir_form_factor_monitor
      return DisplayFormFactor::Monitor;
    case 4: // mir_form_factor_tv
      return DisplayFormFactor::Television;
    case 0: // mir_form_factor_unknown
    case 5: // mir_form_factor_projector
    default:
      return oxide::Screen::GetDisplayFormFactor(display);
  }
}

// static
void Screen::SetEnableQtUbuntuIntegrationForTesting(bool enable) {
  g_enable_qtubuntu_integration_for_testing = enable;
}

} // namespace qt
} // namespace oxide
