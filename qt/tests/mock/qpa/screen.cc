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

#include "screen.h"

#include <QRect>
#include <QSize>
#include <QtDebug>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QTextStream>

#include "integration.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
# error "This code needs updating for changes to DPI handling in Qt 5.6"
#endif

QRect MockScreen::geometry() const {
  QRect geometry = geometry_;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
  int rotation = angleBetween(nativeOrientation(), orientation_);
#else
  int rotation = screen()->angleBetween(nativeOrientation(), orientation_);
#endif
  if (rotation == 90 || rotation == 270) {
    geometry = QRect(geometry.topLeft(), geometry.size().transposed());
  }

  // Qt < 5.6 expects scaled geometry from the platform plugin
  return QRect(geometry.topLeft(), geometry.size() / dpr_);
}

QRect MockScreen::availableGeometry() const {
  int l = work_area_in_screen_.x() / dpr_;
  int r = (geometry_.width() - work_area_in_screen_.width() - work_area_in_screen_.x()) / dpr_;
  int t = work_area_in_screen_.y() / dpr_;
  int b = (geometry_.height() - work_area_in_screen_.height() - work_area_in_screen_.y()) / dpr_;

  return geometry().adjusted(l, t, -r, -b);
}

int MockScreen::depth() const {
  return depth_;
}

QImage::Format MockScreen::format() const {
  return format_;
}

qreal MockScreen::devicePixelRatio() const {
  return dpr_;
}

Qt::ScreenOrientation MockScreen::nativeOrientation() const {
  return geometry_.width() > geometry_.height() ?
      Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

Qt::ScreenOrientation MockScreen::orientation() const {
  return orientation_;
}

QList<QPlatformScreen*> MockScreen::virtualSiblings() const {
  QList<QPlatformScreen*> rv;
  for (auto* screen : MockPlatformIntegration::instance()->screens()) {
    rv << screen;
  }
  return rv;
}

QString MockScreen::name() const {
  QString rv;
  QTextStream stream(&rv);
  stream << "TEST" << id_;
  return rv;
}

MockScreen::MockScreen(int id,
                       const QRect& geometry,
                       const QRect& work_area_in_screen,
                       int depth,
                       QImage::Format format,
                       qreal dpr,
                       int form_factor)
    : id_(id),
      geometry_(geometry),
      work_area_in_screen_(work_area_in_screen),
      depth_(depth),
      format_(format),
      dpr_(dpr),
      form_factor_(form_factor) {
  orientation_ = nativeOrientation();
}

MockScreen::~MockScreen() = default;

void MockScreen::setGeometry(const QRect& geometry,
                             const QRect& work_area_in_screen) {
  geometry_ = geometry;
  work_area_in_screen_ = work_area_in_screen;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     this->geometry(),
                                                     availableGeometry());
#else
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     this->geometry());
  QWindowSystemInterface::handleScreenAvailableGeometryChange(
      screen(), availableGeometry());
#endif
}

void MockScreen::setOrientation(Qt::ScreenOrientation orientation) {
  orientation_ = orientation;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     geometry(),
                                                     availableGeometry());
#else
  QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                     geometry());
  QWindowSystemInterface::handleScreenAvailableGeometryChange(
      screen(), availableGeometry());
#endif
  QWindowSystemInterface::handleScreenOrientationChange(screen(),
                                                        this->orientation());
}

void MockScreen::setFormFactor(int form_factor) {
  form_factor_ = form_factor;
}
