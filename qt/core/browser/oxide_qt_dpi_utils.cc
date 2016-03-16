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

#include "oxide_qt_dpi_utils.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <QString>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include "ui/gfx/geometry/rect_f.h"

namespace oxide {
namespace qt {

namespace {

float GetUbuntuScale(QScreen* screen) {
  QString platform = QGuiApplication::platformName();
  if (!platform.startsWith("ubuntu") && platform != "mirserver") {
    return 1.f;
  }

  QPlatformNativeInterface* interface =
      QGuiApplication::platformNativeInterface();
  void* data =
      interface->nativeResourceForScreen(QByteArray("scale"), screen);
  if (data) {
    return *reinterpret_cast<float*>(data);
  }

  QByteArray grid_unit_px(qgetenv("GRID_UNIT_PX"));
  bool ok;
  float scale = grid_unit_px.toFloat(&ok);
  if (!ok) {
    return 1.f;
  }

  return scale / 8;
}

float GetEnvironmentScaleFactor() {
  QByteArray scale_factor(qgetenv("OXIDE_SCALE_FACTOR"));
  bool ok;
  float scale = scale_factor.toFloat(&ok);
  if (!ok) {
    return 1.f;
  }

  return scale;
}

float GetEnvironmentForceScaleFactor(bool *ok) {
  QByteArray force_scale_factor(qgetenv("OXIDE_FORCE_SCALE_FACTOR"));
  return force_scale_factor.toFloat(ok);
}

float GetExtraDeviceScaleForScreen(QScreen* screen) {
  bool force;
  float force_scale = GetEnvironmentForceScaleFactor(&force);
  if (force) {
    return force_scale / screen->devicePixelRatio();
  }

  return GetUbuntuScale(screen) * GetEnvironmentScaleFactor();
}

}

// static
float DpiUtils::GetScaleFactorForScreen(QScreen* screen) {
  return screen->devicePixelRatio() * GetExtraDeviceScaleForScreen(screen);
}

// static
gfx::Rect DpiUtils::ConvertQtPixelsToChromium(const gfx::Rect& rect,
                                              QScreen* screen) {
  return gfx::ScaleToEnclosedRect(rect,
                                  1 / GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::RectF DpiUtils::ConvertQtPixelsToChromium(const gfx::RectF& rect,
                                               QScreen* screen) {
  return gfx::ScaleRect(rect, 1 / GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::Point DpiUtils::ConvertQtPixelsToChromium(const gfx::Point& point,
                                               QScreen* screen) {
  return gfx::ScaleToFlooredPoint(point,
                                  1 / GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::PointF DpiUtils::ConvertQtPixelsToChromium(const gfx::PointF& point,
                                                QScreen* screen) {
  return gfx::ScalePoint(point, 1 / GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::Size DpiUtils::ConvertQtPixelsToChromium(const gfx::Size& size,
                                              QScreen* screen) {
  return gfx::ScaleToFlooredSize(size,
                                 1 / GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::SizeF DpiUtils::ConvertQtPixelsToChromium(const gfx::SizeF& size,
                                               QScreen* screen) {
  return gfx::ScaleSize(size, 1 / GetExtraDeviceScaleForScreen(screen));
}

// static
float DpiUtils::ConvertQtPixelsToChromium(float value, QScreen* screen) {
  return value / GetExtraDeviceScaleForScreen(screen);
}

// static
gfx::Rect DpiUtils::ConvertChromiumPixelsToQt(const gfx::Rect& rect,
                                              QScreen* screen) {
  return gfx::ScaleToEnclosingRect(rect, GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::RectF DpiUtils::ConvertChromiumPixelsToQt(const gfx::RectF& rect,
                                               QScreen* screen) {
  return gfx::ScaleRect(rect, GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::Point DpiUtils::ConvertChromiumPixelsToQt(const gfx::Point& point,
                                               QScreen* screen) {
  return gfx::ScaleToFlooredPoint(point, GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::PointF DpiUtils::ConvertChromiumPixelsToQt(const gfx::PointF& point,
                                                QScreen* screen) {
  return gfx::ScalePoint(point, GetExtraDeviceScaleForScreen(screen));
}

// static
gfx::Size DpiUtils::ConvertChromiumPixelsToQt(const gfx::Size& size,
                                              QScreen* screen) {
  return gfx::ScaleToFlooredSize(size, GetExtraDeviceScaleForScreen(screen));
}

// static
float DpiUtils::ConvertChromiumPixelsToQt(float value, QScreen* screen) {
  return value * GetExtraDeviceScaleForScreen(screen);
}

} // namespace qt
} // namespace oxide
